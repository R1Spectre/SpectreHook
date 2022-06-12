#include "pch.h"
#include "miscenginefixes.h"
#include "logging.h"

typedef void (*ExitType)();
ExitType ExitOriginal;

void ExitHook() {
	AllowExitToCrash();
	ExitOriginal();
}

void InitialiseMiscEngineFixes(HMODULE baseAddress) {
	HookEnabler hook;
	ENABLER_CREATEHOOK(hook, (char*)baseAddress + 0x1365D0, &ExitHook, reinterpret_cast<LPVOID*>(&ExitOriginal));
}