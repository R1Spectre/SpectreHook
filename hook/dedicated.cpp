#include "pch.h"
#include "dedicated.h"
#include "hookutils.h"

bool IsDedicated()
{
	static bool result = strstr(GetCommandLineA(), "-dedicated");
	return result;
}

// CDedidcatedExports defs
struct CDedicatedExports; // forward declare

typedef void (*DedicatedSys_PrintfType)(CDedicatedExports* dedicated, const char* msg);
typedef void (*DedicatedRunServerType)(CDedicatedExports* dedicated);

// would've liked to just do this as a class but have not been able to get it to work
struct CDedicatedExports
{
	void* vtable; // because it's easier, we just set this to &this, since CDedicatedExports has no props we care about other than funcs

	char unused[56];

	DedicatedSys_PrintfType Sys_Printf;
	DedicatedRunServerType RunServer;
};

void Sys_Printf(CDedicatedExports* dedicated, const char* msg)
{
	Log::Info("[DEDICATED PRINT] %s", msg);
}

typedef bool (*IsGameActiveWindowType)();
IsGameActiveWindowType IsGameActiveWindow;
bool IsGameActiveWindowHook()
{
	return true;
}