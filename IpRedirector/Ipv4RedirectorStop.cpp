#include "IpRedirectInternal.h"
#include <windivert.h>
#include "ScopeGuard.hpp"
#include "Win32LockTraits.hpp"

#pragma comment(lib, "WinDivert")

BOOL WINAPI Ipv4RedirectorStop(PIPV4_REDIRECTOR_CTX Ctx) {
    ScopeGuard<CriticalSectionTraits> Guard(Ctx->CriticalSection);

    // if does not have IPV4_REDIRECTOR_CTX_FLAG_INITIALIZED, refuse to start
    if (!IP_REDIRECTOR_HAS_FLAG(Ctx->Flags, IPV4_REDIRECTOR_CTX_FLAG_INITIALIZED)) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Guard.Lock();

    // if not in working, refuse to stop
    if (!IP_REDIRECTOR_HAS_FLAG(Ctx->Flags, IPV4_REDIRECTOR_CTX_FLAG_WORKING)) {
        SetLastError(ERROR_INVALID_STATE);
        return FALSE;
    }

    Ctx->Flags |= IPV4_REDIRECTOR_CTX_FLAG_STOPPING;

    WinDivertShutdown(Ctx->WinDivertHandle, WINDIVERT_SHUTDOWN_BOTH);
    WaitForSingleObject(Ctx->WinDivertWorker, INFINITE);

    CloseHandle(Ctx->WinDivertWorker);
    Ctx->WinDivertWorker = NULL;

    WinDivertClose(Ctx->WinDivertHandle);
    Ctx->WinDivertHandle = INVALID_HANDLE_VALUE;
    
    Ctx->Flags &= ~IPV4_REDIRECTOR_CTX_FLAG_WORKING;
    Ctx->Flags &= ~IPV4_REDIRECTOR_CTX_FLAG_STOPPING;

    return TRUE;
}

