#pragma once
#include <stdint.h>

template<typename __LockTraits>
class ReadWriteGuard {
private:
    static constexpr uintptr_t FlagUnlockOnLeave = 0x1;
    static constexpr uintptr_t FlagLockedAsReader = 0x2;
    static constexpr uintptr_t FlagLockedAsWriter = 0x4;
    using LockType = typename __LockTraits::LockType;
    LockType& _Lock;
    uintptr_t _Flags;
public:

    ReadWriteGuard(LockType& LockInstance) noexcept :
        _Lock(LockInstance),
        _Flags(FlagUnlockOnLeave) {}

    ReadWriteGuard(const ReadWriteGuard<__LockTraits>&) = delete;
    ReadWriteGuard(ReadWriteGuard<__LockTraits>&&) = delete;

    ReadWriteGuard<__LockTraits>& operator=(const ReadWriteGuard<__LockTraits>&) = delete;
    ReadWriteGuard<__LockTraits>& operator=(ReadWriteGuard<__LockTraits>&&) = delete;

    void LockAsReader() {
        __LockTraits::LockAsReader(_Lock);
        _Flags |= FlagLockedAsReader;
    }

    void LockAsWriter() {
        __LockTraits::LockAsWriter(_Lock);
        _Flags |= FlagLockedAsWriter;
    }

    bool TryLockAsReader() {
        if (__LockTraits::TryLockAsReader(_Lock)) {
            _Flags |= FlagLockedAsReader;
            return true;
        } else {
            return false;
        }
    }

    bool TryLockAsWriter() {
        if (__LockTraits::TryLockAsWriter(_Lock)) {
            _Flags |= FlagLockedAsWriter;
            return true;
        } else {
            return false;
        }
    }

    void UnlockAsReader() {
        __LockTraits::UnlockAsReader(_Lock);
        _Flags &= ~FlagLockedAsReader;
    }

    void UnlockAsWriter() {
        __LockTraits::UnlockAsWriter(_Lock);
        _Flags &= ~FlagLockedAsWriter;
    }

    void Abandon() noexcept {
        _Flags &= ~FlagUnlockOnLeave;
    }

    void AbandonCancel() noexcept {
        _Flags |= FlagUnlockOnLeave;
    }

    ~ReadWriteGuard() {
        if (_Flags & FlagUnlockOnLeave) {
            if (_Flags & FlagLockedAsReader) {
                UnlockAsReader();
            }
            if (_Flags & FlagLockedAsWriter) {
                UnlockAsWriter();
            }
        }
    }
};

