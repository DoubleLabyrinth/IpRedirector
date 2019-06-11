#include "IpRedirectInternal.h"
#include <windivert.h>
#include "ScopeGuard.hpp"
#include "Win32LockTraits.hpp"

#pragma comment(lib, "WinDivert")

BOOL WINAPI Ipv4RedirectorSet(PIPV4_REDIRECTOR_CTX Ctx, 
                              PCSTR lpszRedirectFromAddress, UINT16 RedirectFromPort,
                              PCSTR lpszRedirectToAddress, UINT16 RedirectToPort) {
    ScopeGuard<CriticalSectionTraits> Guard(Ctx->CriticalSection);

    if (!IP_REDIRECTOR_HAS_FLAG(Ctx->Flags, IPV4_REDIRECTOR_CTX_FLAG_INITIALIZED)) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Guard.Lock();

    if (IP_REDIRECTOR_HAS_FLAG(Ctx->Flags, IPV4_REDIRECTOR_CTX_FLAG_WORKING)) {
        SetLastError(ERROR_INVALID_STATE);
        return FALSE;
    }

    UINT32 RedirectFromAddress;
    if (lpszRedirectFromAddress) {
        if (!WinDivertHelperParseIPv4Address(lpszRedirectFromAddress, &RedirectFromAddress))
            return FALSE;
    } else {
        RedirectFromAddress = 0;
    }
    
    UINT32 RedirectToAddress;
    if (lpszRedirectToAddress) {
        if (!WinDivertHelperParseIPv4Address(lpszRedirectToAddress, &RedirectToAddress))
            return FALSE;
    } else {
        RedirectToAddress = 0;
    }

    Ctx->RedirectFromAddress = WinDivertHelperHtonl(RedirectFromAddress);
    Ctx->RedirectFromPort = WinDivertHelperHtons(RedirectFromPort);
    Ctx->RedirectToAddress = WinDivertHelperHtonl(RedirectToAddress);
    Ctx->RedirectToPort = WinDivertHelperHtons(RedirectToPort);

    return TRUE;
}

