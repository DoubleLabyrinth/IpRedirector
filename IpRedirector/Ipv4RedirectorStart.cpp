#include "IpRedirectInternal.h"
// #include <tchar.h>
// #include <stdio.h>
#include <windivert.h>
#include "OwnedResource.hpp"
#include "ScopeGuard.hpp"
#include "ReadWriteGuard.hpp"
#include "Win32ResourceTraits.hpp"
#include "Win32LockTraits.hpp"

#pragma comment(lib, "WinDivert")

struct WinDivertHandleTraits {
    using HandleType = HANDLE;
    static inline const HandleType DefaultInvalidValue = INVALID_HANDLE_VALUE;
    static bool IsValid(const HandleType& Handle) {
        return Handle != DefaultInvalidValue;
    }
    static void Releasor(const HandleType& Handle) {
        WinDivertClose(Handle);
    }
};

// static void LogIpAndPort(PCWSTR lpszPrefix, UINT32 SrcAddr, UINT16 SrcPort, UINT32 DstAddr, UINT16 DstPort) {
//     CHAR addr0[16] = {};
//     CHAR addr1[16] = {};
//     WinDivertHelperFormatIPv4Address(WinDivertHelperNtohl(SrcAddr), addr0, 16);
//     WinDivertHelperFormatIPv4Address(WinDivertHelperNtohl(DstAddr), addr1, 16);
//     if (lpszPrefix) {
//         _tprintf_s(TEXT("%s : %hs(%u) -> %hs(%u)\n"), lpszPrefix, addr0, WinDivertHelperNtohs(SrcPort), addr1, WinDivertHelperNtohs(DstPort));
//     } else {
//         _tprintf_s(TEXT("%hs(%u) -> %hs(%u)\n"), addr0, WinDivertHelperNtohs(SrcPort), addr1, WinDivertHelperNtohs(DstPort));
//     }
// }
// 
// static void LogInterfaceIndex(PCWSTR lpszPrefix, UINT32 IfIdx, UINT32 SubIfIdx) {
//     if (lpszPrefix) {
//         _tprintf_s(TEXT("%s : Interface Index = %u, Sub Interface Index = %u\n"), lpszPrefix, IfIdx, SubIfIdx);
//     } else {
//         _tprintf_s(TEXT("Interface Index = %u, Sub Interface Index = %u\n"), IfIdx, SubIfIdx);
//     }
// }

static VOID Ipv4RedirectorUpdate(PIPV4_REDIRECTOR_CTX Ctx) {
    if (IP_REDIRECTOR_HAS_FLAG(Ctx->Flags, IPV4_REDIRECTOR_CTX_FLAG_INITIALIZED)) {
        SOCKADDR_INET BestSourceAddress = {};
        SOCKADDR_INET RedirectToAddress = {};
        MIB_IPFORWARD_ROW2 IpFwd = {};

        RedirectToAddress.si_family = AF_INET;
        RedirectToAddress.Ipv4.sin_addr.s_addr = Ctx->RedirectToAddress;

        if (GetBestRoute2(NULL, NET_IFINDEX_UNSPECIFIED, NULL, &RedirectToAddress, 0, &IpFwd, &BestSourceAddress) == NO_ERROR) {
            ReadWriteGuard<SRWLockTraits> Guard(Ctx->RwLock);
            Guard.LockAsWriter();
            Ctx->InterfaceIdxOriginal = NET_IFINDEX_UNSPECIFIED;
            Ctx->InterfaceIdxRedirected = IpFwd.InterfaceIndex;
            Ctx->SourceAddressOriginal = 0;
            Ctx->SourceAddressRedirected = BestSourceAddress.Ipv4.sin_addr.s_addr;
        }
    }
}

static VOID NETIOAPI_API_ Ipv4RedirectorIpInterfaceChangeHandler(PVOID CallerContext, PMIB_IPINTERFACE_ROW Row, MIB_NOTIFICATION_TYPE NotificationType) {
    PIPV4_REDIRECTOR_CTX Ctx = reinterpret_cast<PIPV4_REDIRECTOR_CTX>(CallerContext);
    Ipv4RedirectorUpdate(Ctx);
}

