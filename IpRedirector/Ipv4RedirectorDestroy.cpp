#include "IpRedirectInternal.h"
#include "ScopeGuard.hpp"
#include "Win32LockTraits.hpp"

BOOL WINAPI Ipv4RedirectorDestroy(PIPV4_REDIRECT_CTX Ctx) {
    ScopeGuard<CriticalSectionTraits> Guard(Ctx->CriticalSection);

    // refuse to destroy ctx without IPV4_REDIRECTOR_CTX_FLAG_INITIALIZED
    if (!IP_REDIRECTOR_HAS_FLAG(Ctx->Flags, IPV4_REDIRECTOR_CTX_FLAG_INITIALIZED))
        return FALSE;

    Guard.Lock();

    // if in working, refuse to destroy
    if (IP_REDIRECTOR_HAS_FLAG(Ctx->Flags, IPV4_REDIRECTOR_CTX_FLAG_WORKING))
        return FALSE;

    Ctx->Flags &= ~IPV4_REDIRECTOR_CTX_FLAG_INITIALIZED;

    Guard.Unlock();

    // release resource
    DeleteCriticalSection(&Ctx->CriticalSection);

    ZeroMemory(Ctx, sizeof(IPV4_REDIRECT_CTX));
    HeapFree(GetProcessHeap(), 0, Ctx);

    return TRUE;
}

