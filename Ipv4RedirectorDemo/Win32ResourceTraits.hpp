#pragma once
#include <windows.h>

struct GenericHandleTraits {
    using HandleType = HANDLE;

    static inline const HandleType DefaultInvalidValue = NULL;

    static bool IsValid(const HandleType& Handle) {
        return Handle != DefaultInvalidValue;
    }

    static void Releasor(const HandleType& Handle) {
        CloseHandle(Handle);
    }
};

struct HeapAllocTraits {
    using HandleType = PVOID;

    static inline const HandleType DefaultInvalidValue = NULL;

    static bool IsValid(const HandleType& Handle) {
        return Handle != DefaultInvalidValue;
    }

    static void Releasor(const HandleType& Handle) {
        HeapFree(GetProcessHeap(), 0, Handle);
    }
};

