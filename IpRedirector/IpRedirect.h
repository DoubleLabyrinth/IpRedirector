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

    typedef struct _IPV4_REDIRECTOR_CTX IPV4_REDIRECTOR_CTX, *PIPV4_REDIRECTOR_CTX;

    IP_REDIRECTOR_API
    PIPV4_REDIRECTOR_CTX WINAPI Ipv4RedirectorCreate();

    IP_REDIRECTOR_API
    BOOL WINAPI Ipv4RedirectorStart(PIPV4_REDIRECTOR_CTX Ctx);

    IP_REDIRECTOR_API
    BOOL WINAPI Ipv4RedirectorStop(PIPV4_REDIRECTOR_CTX Ctx);

    IP_REDIRECTOR_API
    BOOL WINAPI Ipv4RedirectorDestroy(PIPV4_REDIRECTOR_CTX Ctx);

    IP_REDIRECTOR_API
    BOOL WINAPI Ipv4RedirectorSet(PIPV4_REDIRECTOR_CTX Ctx,
                                  PCSTR lpszRedirectFromAddress, UINT16 RedirectFromPort,
                                  PCSTR lpszRedirectToAddress, UINT16 RedirectToPort);

    IP_REDIRECTOR_API
    BOOL WINAPI Ipv4RedirectorGet(PIPV4_REDIRECTOR_CTX Ctx,
                                  PSTR lpszRedirectFromAddress, DWORD cbRedirectFromAddress, PUINT16 lpRedirectFromPort,
                                  PSTR lpszRedirectToAddress, DWORD cbRedirectToAddress, PUINT16 lpRedirectToPort);

#ifdef __cplusplus
}
#endif

