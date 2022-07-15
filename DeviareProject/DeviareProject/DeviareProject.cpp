#define WIN32_LEAN_AND_MEAN
#include <iostream>

// allow calls to the Deviare functions
#include <NktHookLib.h>
#if _WIN64
#pragma comment(lib, "NktHookLib64")
#else
#pragma comment(lib, "NktHookLib")
#endif


// Declare original and hooked functions
typedef ULONG(WINAPI* lpfnNtClose)(__in HANDLE Handle);
static ULONG WINAPI HookNtClose(__in HANDLE Handle);

static struct 
{
    SIZE_T nHookId;
    lpfnNtClose fnHookedNtClose;
} sNtClose_Hook = { 0, NULL };

// Declare your function that will be handle a hook
static ULONG WINAPI HookNtClose(__in HANDLE Handle)
{
    // Changing parameter of NtClose to NULL
    // See https://docs.microsoft.com/en-us/windows/win32/api/winternl/nf-winternl-ntclose
   
    printf("Hooked NtClose changed parameter from 0x%p to NULL\n", Handle);
    return sNtClose_Hook.fnHookedNtClose(NULL);
}

int main()
{
    CNktHookLib cHookMgr;
    HINSTANCE ntDll;
    LPVOID fnOrigNtClose;
    DWORD dwOsErr = -1;

    cHookMgr.SetEnableDebugOutput(TRUE);

    ntDll = NktHookLibHelpers::GetModuleBaseAddress(L"ntdll.dll");
    if (ntDll == NULL)
    {
        printf("Error: Cannot get handle of ntdll.dll\n");
        return -1;
    }

    fnOrigNtClose = NktHookLibHelpers::GetProcedureAddress(ntDll, "NtClose");
    if (fnOrigNtClose == NULL)
    {
        printf("Error: Cannot get address of NtClose\n");
        return -1;
    }

    dwOsErr = cHookMgr.Hook(&(sNtClose_Hook.nHookId), (LPVOID*)&(sNtClose_Hook.fnHookedNtClose), fnOrigNtClose, HookNtClose, 0);
    if (FAILED(dwOsErr))
    {
        printf("Error: Cannot set hook for NtClose\n");
        return -1;
    }

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

    CloseHandle(hFile);  // Calling to the hooked function

    dwOsErr = cHookMgr.Unhook(sNtClose_Hook.nHookId);
    if (FAILED(dwOsErr))
    {
        printf("Error: Cannot unhook the NtClose\n");
        return -1;
    }

    if (CloseHandle(hFile)) // Calling the original NtClose function
    {
        printf("Original NtClose function was called for 0x%p\n", hFile);
    }

    return 0;
}
