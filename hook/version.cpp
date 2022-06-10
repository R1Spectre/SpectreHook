#include "pch.h"
#include "squirrel.h"
#include "gameutils.h"

const char* SPECTRE_VERSION = "0.0.1-beta";

// string function R1SGetVersion()
SQRESULT SQ_GetVersion(void* sqvm)
{
	ClientSq_pushstring(sqvm, SPECTRE_VERSION, -1);
	return SQRESULT_NOTNULL;
}

void InitialiseVersion(HMODULE baseAddress)
{
	g_UISquirrelManager->AddFuncRegistration("string", "R1SGetVersion", "", "", SQ_GetVersion);
	g_ClientSquirrelManager->AddFuncRegistration("string", "R1SGetVersion", "", "", SQ_GetVersion);
}