//  This DLL will detour the Windows SleepEx API so that TimedSleep function
//  gets called instead.  TimedSleepEx records the before and after times, and
//  calls the real SleepEx API through the TrueSleepEx function pointer.

#include "pch.h"
#include <stdio.h>

// allow calls to the Mhook functions
#include <detours.h>

// Declare original function
static LONG dwSlept = 0;
static DWORD(WINAPI* TrueSleepEx)(DWORD dwMilliseconds, BOOL bAlertable) = SleepEx;

// Declare your function that will be handle a hook
DWORD WINAPI TimedSleepEx(DWORD dwMilliseconds, BOOL bAlertable)
{
    // See https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-sleepex
    DWORD dwBeg = GetTickCount();
    DWORD ret = TrueSleepEx(dwMilliseconds, bAlertable);
    DWORD dwEnd = GetTickCount();

    InterlockedExchangeAdd(&dwSlept, dwEnd - dwBeg);

    return ret;
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
        DetourAttach(&(PVOID&)TrueSleepEx, TimedSleepEx);
        error = DetourTransactionCommit();

        if (error == NO_ERROR) {
            printf("CustomDll.dll:"
                " Detoured SleepEx().\n");
        }
        else {
            printf("CustomDll.dll:"
                " Error detouring SleepEx(): %ld\n", error);
        }
        break;

    case DLL_PROCESS_DETACH:
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourDetach(&(PVOID&)TrueSleepEx, TimedSleepEx);
        error = DetourTransactionCommit();

        printf("CustomDll.dll:"
            " Removed SleepEx() (result=%ld), slept %ld ticks.\n", error, dwSlept);
        fflush(stdout);
        break;
    }
    return TRUE;
}

