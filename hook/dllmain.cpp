#include "pch.h"
#include "main.h"
#include "hooks.h"
#include <string.h>

#include "logging.h"
#include "gameutils.h"
#include "configurables.h"
#include "modmanager.h"
#include "squirrel.h"
#include "keyvalues.h"
#include "filesystem.h"
#include "sourceconsole.h"
#include "masterserver.h"
#include "version.h"
#include "miscenginefixes.h"

bool initialised = false;

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

void WaitForDebugger(HMODULE baseAddress)
{
    // earlier waitfordebugger call than is in vanilla, just so we can debug stuff a little easier
    if (CommandLine()->CheckParm("-waitfordebugger"))
    {
        Log::Info("Waiting for debugger...");

        while (!IsDebuggerPresent())
            Sleep(100);
    }
}

bool InitialiseSpectre()
{
    if (initialised)
    {
        return false;
    }

    initialised = true;

    InitialiseLogging();

    InstallInitialHooks();

    ParseConfigurables();

    InitialiseInterfaceCreationHooks();

    AddDllLoadCallback("tier0.dll", InitialiseLogHooks);
    AddDllLoadCallback("tier0.dll", InitialiseTier0GameUtilFunctions);
    AddDllLoadCallback("engine.dll", WaitForDebugger);

    AddDllLoadCallback("engine.dll", InitialiseConVars);
    AddDllLoadCallback("engine.dll", InitialiseConCommands);
    AddDllLoadCallback("engine.dll", InitialiseMiscEngineFixes);

    // client-exclusive patches
    {
        AddDllLoadCallbackForClient("client.dll", InitialiseSquirrel);
        AddDllLoadCallbackForClient("client.dll", InitialiseSourceConsole);
    }

    //AddDllLoadCallback("server.dll", InitialiseServerSquirrel);

    AddDllLoadCallback("client.dll", InitialiseVersion);

    AddDllLoadCallback("filesystem_stdio.dll", InitialiseFilesystem);
    AddDllLoadCallback("engine.dll", InitialiseKeyValues);

    AddDllLoadCallback("engine.dll", InitialiseEngineMasterServer);
    AddDllLoadCallback("client.dll", InitialiseClientMasterServer);

    // mod manager after everything else
    AddDllLoadCallback("engine.dll", InitialiseModManager);

    // run callbacks for any libraries that are already loaded by now
    CallAllPendingDLLLoadCallbacks();

    return true;
}