static VOID NETIOAPI_API_ Ipv4RedirectorRouteChangeHandler(PVOID CallerContext, PMIB_IPFORWARD_ROW2 Row, MIB_NOTIFICATION_TYPE NotificationType) {
    PIPV4_REDIRECTOR_CTX Ctx = reinterpret_cast<PIPV4_REDIRECTOR_CTX>(CallerContext);
    Ipv4RedirectorUpdate(Ctx);
}

static DWORD WINAPI Ipv4RedirectorWorker(PVOID lpParameters) {
    PIPV4_REDIRECTOR_CTX Ctx = reinterpret_cast<PIPV4_REDIRECTOR_CTX>(lpParameters);

    while (true) {
        if (IP_REDIRECTOR_HAS_FLAG(Ctx->Flags, IPV4_REDIRECTOR_CTX_FLAG_STOPPING))
            return 0;

        if (IP_REDIRECTOR_HAS_FLAG(Ctx->Flags, IPV4_REDIRECTOR_CTX_FLAG_WORKING) && Ctx->WinDivertHandle != INVALID_HANDLE_VALUE) {
            BYTE BytesOfPacket[65536] = {};
            UINT SizeOfPacket = sizeof(BytesOfPacket);
            WINDIVERT_ADDRESS WindivertAddress = {};
            PWINDIVERT_IPHDR Ipv4Header = NULL;
            PWINDIVERT_TCPHDR TcpHeader = NULL;
            PWINDIVERT_UDPHDR UdpHeader = NULL;

            if (WinDivertRecv(Ctx->WinDivertHandle, BytesOfPacket, SizeOfPacket, &SizeOfPacket, &WindivertAddress) && WinDivertHelperParsePacket(BytesOfPacket, SizeOfPacket, &Ipv4Header, NULL, NULL, NULL, NULL, &TcpHeader, &UdpHeader, NULL, NULL, NULL, NULL)) {

//                 if (TcpHeader)
//                     LogIpAndPort(NULL, Ipv4Header->SrcAddr, TcpHeader->SrcPort, Ipv4Header->DstAddr, TcpHeader->DstPort);
//                 if (UdpHeader)
//                     LogIpAndPort(NULL, Ipv4Header->SrcAddr, UdpHeader->SrcPort, Ipv4Header->DstAddr, UdpHeader->DstPort);
//                 LogInterfaceIndex(NULL, WindivertAddress.Network.IfIdx, WindivertAddress.Network.SubIfIdx);

                if (Ipv4Header->DstAddr == Ctx->RedirectFromAddress && (TcpHeader && TcpHeader->DstPort == Ctx->RedirectFromPort || UdpHeader && UdpHeader->DstPort == Ctx->RedirectFromPort)) {
                    ReadWriteGuard<SRWLockTraits> Guard(Ctx->RwLock);
                    
                    Guard.LockAsReader();

                    if (Ctx->InterfaceIdxOriginal == NET_IFINDEX_UNSPECIFIED || Ctx->SourceAddressOriginal == 0) {
                        Guard.UnlockAsReader();
                        Guard.LockAsWriter();
                        Ctx->InterfaceIdxOriginal = WindivertAddress.Network.IfIdx;
                        Ctx->SourceAddressOriginal = Ipv4Header->SrcAddr;
                        Guard.UnlockAsWriter();
                        Guard.LockAsReader();
                    }

                    WindivertAddress.Network.IfIdx = Ctx->InterfaceIdxRedirected;
                    Ipv4Header->SrcAddr = Ctx->SourceAddressRedirected;
                    Ipv4Header->DstAddr = Ctx->RedirectToAddress;
                    if (TcpHeader)
                        TcpHeader->DstPort = Ctx->RedirectToPort;
                    else if (UdpHeader)
                        UdpHeader->DstPort = Ctx->RedirectToPort;

                    Guard.UnlockAsReader();

                    WinDivertHelperCalcChecksums(BytesOfPacket, SizeOfPacket, &WindivertAddress, 
                                                 WINDIVERT_HELPER_NO_ICMP_CHECKSUM | WINDIVERT_HELPER_NO_ICMPV6_CHECKSUM);
                } else if (Ipv4Header->SrcAddr == Ctx->RedirectToAddress && (TcpHeader && TcpHeader->SrcPort == Ctx->RedirectToPort || UdpHeader && UdpHeader->SrcPort == Ctx->RedirectToPort)) {
                    ReadWriteGuard<SRWLockTraits> Guard(Ctx->RwLock);

                    Guard.LockAsReader();

                    WindivertAddress.Network.IfIdx = Ctx->InterfaceIdxOriginal;
                    Ipv4Header->DstAddr = Ctx->SourceAddressOriginal;
                    Ipv4Header->SrcAddr = Ctx->RedirectFromAddress;
                    if (TcpHeader)
                        TcpHeader->SrcPort = Ctx->RedirectFromPort;
                    else if (UdpHeader)
                        UdpHeader->SrcPort = Ctx->RedirectFromPort;

                    Guard.UnlockAsReader();

                    WinDivertHelperCalcChecksums(BytesOfPacket, SizeOfPacket, &WindivertAddress,
                                                 WINDIVERT_HELPER_NO_ICMP_CHECKSUM | WINDIVERT_HELPER_NO_ICMPV6_CHECKSUM);
                }

//                 if (TcpHeader)
//                     LogIpAndPort(L"Modified", Ipv4Header->SrcAddr, TcpHeader->SrcPort, Ipv4Header->DstAddr, TcpHeader->DstPort);
//                 if (UdpHeader)
//                     LogIpAndPort(L"Modified", Ipv4Header->SrcAddr, UdpHeader->SrcPort, Ipv4Header->DstAddr, UdpHeader->DstPort);
//                 LogInterfaceIndex(L"Modified", WindivertAddress.Network.IfIdx, WindivertAddress.Network.SubIfIdx);
//                 _putts(TEXT(""));

                WinDivertSend(Ctx->WinDivertHandle, BytesOfPacket, SizeOfPacket, NULL, &WindivertAddress);
            } else {
                return GetLastError();
            }
        } else {
            SwitchToThread();
        }
    }
}

