#include "IpRedirectInternal.h"
#include "OwnedResource.hpp"
#include "Win32ResourceTraits.hpp"

PIPV4_REDIRECT_CTX WINAPI Ipv4RedirectorCreate() {
    OwnedResource Ctx(HeapAllocTraits{});

    Ctx.TakeOver(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IPV4_REDIRECT_CTX)));
    if (Ctx.IsValid() == false)
        return nullptr;

    InitializeCriticalSection(&Ctx.template As<PIPV4_REDIRECT_CTX>()->CriticalSection);
    InitializeSRWLock(&Ctx.template As<PIPV4_REDIRECT_CTX>()->RwLock);
    Ctx.template As<PIPV4_REDIRECT_CTX>()->Flags = 0;
    Ctx.template As<PIPV4_REDIRECT_CTX>()->WinDivertHandle = INVALID_HANDLE_VALUE;
    Ctx.template As<PIPV4_REDIRECT_CTX>()->WinDivertWorker = NULL;
    Ctx.template As<PIPV4_REDIRECT_CTX>()->Flags |= IPV4_REDIRECTOR_CTX_FLAG_INITIALIZED;

    return Ctx.template TransferAs<PIPV4_REDIRECT_CTX>();
}

