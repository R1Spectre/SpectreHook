#include "pch.h"
#include "convar.h"
#include "sourceconsole.h"
#include "sourceinterface.h"
#include "concommand.h"
#include "hookutils.h"

SourceInterface<CGameConsole>* g_SourceGameConsole;

void ConCommand_toggleconsole(const CCommand& arg)
{
	if ((*g_SourceGameConsole)->IsConsoleVisible())
		(*g_SourceGameConsole)->Hide();
	else
		(*g_SourceGameConsole)->Activate();
}

typedef void (*OnCommandSubmittedType)(CConsoleDialog* consoleDialog, const char* pCommand);
OnCommandSubmittedType onCommandSubmittedOriginal;
void OnCommandSubmittedHook(CConsoleDialog* consoleDialog, const char* pCommand)
{
	consoleDialog->m_pConsolePanel->Print("] ");
	consoleDialog->m_pConsolePanel->Print(pCommand);
	consoleDialog->m_pConsolePanel->Print("\n");

	// todo: call the help command in the future

	onCommandSubmittedOriginal(consoleDialog, pCommand);
}

// called from sourceinterface.cpp in client createinterface hooks, on GameClientExports001
void InitialiseConsoleOnInterfaceCreation()
{
	(*g_SourceGameConsole)->Initialize();

	// hook OnCommandSubmitted so we print inputted commands
	HookEnabler hook;
	ENABLER_CREATEHOOK(
		hook,
		(void*)((*g_SourceGameConsole)->m_pConsole->m_vtable->OnCommandSubmitted),
		&OnCommandSubmittedHook,
		reinterpret_cast<LPVOID*>(&onCommandSubmittedOriginal));
}

void SC_ColorPrint(const SourceColor& clr, const char* pMessage)
{
	if (!g_SourceGameConsole || !(*g_SourceGameConsole)->m_bInitialized)
	{
		return;
	}

	(*g_SourceGameConsole)->m_pConsole->m_pConsolePanel->ColorPrint(clr, pMessage);
}

void SC_Print(const char* pMessage)
{
	if (!g_SourceGameConsole || !(*g_SourceGameConsole)->m_bInitialized)
	{
		return;
	}

	(*g_SourceGameConsole)->m_pConsole->m_pConsolePanel->Print(pMessage);
}

void SC_DPrint(const char* pMessage)
{
	if (!g_SourceGameConsole || !(*g_SourceGameConsole)->m_bInitialized)
	{
		return;
	}

	(*g_SourceGameConsole)->m_pConsole->m_pConsolePanel->DPrint(pMessage);
}

void InitialiseSourceConsole(HMODULE baseAddress)
{
	g_SourceGameConsole = new SourceInterface<CGameConsole>("client.dll", "GameConsole004");
	RegisterConCommand("toggleconsole", ConCommand_toggleconsole, "toggles the console", FCVAR_DONTRECORD);
}