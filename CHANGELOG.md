# Changelog

All notable changes to Eglantine are documented here. Each release's notes are taken from its
`## [X.Y.Z]` section (release candidates use the section of the version they are a candidate for).
New entries go under `## [Unreleased]`; `tools/ci/bump-version.sh` turns them into a version section.

## [Unreleased]

- Run native tensor programs through Intel Arc cooperative matrices (HMMA/XMX) instead of the
  upscale stand-in, including the integrated Xe2 8x16x16 path.
- Enable FP16 and dynamic vertex input on integrated Arc GPUs.
- Log the Intel device profile and the selected cooperative matrix shape.

## [0.1.0]

- Initial release candidate
