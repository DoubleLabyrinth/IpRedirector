#include <tchar.h>
#include <stdio.h>
#include <windows.h>
#include <IpRedirect.h>
#include "OwnedResource.hpp"
#include "Win32ResourceTraits.hpp"

#define LOG_SUCCESS(tab, fmt, ...) tab ? _tprintf_s(TEXT("%*c[+] " fmt "\n"), tab * 4, ' ', __VA_ARGS__) : _tprintf_s(TEXT("[+] " fmt "\n"), __VA_ARGS__)
#define LOG_FAILURE(tab, fmt, ...) tab ? _tprintf_s(TEXT("%*c[-] " fmt "\n"), tab * 4, ' ', __VA_ARGS__) : _tprintf_s(TEXT("[-] " fmt "\n"), __VA_ARGS__)
#define LOG_HINT(tab, fmt, ...) tab ? _tprintf_s(TEXT("%*c[*] " fmt "\n"), tab * 4, ' ', __VA_ARGS__) : _tprintf_s(TEXT("[*] " fmt "\n"), __VA_ARGS__)

int _tmain(int argc, PTSTR argv[]) {
    if (argc == 5) {
        DWORD dwStatus = ERROR_SUCCESS;
        PIPV4_REDIRECT_CTX Ctx = NULL;

#if defined(UNICODE) || defined(_UNICODE)
        OwnedResource RedirectFromAddress(HeapAllocTraits{});
        OwnedResource RedirectToAddress(HeapAllocTraits{});

        {
            int ReqSize;

            ReqSize = WideCharToMultiByte(CP_ACP, 0, argv[1], -1, NULL, 0, NULL, NULL);
            if (ReqSize == 0) {
                dwStatus = GetLastError();
                LOG_FAILURE(0, "WideCharToMultiByte failed. CODE: 0x%.8x", dwStatus);
                return dwStatus;
            }

            RedirectFromAddress.TakeOver(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ReqSize));
            if (RedirectFromAddress.IsValid() == false) {
                dwStatus = GetLastError();
                LOG_FAILURE(0, "HeapAlloc failed. CODE: 0x%.8x", dwStatus);
                return dwStatus;
            }

            if (!WideCharToMultiByte(CP_ACP, 0, argv[1], -1, RedirectFromAddress.template As<PSTR>(), ReqSize, NULL, NULL)) {
                dwStatus = GetLastError();
                LOG_FAILURE(0, "WideCharToMultiByte failed. CODE: 0x%.8x", dwStatus);
                return dwStatus;
            }

            ReqSize = WideCharToMultiByte(CP_ACP, 0, argv[3], -1, NULL, 0, NULL, NULL);
            if (ReqSize == 0) {
                dwStatus = GetLastError();
                LOG_FAILURE(0, "WideCharToMultiByte failed. CODE: 0x%.8x", dwStatus);
                return dwStatus;
            }

            RedirectToAddress.TakeOver(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ReqSize));
            if (RedirectToAddress.IsValid() == false) {
                dwStatus = GetLastError();
                LOG_FAILURE(0, "HeapAlloc failed. CODE: 0x%.8x", dwStatus);
                return dwStatus;
            }

            if (!WideCharToMultiByte(CP_ACP, 0, argv[3], -1, RedirectToAddress.template As<PSTR>(), ReqSize, NULL, NULL)) {
                dwStatus = GetLastError();
                LOG_FAILURE(0, "WideCharToMultiByte failed. CODE: 0x%.8x", dwStatus);
                return dwStatus;
            }
        }
#endif

        UINT16 RedirectFromPort;
        UINT16 RedirectToPort;

        {
            unsigned long PortValue;
            
            PortValue = _tcstoul(argv[2], NULL, 0);
            if (0 < PortValue && PortValue < 0x10000) {
                RedirectFromPort = static_cast<UINT16>(PortValue);
            } else {
                LOG_FAILURE(0, "Invalid port value for RedirectFromPort.");
                return -1;
            }

            PortValue = _tcstoul(argv[4], NULL, 0);
            if (0 < PortValue && PortValue < 0x10000) {
                RedirectToPort = static_cast<UINT16>(PortValue);
            } else {
                LOG_FAILURE(0, "Invalid port value for RedirectToPort.");
                return -1;
            }
        }
        
        Ctx = Ipv4RedirectorCreate();
        if (Ctx == NULL) {
            dwStatus = GetLastError();
            LOG_FAILURE(0, "Ipv4RedirectorCreate failed. CODE: 0x%.8x", dwStatus);
            return dwStatus;
        } else {
            LOG_SUCCESS(0, "Ipv4RedirectorCreate succeeded.");
        }

#if defined(UNICODE) || defined(_UNICODE)
        if (!Ipv4RedirectorSet(Ctx, RedirectFromAddress.template As<PSTR>(), RedirectFromPort, RedirectToAddress.template As<PSTR>(), RedirectToPort)) {
#else
        if (!Ipv4RedirectorSet(Ctx, argv[1], RedirectFromPort, argv[3], RedirectToPort)) {
#endif
            dwStatus = GetLastError();
            LOG_FAILURE(0, "Ipv4RedirectorSet failed. CODE: 0x%.8x", dwStatus);
            return dwStatus;
        } else {
            LOG_SUCCESS(0, "Ipv4RedirectorSet succeeded.");
        }

        if (!Ipv4RedirectorStart(Ctx)) {
            dwStatus = GetLastError();
            LOG_FAILURE(0, "Ipv4RedirectorStart failed. CODE: 0x%.8x", dwStatus);
            return dwStatus;
        } else {
            LOG_SUCCESS(0, "Ipv4RedirectorStart succeeded.");
        }

        _gettchar();

        if (!Ipv4RedirectorStop(Ctx)) {
            dwStatus = GetLastError();
            LOG_FAILURE(0, "Ipv4RedirectorStop failed. CODE: 0x%.8x", dwStatus);
            return 0;
        } else {
            LOG_SUCCESS(0, "Ipv4RedirectorStop succeeded.");
        }

        if (!Ipv4RedirectorDestroy(Ctx)) {
            dwStatus = GetLastError();
            LOG_FAILURE(0, "Ipv4RedirectorDestroy failed. CODE: 0x%.8x", dwStatus);
            return dwStatus;
        } else {
            LOG_SUCCESS(0, "Ipv4RedirectorDestroy succeeded.");
        }

        return 0;
    } else {
        _putts(TEXT("Usage:"));
        _putts(TEXT("    Ipv4RedirectorDemo.exe <Redirect from IP> <Redirect from port> <Redirect to IP> <Redirect to port>"));
        _putts(TEXT(""));
        return -1;
    }
}

