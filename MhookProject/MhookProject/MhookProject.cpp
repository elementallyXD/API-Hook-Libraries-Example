#include <iostream>
#include <Windows.h>

// allow calls to the Mhook functions
#include <mhook-lib/mhook.h> 
#pragma comment(lib, "mhook")

// Declare original function
typedef ULONG(WINAPI* _NtClose)(IN HANDLE Handle);
_NtClose TrueNtClose = (_NtClose)GetProcAddress(GetModuleHandleW(L"ntdll"), "NtClose");

// Declare your function that will be handle a hook
ULONG WINAPI HookNtClose(HANDLE hHandle)
{
    // Changing parameter of NtClose to NULL
    // See https://docs.microsoft.com/en-us/windows/win32/api/winternl/nf-winternl-ntclose
   
    printf("Hooked NtClose changed parameter from 0x%p to NULL\n", hHandle);
    return TrueNtClose(NULL);
}

int main()
{
    // Creating handle for testing
    HANDLE hFile = CreateFileW(
        L"file.txt",
        GENERIC_WRITE,
        NULL,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (hFile != INVALID_HANDLE_VALUE && hFile != NULL)
    {
        printf("Created file with handle: 0x%p\n", hFile);
    }

    const BOOL isHookSet = Mhook_SetHook((PVOID*)&TrueNtClose, HookNtClose);
    if (isHookSet)
    {
        CloseHandle(hFile); // Calling to the hooked function

        Mhook_Unhook((PVOID*)&TrueNtClose); // After finishing using the hook – remove it
    }

    if (CloseHandle(hFile)) // Calling the original NtClose function
    {
        printf("Original NtClose function was called for 0x%p\n", hFile);
    }

    return 0;
}
