#pragma once
#include <stdint.h>

template<typename __LockTraits>
class ScopeGuard {
private:
    static constexpr uintptr_t FlagUnlockOnLeave = 0x1;
    static constexpr uintptr_t FlagLocked = 0x2;
    using LockType = typename __LockTraits::LockType;
    LockType& _Lock;
    uintptr_t _Flags;
public:

    ScopeGuard(LockType& LockInstance) noexcept :
        _Lock(LockInstance),
        _Flags(FlagUnlockOnLeave) {}

    ScopeGuard(const ScopeGuard<__LockTraits>&) = delete;
    ScopeGuard(ScopeGuard<__LockTraits>&&) = delete;

    ScopeGuard<__LockTraits>& operator=(const ScopeGuard<__LockTraits>&) = delete;
    ScopeGuard<__LockTraits>& operator=(ScopeGuard<__LockTraits>&&) = delete;

    void Lock() {
        __LockTraits::Lock(_Lock);
        _Flags |= FlagLocked;
    }

    bool TryLock() {
        if (__LockTraits::TryLock(_Lock)) {
            _Flags |= FlagLocked;
            return true;
        } else {
            return false;
        }
    }

    void Unlock() {
        __LockTraits::Unlock(_Lock);
        _Flags &= ~FlagLocked;
    }

    void Abandon() noexcept {
        _Flags &= ~FlagUnlockOnLeave;
    }

    void AbandonCancel() noexcept {
        _Flags |= FlagUnlockOnLeave;
    }

    ~ScopeGuard() {
        if (_Flags & FlagUnlockOnLeave) {
            if (_Flags & FlagLocked) {
                Unlock();
            }
        }
    }
};

