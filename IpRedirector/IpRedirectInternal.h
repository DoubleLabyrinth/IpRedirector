#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <netioapi.h>
#include <iphlpapi.h>
#include <windows.h>

#pragma comment(lib, "Iphlpapi")

#ifdef IP_REDIRECTOR_IMPORT
#define IP_REDIRECTOR_API __declspec(dllimport)
#else
#define IP_REDIRECTOR_API __declspec(dllexport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define IP_REDIRECTOR_HAS_FLAG(v, f) (v & f)
#define IPV4_REDIRECTOR_IS_LOOPBACK(addr) ((addr & 0x000000ffu) == 0x0000007f)      // addr must be in network-byte-order

#define IPV4_REDIRECTOR_CTX_FLAG_INITIALIZED  0x00000001
#define IPV4_REDIRECTOR_CTX_FLAG_STARTING     0x00000002
#define IPV4_REDIRECTOR_CTX_FLAG_STOPPING     0x00000004
#define IPV4_REDIRECTOR_CTX_FLAG_WORKING      0x00000008

    typedef struct _IPV4_REDIRECTOR_CTX {
        CRITICAL_SECTION CriticalSection;

        UINT32 RedirectFromAddress; // network-byte-order
        UINT16 RedirectFromPort;    // network-byte-order
        UINT32 RedirectToAddress;   // network-byte-order
        UINT16 RedirectToPort;      // network-byte-order

        // Update when route changes or IP interface changes
        SRWLOCK RwLock;
        NET_IFINDEX InterfaceIdxOriginal;
        NET_IFINDEX InterfaceIdxRedirected;
        UINT32 SourceAddressOriginal;  // network-byte-order
        UINT32 SourceAddressRedirected;  // network-byte-order

        UINT32 Flags;

        HANDLE IpInterfaceChangeNotifyHandle;
        HANDLE RouteChangeNotifyHandle;
        HANDLE WinDivertHandle;
        HANDLE WinDivertWorker;
    } IPV4_REDIRECT_CTX, *PIPV4_REDIRECT_CTX;

    IP_REDIRECTOR_API 
    PIPV4_REDIRECT_CTX WINAPI Ipv4RedirectorCreate();

    IP_REDIRECTOR_API
    BOOL WINAPI Ipv4RedirectorStart(PIPV4_REDIRECT_CTX Ctx);

    IP_REDIRECTOR_API
    BOOL WINAPI Ipv4RedirectorStop(PIPV4_REDIRECT_CTX Ctx);

    IP_REDIRECTOR_API
    BOOL WINAPI Ipv4RedirectorDestroy(PIPV4_REDIRECT_CTX Ctx);

    IP_REDIRECTOR_API
    BOOL WINAPI Ipv4RedirectorSet(PIPV4_REDIRECT_CTX Ctx,
                                  PCSTR lpszRedirectFromAddress, UINT16 RedirectFromPort,
                                  PCSTR lpszRedirectToAddress, UINT16 RedirectToPort);

    IP_REDIRECTOR_API
    BOOL WINAPI Ipv4RedirectorGet(PIPV4_REDIRECT_CTX Ctx,
                                  PSTR lpszRedirectFromAddress, DWORD cbRedirectFromAddress, PUINT16 lpRedirectFromPort,
                                  PSTR lpszRedirectToAddress, DWORD cbRedirectToAddress, PUINT16 lpRedirectToPort);

#ifdef __cplusplus
}
#endif

