#include <string>
#include <iostream>
#include <Windows.h>

#include <easyhook.h> // allow calls to the EasyHook functions

using namespace std;

#if _WIN64
#pragma comment(lib, "EasyHook64.lib")
#else
#pragma comment(lib, "EasyHook32.lib")
#endif

void WINAPI SleepHook(DWORD)
{
    // Changing the sleep duration to 5 seconds
    // See https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-sleep
    
    cout << "Sleep duration changed to 5 seconds\n";
    ::Sleep(5000);
}

int main()
{
    HOOK_TRACE_INFO hHook = { NULL }; // keep track of hooks
    NTSTATUS result = LhInstallHook( // Install the hook
        GetProcAddress(GetModuleHandleW(TEXT("kernel32")), "Sleep"), // Here we providing function that want to hook
        SleepHook,
        NULL,
        &hHook);
    if (FAILED(result))
    {
        wstring s(RtlGetLastErrorString());
        wcout << L"Failed to install hook: ";
        wcout << s;
        cout << "\n\nPress any key to exit.";
        cin.get();
        return -1;
    }
    ULONG ACLEntries[1] = { 0 }; // If the threadId in the ACL is set to 0, then internally EasyHook uses GetCurrentThreadId()
    LhSetInclusiveACL(ACLEntries, 1, &hHook); // Activates hook for the thread

    Sleep (3000); // Call function that we hooking with 3 sec duration

    // Unistall hooks
    LhUninstallHook(&hHook);
    LhWaitForPendingRemovals();

    cout << "Sleep duration is 3 sec";
    Sleep(3000); // Actually Sleep for 3 sec

    return 0;
}
