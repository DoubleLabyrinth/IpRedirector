#include "IpRedirectInternal.h"
#include <windivert.h>
#include "ScopeGuard.hpp"
#include "Win32LockTraits.hpp"

#pragma comment(lib, "WinDivert")

BOOL WINAPI Ipv4RedirectorGet(PIPV4_REDIRECT_CTX Ctx, 
                              PSTR lpszRedirectFromAddress, DWORD cbRedirectFromAddress, PUINT16 lpRedirectFromPort, 
                              PSTR lpszRedirectToAddress, DWORD cbRedirectToAddress, PUINT16 lpRedirectToPort) {
    ScopeGuard<CriticalSectionTraits> Guard(Ctx->CriticalSection);
    CHAR szSourceAddress[16] = {};
    CHAR szDestinationAddress[16] = {};

    // if does not have IPV4_REDIRECTOR_CTX_FLAG_INITIALIZED, return failure
    if (!IP_REDIRECTOR_HAS_FLAG(Ctx->Flags, IPV4_REDIRECTOR_CTX_FLAG_INITIALIZED)) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Guard.Lock();

    WinDivertHelperFormatIPv4Address(WinDivertHelperNtohl(Ctx->RedirectFromAddress), szSourceAddress, 16);
    WinDivertHelperFormatIPv4Address(WinDivertHelperNtohl(Ctx->RedirectToAddress), szDestinationAddress, 16);

    if (cbRedirectFromAddress && cbRedirectFromAddress < strlen(szSourceAddress) + 1) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    if (cbRedirectToAddress && cbRedirectToAddress < strlen(szDestinationAddress) + 1) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    if (cbRedirectFromAddress)
        strcpy_s(lpszRedirectFromAddress, cbRedirectFromAddress, szSourceAddress);
    if (lpRedirectFromPort)
        *lpRedirectFromPort = WinDivertHelperNtohs(Ctx->RedirectFromPort);
    if (cbRedirectToAddress)
        strcpy_s(lpszRedirectToAddress, cbRedirectToAddress, szDestinationAddress);
    if (lpRedirectToPort)
        *lpRedirectToPort = WinDivertHelperNtohs(Ctx->RedirectToPort);

    return TRUE;
}
