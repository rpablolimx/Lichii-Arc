# Intel Arc GPUs and Switch 2 titles

This page documents the Intel Arc policy of the Lichii Arc fork. It targets Intel Arc GPUs,
integrated Xe2 parts included (Lunar Lake's Arc 140V, Arc B-series and newer). The changes here
assume the Intel Windows driver; the Vulkan driver family is the number before the last dot
(`32.0.101.9175` is family 101).

Arc GPUs expose Xe Matrix Extensions (XMX) through `VK_KHR_cooperative_matrix`. Switch 2 titles run
their tensor work as `HMMA.16816.F16`, and Lichii can lower it to a cooperative matrix multiply
instead of the software warp expansion. [#779] implements the 8x16x16 shared-memory path that Xe2
parts need, and [#876] carries the Intel shader/cache fixes this fork starts from.

## What this fork changes

| Area | Behaviour |
|---|---|
| Native tensor programs | On Intel, a title's native matrix multiplies are kept when the pipeline cache selected a usable cooperative matrix shape. Without one, the program is rejected and the upscale stand-in runs, as before. `EDEN_NX2_INTEL_HMMA=0` keeps the stand-in for A/B tests. |
| Cooperative matrix shapes | The 16x8x16 register path is used when the device advertises it and its self-test passes. Otherwise the 16x16x16 or 8x16x16 shared-memory path runs with the compute subgroups forced to the guest's 32 lanes. |
| FP16 | Enabled on Arc, discrete or integrated. The Astral Chain workaround stays only for older Intel Xe iGPUs, whose model name has no "Arc". |
| Dynamic vertex input | `VK_EXT_vertex_input_dynamic_state` is used on Arc with driver family 101 or newer, integrated parts included. Older Intel drivers keep the fallback. |
| Descriptor buffer | Left at the driver's defaults; this fork does not force descriptor buffers or push descriptors on Intel. |
| Shader cache v2 | Captured native programs and their lowering are cached per program, keyed by the Intel native/stand-in decision. `EDEN_NX2_INTEL_HMMA` flipping the decision lowers the affected programs again instead of reusing the other path's code. |

The Arc 140V advertises only the 8x16x16 fp16 shape, so it takes the shared-memory path and does
not need the 16x8x16 self-test. Discrete Arc parts may advertise 16x8x16 and take the register path.

## Log lines

The device log now starts with one Intel profile line:

```text
Intel profile: Intel(R) Arc(TM) 140V GPU (8GB) | integrated | driver 101.9175 | Arc=true | float16_math=true | cooperative HMMA=8x16x16
```

The pipeline cache then reports the chosen path:

```text
Guest tensor operations run on 8x16x16 cooperative matrices
```

`cooperative HMMA=none` or a missing "Guest tensor operations" line means the title stays on the
software expansion or the upscale stand-in. Check the driver version and whether the game actually
dispatches a tensor program before changing anything else.

## Investigating an Intel-only bug

1. Confirm the profile line: `Arc=true`, `float16_math=true`, and the expected HMMA shape.
2. Try the same scene with `EDEN_NX2_REJECT_HASH` on the affected native program to see whether the
   stand-in path differs. Restore the hashes afterwards.
3. Use `EDEN_NX2_FRAME_DUMP` and the live debug server as described in [AGENTS.md](../../AGENTS.md).
4. Capture the translated frame with RenderDoc when the wrong output is in a specific pass.
5. Compare against a run without `float16_math` when the artifact looks like half-precision noise.

## Settings

- `EDEN_NX2_SPLIT=2` and `EDEN_NX2_VRAM_OVERRIDE_MIB=6144` remain useful on 8 GB integrated parts.
- `EDEN_NX2_INTEL_HMMA=0` falls back to the upscale stand-in on Intel without rebuilding. Use it to
  check whether a rendering difference comes from the cooperative matrix path.
- Keep the Vulkan driver pipeline cache on; Intel rebuilds are slow.
- `EDEN_NX2_REJECT_HASH` takes full 16-hex-digit program hashes. Reject only for A/B tests.

## Known limits

- [#779] was validated on an Arc B580 with Linux ANV. The Arc 140V is an integrated Xe2 part on the
  Windows driver, so the 8x16x16 path there still needs runtime testing before it is treated as
  proven. `vulkaninfo` on the 140V confirms the advertised requirements (fp16 8x16x16 shape,
  `shaderFloat16`, Vulkan memory model, subgroup size control with 32-lane compute subgroups and
  `computeFullSubgroups`), but that is not a rendering test.
- The cooperative matrix self-test only covers the 16x8x16 register path. The shared-memory shapes
  rely on the driver's advertised properties.
- A device that advertises a shape but fails the self-test keeps the software expansion; that is
  correct, but slower.

[#779]: https://github.com/zortos293/Lichii/pull/779
[#876]: https://github.com/zortos293/Lichii/pull/876
