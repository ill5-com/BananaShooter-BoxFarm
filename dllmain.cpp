// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

// steam api shit
typedef int32_t SteamInventoryResult_t;
typedef int32_t SteamItemDef_t;
static const SteamInventoryResult_t k_SteamInventoryResultInvalid = -1;

// game specific
static const SteamItemDef_t itemDropListId = 0xB; 
static const uintptr_t triggerItemDropWrapperOffset = 0x3e4260;

// function definition for Unity wrapper for ISteamInventory::TriggerItemDrop
typedef bool (__fastcall* TRIGGERITEMDROPWRAPPER)(SteamInventoryResult_t* pResultHandle, SteamItemDef_t dropListDefinition);

// functions :-)
BOOL WINAPI MainThread(HMODULE hModule)
{
    bool shouldRunHack = true;

    // allocate console
    AllocConsole();

    // get stuff ready so we can print
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    // whatever
    SetConsoleTitleA("");

    // debug message
    printf("Hello from www.ill5.com's Banana Box farmer!\nPress END to uninject!\n\n");
    
    TRIGGERITEMDROPWRAPPER TriggerItemDropWrapper = nullptr;
    uintptr_t gameAssemblyBaseAddress = reinterpret_cast<uintptr_t>(GetModuleHandleA("gameassembly"));
    if (gameAssemblyBaseAddress == NULL)
    {
        printf("Could not find gameassembly.dll!\n");
        shouldRunHack = false;
    }
    else
    {
        printf("gameassembly.dll: 0x%016llx\n", gameAssemblyBaseAddress);
        TriggerItemDropWrapper = reinterpret_cast<TRIGGERITEMDROPWRAPPER>(gameAssemblyBaseAddress + triggerItemDropWrapperOffset);
    }

    // sanity checking, making sure the function we're calling is within somewhat sane memory regions
    // also make sure the first byte is what we expect of the target function
    if (TriggerItemDropWrapper == nullptr
        || reinterpret_cast<uintptr_t>(TriggerItemDropWrapper) <= triggerItemDropWrapperOffset
        || *reinterpret_cast<BYTE*>(TriggerItemDropWrapper) != 0x48)
    {
        printf("Could not find TriggerItemDropWrapper!\n");
        shouldRunHack = false;
    }
    else
    {
        printf("TriggerItemDropWrapper: 0x%016llx\n", reinterpret_cast<uintptr_t>(TriggerItemDropWrapper));
    }

    // newline for aesthetics
    printf("\n");

    if (shouldRunHack)
    {
        printf("Everything seems OK, starting farmer...\n");
    }
    else
    {
        printf("Seems like something went wrong! No farming today...\n");
    }

    time_t nextDrop = 0;
    bool shouldBreakLoop = false;
    while (!shouldBreakLoop)
    {
        time_t currentTime = time(NULL);

        if (shouldRunHack && currentTime >= nextDrop)
        {
            // ISteamInventory::TriggerItemDrop requires a SteamInventoryResult_t pointer passed to it to write into
            // we don't actually do anything with it, but it's needed for the API call
            void* tempPtr = malloc(sizeof(void*));
            bool result = TriggerItemDropWrapper(reinterpret_cast<SteamInventoryResult_t*>(tempPtr), itemDropListId);
            free(tempPtr);

            nextDrop = currentTime + 15;
        }

        if (GetAsyncKeyState(VK_END)
            && GetForegroundWindow() == GetConsoleWindow())
        {
            shouldBreakLoop = true;
        }

        Sleep(15);
    }

    FreeConsole();

    if (f != NULL)
        fclose(f);

    FreeLibraryAndExitThread(hModule, 0);

    return TRUE;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        HANDLE thread = CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(MainThread), hModule, NULL, NULL);
        if (thread)
            CloseHandle(thread);
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
