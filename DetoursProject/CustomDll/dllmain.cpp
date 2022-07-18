#include "pch.h"
#include <stdio.h>

// allow calls to the Mhook functions
#include <detours.h>

// Declare original function
typedef ULONG(WINAPI* _NtClose)(HANDLE Handle);
_NtClose TrueNtClose = (_NtClose)GetProcAddress(GetModuleHandleW(L"ntdll"), "NtClose");

// Declare your function that will be handle a hook
ULONG WINAPI HookNtClose(HANDLE Handle)
{
    // Changing parameter of NtClose to NULL
    // See https://docs.microsoft.com/en-us/windows/win32/api/winternl/nf-winternl-ntclose

    printf("Hooked NtClose changed parameter from 0x%p to NULL\n", Handle);
    return TrueNtClose(NULL);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    if (DetourIsHelperProcess()) 
    {
        return TRUE;
    }

    LONG error;
    (void)hModule;
    (void)lpReserved;
    
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DetourRestoreAfterWith();

        printf("CustomDll.dll:"
            " Starting.\n");
        fflush(stdout);

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)TrueNtClose, HookNtClose);
        error = DetourTransactionCommit();

        if (error == NO_ERROR) {
            printf("CustomDll.dll:"
                " Detoured NtClose().\n");
        }
        else {
            printf("CustomDll.dll:"
                " Error detouring NtClose(): %ld\n", error);
        }
        break;

    case DLL_PROCESS_DETACH:
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)TrueNtClose, HookNtClose);
        error = DetourTransactionCommit();

        printf("CustomDll.dll:"
            " Removed NtClose() (result=%ld)\n", error);
        fflush(stdout);
        break;
    }
    return TRUE;
}

