#pragma once
#include <windows.h>

#define IP_REDIRECTOR_IMPORT
#ifdef IP_REDIRECTOR_IMPORT
#define IP_REDIRECTOR_API __declspec(dllimport)
#else
#define IP_REDIRECTOR_API __declspec(dllexport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct _IPV4_REDIRECTOR_CTX IPV4_REDIRECT_CTX, *PIPV4_REDIRECT_CTX;

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
                                  PCSTR lpszSourceAddress, UINT16 SourcePort,
                                  PCSTR lpszDestinationAddress, UINT16 DestinationPort);

    IP_REDIRECTOR_API
    BOOL WINAPI Ipv4RedirectorGet(PIPV4_REDIRECT_CTX Ctx,
                                  PSTR lpszSourceAddress, DWORD cbSourceAddress, PUINT16 lpSourcePort,
                                  PSTR lpszDestinationAddress, DWORD cbDestinationAddress, PUINT16 lpDestinationPort);

#ifdef __cplusplus
}
#endif

