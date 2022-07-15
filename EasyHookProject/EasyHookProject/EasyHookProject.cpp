#include <iostream>

// allow calls to the EasyHook functions
#include <easyhook.h> 
#if _WIN64
#pragma comment(lib, "EasyHook64.lib")
#else
#pragma comment(lib, "EasyHook32.lib")
#endif

// Declare your function that will be handle a hook
ULONG WINAPI HookNtClose(HANDLE hHandle)
{
    // Changing parameter of NtClose to NULL
    // See https://docs.microsoft.com/en-us/windows/win32/api/winternl/nf-winternl-ntclose

    printf("Hooked NtClose changed parameter from 0x%p to NULL\n", hHandle);
    return CloseHandle(NULL);
}

int main()
{
    HOOK_TRACE_INFO hHook = { NULL }; // keep track of hooks
    NTSTATUS result = LhInstallHook( // Install the hook
        GetProcAddress(GetModuleHandleW(TEXT("ntdll")), "NtClose"), // Here we providing function that want to hook
        HookNtClose,
        NULL,
        &hHook);
    if (FAILED(result))
    {
        std::wstring s(RtlGetLastErrorString());
        std::wcout << L"Failed to install hook: ";
        std::wcout << s;
        std::cout << "\n\nPress any key to exit.";
        std::cin.get();
        return -1;
    }
    ULONG ACLEntries[1] = { 0 }; // If the threadId in the ACL is set to 0, then internally EasyHook uses GetCurrentThreadId()
    LhSetInclusiveACL(ACLEntries, 1, &hHook); // Activates hook for the thread

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
    
    CloseHandle(hFile); // Calling to the hooked function

    // Unistall hooks
    LhUninstallHook(&hHook);
    LhWaitForPendingRemovals();

    if (CloseHandle(hFile)) // Calling the original NtClose function
    {
        printf("Original NtClose function was called for 0x%p\n", hFile);
    }

    return 0;
}
