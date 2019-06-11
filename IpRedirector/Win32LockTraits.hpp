#pragma once
#include <windows.h>

struct CriticalSectionTraits {
    using LockType = CRITICAL_SECTION;

    static void Lock(LockType& LockInstance) noexcept {
        EnterCriticalSection(&LockInstance);
    }

    static bool TryLock(LockType& LockInstance) noexcept {
        return TryEnterCriticalSection(&LockInstance) ? true : false;
    }

    static void Unlock(LockType& LockInstance) noexcept {
        LeaveCriticalSection(&LockInstance);
    }
};

struct SRWLockTraits {
    using LockType = SRWLOCK;

    static void LockAsReader(LockType& LockInstance) noexcept {
        AcquireSRWLockShared(&LockInstance);
    }

    static void LockAsWriter(LockType& LockInstance) noexcept {
        AcquireSRWLockExclusive(&LockInstance);
    }

    static bool TryLockAsReader(LockType& LockInstance) noexcept {
        return TryAcquireSRWLockShared(&LockInstance) ? true : false;
    }

    static bool TryLockAsWriter(LockType& LockInstance) noexcept {
        return TryAcquireSRWLockExclusive(&LockInstance) ? true : false;
    }

    static void UnlockAsReader(LockType& LockInstance) noexcept {
        ReleaseSRWLockShared(&LockInstance);
    }

    static void UnlockAsWriter(LockType& LockInstance) noexcept {
        ReleaseSRWLockExclusive(&LockInstance);
    }
};