BOOL WINAPI Ipv4RedirectorStart(PIPV4_REDIRECTOR_CTX Ctx) {
    ScopeGuard<CriticalSectionTraits> Guard(Ctx->CriticalSection);

    // if does not have IPV4_REDIRECTOR_CTX_FLAG_INITIALIZED, refuse to start
    if (!IP_REDIRECTOR_HAS_FLAG(Ctx->Flags, IPV4_REDIRECTOR_CTX_FLAG_INITIALIZED)) {
        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    Guard.Lock();

    // if already in working, refuse to start
    if (IP_REDIRECTOR_HAS_FLAG(Ctx->Flags, IPV4_REDIRECTOR_CTX_FLAG_WORKING)) {
        SetLastError(ERROR_INVALID_STATE);
        return FALSE;
    }

    // if either source ipv4 address or port is not ready, refuse to start
    if (Ctx->RedirectFromAddress == 0 || Ctx->RedirectFromPort == 0) {
        SetLastError(ERROR_NOT_READY);
        return FALSE;
    }

    // if either destination ipv4 address or port is not ready, refuse to start
    if (Ctx->RedirectToAddress == 0 || Ctx->RedirectToPort == 0) {
        SetLastError(ERROR_NOT_READY);
        return FALSE;
    }

    Ctx->Flags |= IPV4_REDIRECTOR_CTX_FLAG_STARTING;

    DWORD dwStatus;
    OwnedResource IpInterfaceChangeNotifyHandle(GenericHandleTraits{}, [](HANDLE Handle) { CancelMibChangeNotify2(Handle); });
    OwnedResource RouteChangeNotifyHandle(GenericHandleTraits{}, [](HANDLE Handle) { CancelMibChangeNotify2(Handle); });
    dwStatus = NotifyIpInterfaceChange(AF_INET, Ipv4RedirectorIpInterfaceChangeHandler, Ctx, TRUE, IpInterfaceChangeNotifyHandle.GetAddress());
    if (dwStatus != NO_ERROR) {
        Ctx->Flags &= ~IPV4_REDIRECTOR_CTX_FLAG_STARTING;
        SetLastError(dwStatus);
        return FALSE;
    }

    dwStatus = NotifyRouteChange2(AF_INET, Ipv4RedirectorRouteChangeHandler, Ctx, TRUE, RouteChangeNotifyHandle.GetAddress());
    if (dwStatus != NO_ERROR) {
        Ctx->Flags &= ~IPV4_REDIRECTOR_CTX_FLAG_STARTING;
        SetLastError(dwStatus);
        return FALSE;
    }
    
    CHAR szRedirectFromAddress[16] = {};
    CHAR szRedirectToAddress[16] = {};
    CHAR szFilterRule[256] = {};
    WinDivertHelperFormatIPv4Address(WinDivertHelperNtohl(Ctx->RedirectFromAddress), szRedirectFromAddress, 16);
    WinDivertHelperFormatIPv4Address(WinDivertHelperNtohl(Ctx->RedirectToAddress), szRedirectToAddress, 16);
    sprintf_s(szFilterRule, _countof(szFilterRule), "ip && (ip.DstAddr == %s && tcp.DstPort == %u || ip.SrcAddr == %s && tcp.SrcPort == %u)",
              szRedirectFromAddress,
              WinDivertHelperNtohs(Ctx->RedirectFromPort),
              szRedirectToAddress,
              WinDivertHelperNtohs(Ctx->RedirectToPort)
    );

    OwnedResource hWinDivertWorker(GenericHandleTraits{});
    OwnedResource hWinDivert(WinDivertHandleTraits{}, [](HANDLE Handle) { WinDivertShutdown(Handle, WINDIVERT_SHUTDOWN_BOTH); WinDivertClose(Handle); });
    hWinDivertWorker.TakeOver(CreateThread(NULL, 0, Ipv4RedirectorWorker, Ctx, 0, NULL));
    if (hWinDivertWorker.IsValid() == false) {
        Ctx->Flags &= ~IPV4_REDIRECTOR_CTX_FLAG_STARTING;
        return FALSE;
    }

    hWinDivert.TakeOver(WinDivertOpen(szFilterRule, WINDIVERT_LAYER_NETWORK, 0, 0));
    if (hWinDivert.IsValid() == false) {
        dwStatus = GetLastError();

        Ctx->Flags |= IPV4_REDIRECTOR_CTX_FLAG_STOPPING;
        WaitForSingleObject(hWinDivertWorker, INFINITE);
        
        Ctx->Flags &= ~IPV4_REDIRECTOR_CTX_FLAG_STOPPING;
        Ctx->Flags &= ~IPV4_REDIRECTOR_CTX_FLAG_STARTING;
        SetLastError(dwStatus);
        return FALSE;
    }

    Ctx->IpInterfaceChangeNotifyHandle = IpInterfaceChangeNotifyHandle.Transfer();
    Ctx->RouteChangeNotifyHandle = RouteChangeNotifyHandle.Transfer();
    Ctx->WinDivertHandle = hWinDivert.Transfer();
    Ctx->WinDivertWorker = hWinDivertWorker.Transfer();

    Ctx->Flags |= IPV4_REDIRECTOR_CTX_FLAG_WORKING;
    Ctx->Flags &= ~IPV4_REDIRECTOR_CTX_FLAG_STARTING;
    SwitchToThread();

    return TRUE;
}

