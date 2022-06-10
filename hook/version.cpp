#include "pch.h"
#include "squirrel.h"
#include "gameutils.h"

const char* SPECTRE_VERSION = "0.0.1-beta";

// string function R1SGetVersion()
SQRESULT SQ_GetVersion(HSQUIRRELVM sqvm)
{
	sq_pushstring(sqvm, SPECTRE_VERSION, -1);
	return 1;
}

void InitialiseVersion(HMODULE baseAddress)
{
	g_SquirrelManager->AddFuncRegistration(SCRIPT_CONTEXT_UI, "R1SGetVersion", &SQ_GetVersion, ".", 0, "string", "", "Gets the Spectre version string");
}