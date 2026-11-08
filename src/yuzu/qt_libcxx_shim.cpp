#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QTimer>
#include <QtCore/QDir>
#include <QtCore/QMetaType>
#include <QtCore/QException>
#include <QtCore/QSharedData>
#include <chrono>
#include <string>
#include <cstring>
#include <optional>
#include <functional>
#include <exception>

// ABI bridge definitions between LLVM libc++ and Qt6 MinGW static binaries

std::string QString::toStdString() const {
    QByteArray utf8 = toUtf8();
    return std::string(utf8.constData(), static_cast<size_t>(utf8.size()));
}

std::string QByteArray::toStdString() const {
    return std::string(constData(), static_cast<size_t>(size()));
}

QByteArray QByteArray::fromStdString(const std::string &s) {
    return QByteArray(s.data(), static_cast<qsizetype>(s.size()));
}

bool QDir::mkpath(const QString &dirPath, std::optional<QFlags<QFileDevice::Permission>> permissions) const {
    return mkpath(dirPath);
}

extern "C" void _ZN6QTimer14singleShotImplEiN2Qt9TimerTypeEPK7QObjectPN9QtPrivate15QSlotObjectBaseE(
    int, Qt::TimerType, const QObject*, QtPrivate::QSlotObjectBase*);

void QTimer::singleShotImpl(std::chrono::nanoseconds interval, Qt::TimerType timerType,
                            const QObject *receiver, QtPrivate::QSlotObjectBase *slotObj) {
    int msec = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(interval).count());
    _ZN6QTimer14singleShotImplEiN2Qt9TimerTypeEPK7QObjectPN9QtPrivate15QSlotObjectBaseE(
        msec, timerType, receiver, slotObj);
}

bool QMetaType::registerConverterFunction(const ConverterFunction &f, QMetaType from, QMetaType to) {
    return false;
}

bool QMetaType::registerMutableViewFunction(const MutableViewFunction &f, QMetaType from, QMetaType to) {
    return false;
}

class QUnhandledExceptionPrivate : public QSharedData {};

extern "C" void _ZN19QUnhandledExceptionC1ENSt15__exception_ptr13exception_ptrE(void* this_ptr, void* ex_ptr);

QUnhandledException::QUnhandledException(std::exception_ptr exception) noexcept
    : QException() {
    void* raw_ptr = nullptr;
    static_assert(sizeof(std::exception_ptr) == sizeof(void*), "Size mismatch");
    std::memcpy(&raw_ptr, &exception, sizeof(void*));
    _ZN19QUnhandledExceptionC1ENSt15__exception_ptr13exception_ptrE(this, &raw_ptr);
}
