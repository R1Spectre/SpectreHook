#include "pch.h"
#include "squirrel.h"
#include "hooks.h"
#include "hookutils.h"
#include "sigscanning.h"
#include "concommand.h"
#include "modmanager.h"
#include <iostream>

// hook forward declarations
typedef SQInteger(*SQPrintType)(SQVM* sqvm);
SQPrintType ClientSQPrint;
SQPrintType UISQPrint;
SQPrintType ServerSQPrint;
template <ScriptContext context> SQInteger SQPrintHook(SQVM* sqvm);

typedef SQVM* (*CreateNewVMType)(void* a1, ScriptContext contextArg);
CreateNewVMType ClientCreateNewVM; // only need a client one since ui doesn't have its own func for this
CreateNewVMType ServerCreateNewVM;
template <ScriptContext context> void* CreateNewVMHook(void* a1, ScriptContext contextArg);

typedef void (*DestroyVMType)(void* a1, void* sqvm);
DestroyVMType ClientDestroyVM; // only need a client one since ui doesn't have its own func for this
DestroyVMType ServerDestroyVM;
template <ScriptContext context> void DestroyVMHook(void* a1, void* sqvm);

typedef void (*ScriptCompileError)(void* sqvm, const char* error, const char* file, int line, int column);
ScriptCompileError ClientSQCompileError; // only need a client one since ui doesn't have its own func for this
ScriptCompileError ServerSQCompileError;
template <ScriptContext context> void ScriptCompileErrorHook(void* sqvm, const char* error, const char* file, int line, int column);

typedef char (*CallScriptInitCallbackType)(void* sqvm, const char* callback);
CallScriptInitCallbackType ClientCallScriptInitCallback;
CallScriptInitCallbackType ServerCallScriptInitCallback;
template <ScriptContext context> char CallScriptInitCallbackHook(void* sqvm, const char* callback);

// core sqvm funcs
sq_compilebufferType ClientSq_compilebuffer;
sq_compilebufferType ServerSq_compilebuffer;

sq_pushroottableType ClientSq_pushroottable;
sq_pushroottableType ServerSq_pushroottable;

sq_callType ClientSq_call;
sq_callType ServerSq_call;

RegisterSquirrelFuncType ClientRegisterSquirrelFunc;
RegisterSquirrelFuncType ServerRegisterSquirrelFunc;

// sq stack array funcs
sq_newarrayType ClientSq_newarray;
sq_newarrayType ServerSq_newarray;

sq_arrayappendType ClientSq_arrayappend;
sq_arrayappendType ServerSq_arrayappend;

// sq stack push funcs
sq_pushstringType ClientSq_pushstring;
sq_pushstringType ServerSq_pushstring;

sq_pushintegerType ClientSq_pushinteger;
sq_pushintegerType ServerSq_pushinteger;

sq_pushfloatType ClientSq_pushfloat;
sq_pushfloatType ServerSq_pushfloat;

sq_pushboolType ClientSq_pushbool;
sq_pushboolType ServerSq_pushbool;

sq_pusherrorType ClientSq_pusherror;
sq_pusherrorType ServerSq_pusherror;

// sq stack get funcs
sq_getstringType ClientSq_getstring;
sq_getstringType ServerSq_getstring;

sq_getintegerType ClientSq_getinteger;
sq_getintegerType ServerSq_getinteger;

sq_getfloatType ClientSq_getfloat;
sq_getfloatType ServerSq_getfloat;

sq_getboolType ClientSq_getbool;
sq_getboolType ServerSq_getbool;

sq_getType ClientSq_sq_get;
sq_getType ServerSq_sq_get;

template <ScriptContext context> void ExecuteCodeCommand(const CCommand& args);

// inits
SquirrelManager<ScriptContext::CLIENT>* g_ClientSquirrelManager;
SquirrelManager<ScriptContext::SERVER>* g_ServerSquirrelManager;
SquirrelManager<ScriptContext::UI>* g_UISquirrelManager;

__int64 __fastcall SQFuncBindingFn(__int64(__fastcall* a1)(_QWORD), __int64 a2, _QWORD* a3, __int64 a4, __int64 a5)
{
	__int64 result; // rax

	result = ((__int64(__fastcall*)(_QWORD, __int64))a1)(*a3, a2);
	*(unsigned __int16*)(a5 + 8) = 5;
	*(_DWORD*)a5 = result;
	return result;
}

void InitialiseClientSquirrel(HMODULE baseAddress)
{
	HookEnabler hook;

	// client inits
	g_ClientSquirrelManager = new SquirrelManager<ScriptContext::CLIENT>();

	ENABLER_CREATEHOOK(
		hook,
		(char*)baseAddress + 0x56B40,
		&SQPrintHook<ScriptContext::CLIENT>,
		reinterpret_cast<LPVOID*>(&ClientSQPrint)); // client print function
	RegisterConCommand("script_client", ExecuteCodeCommand<ScriptContext::CLIENT>, "Executes script code on the client vm", FCVAR_CLIENTDLL);

	// ui inits
	g_UISquirrelManager = new SquirrelManager<ScriptContext::UI>();

	//ENABLER_CREATEHOOK(
	//	hook, (char*)baseAddress + 0x12BA0, &SQPrintHook<ScriptContext::UI>, reinterpret_cast<LPVOID*>(&UISQPrint)); // ui print function
	RegisterConCommand("script_ui", ExecuteCodeCommand<ScriptContext::UI>, "Executes script code on the ui vm", FCVAR_CLIENTDLL);

	// inits for both client and ui, since they share some functions
	ClientSq_compilebuffer = (sq_compilebufferType)((char*)baseAddress + 0x1A5E0);
	ClientSq_pushroottable = (sq_pushroottableType)((char*)baseAddress + 0x56440);
	ClientSq_call = (sq_callType)((char*)baseAddress + 0x18C40);
	ClientRegisterSquirrelFunc = (RegisterSquirrelFuncType)((char*)baseAddress + 0x8E50); // not sure

	ClientSq_newarray = (sq_newarrayType)((char*)baseAddress + 0x14FB0);
	ClientSq_arrayappend = (sq_arrayappendType)((char*)baseAddress + 0x152A0);

	ClientSq_pushstring = (sq_pushstringType)((char*)baseAddress + 0x14C30);
	ClientSq_pushinteger = (sq_pushintegerType)((char*)baseAddress + 0xEA10);
	ClientSq_pushfloat = (sq_pushfloatType)((char*)baseAddress + 0xE9B0);
	ClientSq_pushbool = (sq_pushboolType)((char*)baseAddress + 0xE930);
	//ClientSq_pusherror = (sq_pusherrorType)((char*)baseAddress + 0x8470);// can't find

	ClientSq_getstring = (sq_getstringType)((char*)baseAddress + 0x16880);
	ClientSq_getinteger = (sq_getintegerType)((char*)baseAddress + 0xE740);
	ClientSq_getfloat = (sq_getfloatType)((char*)baseAddress + 0xE6F0);
	ClientSq_getbool = (sq_getboolType)((char*)baseAddress + 0xE6B0);

	ClientSq_sq_get = (sq_getType)((char*)baseAddress + 0x17F10);

	ENABLER_CREATEHOOK(
		hook,
		(char*)baseAddress + 0x1210,
		&CreateNewVMHook<ScriptContext::CLIENT>,
		reinterpret_cast<LPVOID*>(&ClientCreateNewVM)); // client createnewvm function
	//ENABLER_CREATEHOOK(
	//	hook,
	//	(char*)baseAddress + 0x26E70,
	//	&DestroyVMHook<ScriptContext::CLIENT>,
	//	reinterpret_cast<LPVOID*>(&ClientDestroyVM)); // client destroyvm function
	ENABLER_CREATEHOOK(
		hook,
		(char*)baseAddress + 0x45440,
		&ScriptCompileErrorHook<ScriptContext::CLIENT>,
		reinterpret_cast<LPVOID*>(&ClientSQCompileError)); // client compileerror function
	//ENABLER_CREATEHOOK(
	//	hook,
	//	(char*)baseAddress + 0x25C70,
	//	&CallScriptInitCallbackHook<ScriptContext::CLIENT>,
	//	reinterpret_cast<LPVOID*>(&ClientCallScriptInitCallback)); // client callscriptinitcallback function
}

void InitialiseServerSquirrel(HMODULE baseAddress)
{
	g_ServerSquirrelManager = new SquirrelManager<ScriptContext::SERVER>();

	HookEnabler hook;

	ServerSq_compilebuffer = (sq_compilebufferType)((char*)baseAddress + 0x3110);
	ServerSq_pushroottable = (sq_pushroottableType)((char*)baseAddress + 0x5840);
	ServerSq_call = (sq_callType)((char*)baseAddress + 0x8620);
	ServerRegisterSquirrelFunc = (RegisterSquirrelFuncType)((char*)baseAddress + 0x1DD10);

	ServerSq_newarray = (sq_newarrayType)((char*)baseAddress + 0x39F0);
	ServerSq_arrayappend = (sq_arrayappendType)((char*)baseAddress + 0x3C70);

	ServerSq_pushstring = (sq_pushstringType)((char*)baseAddress + 0x3440);
	ServerSq_pushinteger = (sq_pushintegerType)((char*)baseAddress + 0x36A0);
	ServerSq_pushfloat = (sq_pushfloatType)((char*)baseAddress + 0x3800);
	ServerSq_pushbool = (sq_pushboolType)((char*)baseAddress + 0x3710);
	ServerSq_pusherror = (sq_pusherrorType)((char*)baseAddress + 0x8440);

	ServerSq_getstring = (sq_getstringType)((char*)baseAddress + 0x60A0);
	ServerSq_getinteger = (sq_getintegerType)((char*)baseAddress + 0x60C0);
	ServerSq_getfloat = (sq_getfloatType)((char*)baseAddress + 0x60E0);
	ServerSq_getbool = (sq_getboolType)((char*)baseAddress + 0x6110);

	ServerSq_sq_get = (sq_getType)((char*)baseAddress + 0x7C00);

	ENABLER_CREATEHOOK(
		hook,
		(char*)baseAddress + 0x1FE90,
		&SQPrintHook<ScriptContext::SERVER>,
		reinterpret_cast<LPVOID*>(&ServerSQPrint)); // server print function
	ENABLER_CREATEHOOK(
		hook,
		(char*)baseAddress + 0x260E0,
		&CreateNewVMHook<ScriptContext::SERVER>,
		reinterpret_cast<LPVOID*>(&ServerCreateNewVM)); // server createnewvm function
	ENABLER_CREATEHOOK(
		hook,
		(char*)baseAddress + 0x26E20,
		&DestroyVMHook<ScriptContext::SERVER>,
		reinterpret_cast<LPVOID*>(&ServerDestroyVM)); // server destroyvm function
	ENABLER_CREATEHOOK(
		hook,
		(char*)baseAddress + 0x799E0,
		&ScriptCompileErrorHook<ScriptContext::SERVER>,
		reinterpret_cast<LPVOID*>(&ServerSQCompileError)); // server compileerror function
	ENABLER_CREATEHOOK(
		hook,
		(char*)baseAddress + 0x1D5C0,
		&CallScriptInitCallbackHook<ScriptContext::SERVER>,
		reinterpret_cast<LPVOID*>(&ServerCallScriptInitCallback)); // server callscriptinitcallback function

	// cheat and clientcmd_can_execute allows clients to execute this, but since it's unsafe we only allow it when cheats are enabled
	// for script_client and script_ui, we don't use cheats, so clients can execute them on themselves all they want
	//RegisterConCommand(
	//	"script",
	//	ExecuteCodeCommand<ScriptContext::SERVER>,
	//	"Executes script code on the server vm",
	//	FCVAR_GAMEDLL | FCVAR_CLIENTCMD_CAN_EXECUTE | FCVAR_CHEAT);
}

void PrintFunc(SQVM* v, ScriptContext context, const SQChar* s, va_list args);

template <ScriptContext context> SQInteger SQPrintHook(SQVM* v)
{
	static auto printFuncLambda = [](SQVM* v, const SQChar* s, ...) {
		va_list vl;
		va_start(vl, s);
		PrintFunc(v, context, s, vl);
		va_end(vl);
	};

	SQPRINTFUNCTION oldPrintFunc = _ss(v)->_printfunc;
	_ss(v)->_printfunc = printFuncLambda;
	if (context == ScriptContext::CLIENT) ClientSQPrint(v);
	if (context == ScriptContext::UI) UISQPrint(v);
	if (context == ScriptContext::SERVER) ServerSQPrint(v);
	_ss(v)->_printfunc = oldPrintFunc; 
	SQInteger res;
	if (context == ScriptContext::CLIENT) res = ClientSQPrint(v);
	if (context == ScriptContext::UI) res = UISQPrint(v);
	if (context == ScriptContext::SERVER) res = ServerSQPrint(v);

	return res;
}

void PrintFunc(SQVM* v, ScriptContext context, const SQChar* s, va_list args)
{
	SQChar buf[1024];

	int charactersWritten = _vsnprintf_s(buf, _TRUNCATE, s, args);
	if (charactersWritten > 0)
	{
		if (buf[charactersWritten - 1] == '\n')
		{
			buf[charactersWritten - 1] = 0;
		}

		//Log::Info("[%s] %s", GetContextName(context), buf);
		Log::Info("[SCRIPT] %s", buf);
	}
}

template <ScriptContext context> void* CreateNewVMHook(void* a1, ScriptContext realContext)
{
	SQVM* sqvm;

	if (context == ScriptContext::CLIENT)
	{
		sqvm = ClientCreateNewVM(a1, realContext);

		if (realContext == ScriptContext::UI)
			g_UISquirrelManager->VMCreated(sqvm);
		else
			g_ClientSquirrelManager->VMCreated(sqvm);
	}
	else if (context == ScriptContext::SERVER)
	{
		sqvm = ServerCreateNewVM(a1, context);
		g_ServerSquirrelManager->VMCreated(sqvm);
	}

	Log::Info("CreateNewVM %s 0x%08x", GetContextName(realContext), sqvm);
	return sqvm;
}

template <ScriptContext context> void DestroyVMHook(void* a1, void* sqvm)
{
	ScriptContext realContext = context; // ui and client use the same function so we use this for prints

	if (context == ScriptContext::CLIENT)
	{
		if (g_ClientSquirrelManager->sqvm == sqvm)
			g_ClientSquirrelManager->VMDestroyed();
		else if (g_UISquirrelManager->sqvm == sqvm)
		{
			g_UISquirrelManager->VMDestroyed();
			realContext = ScriptContext::UI;
		}

		ClientDestroyVM(a1, sqvm);
	}
	else if (context == ScriptContext::SERVER)
	{
		g_ServerSquirrelManager->VMDestroyed();
		ServerDestroyVM(a1, sqvm);
	}

	// REPLACE WITH LOG //spdlog::info("DestroyVM {} {}", GetContextName(realContext), sqvm);
}

template <ScriptContext context> void ScriptCompileErrorHook(void* sqvm, const char* error, const char* file, int line, int column)
{
	ScriptContext realContext = context; // ui and client use the same function so we use this for prints
	if (context == ScriptContext::CLIENT && sqvm == g_UISquirrelManager->sqvm)
		realContext = ScriptContext::UI;

	Log::Error("%s SCRIPT COMPILE ERROR %s", GetContextName(realContext), error);
	Log::Error("%s line [%s] column [%s]", file, line, column);

	// dont call the original since it kills game
	// in the future it'd be nice to do an actual error with UICodeCallback_ErrorDialog here, but only if we're compiling level scripts
	// compilestring and stuff shouldn't tho
	// though, that also has potential to be REALLY bad if we're compiling ui scripts lol
}

template <ScriptContext context> char CallScriptInitCallbackHook(void* sqvm, const char* callback)
{
	char ret;

	if (context == ScriptContext::CLIENT)
	{

		ScriptContext realContext = context; // ui and client use the same function so we use this for prints
		bool shouldCallCustomCallbacks = false;

		// since we don't hook arbitrary callbacks yet, make sure we're only doing callbacks on inits
		if (!strcmp(callback, "UICodeCallback_UIInit"))
		{
			realContext = ScriptContext::UI;
			shouldCallCustomCallbacks = true;
		}
		else if (!strcmp(callback, "ClientCodeCallback_MapSpawn"))
			shouldCallCustomCallbacks = true;

		// run before callbacks
		// todo: we need to verify if RunOn is valid for current state before calling callbacks
		if (shouldCallCustomCallbacks)
		{
			for (Mod mod : g_ModManager->m_loadedMods)
			{
				if (!mod.Enabled)
					continue;

				for (ModScript script : mod.Scripts)
				{
					for (ModScriptCallback modCallback : script.Callbacks)
					{
						if (modCallback.Context == realContext && modCallback.BeforeCallback.length())
						{
							// REPLACE WITH LOG //spdlog::info(
							//	"Running custom {} script callback \"{}\"", GetContextName(realContext), modCallback.BeforeCallback);
							ClientCallScriptInitCallback(sqvm, modCallback.BeforeCallback.c_str());
						}
					}
				}
			}
		}

		// REPLACE WITH LOG //spdlog::info("{} CodeCallback {} called", GetContextName(realContext), callback);
		if (!shouldCallCustomCallbacks)
			// REPLACE WITH LOG //spdlog::info("Not executing custom callbacks for CodeCallback {}", callback);
		ret = ClientCallScriptInitCallback(sqvm, callback);

		// run after callbacks
		if (shouldCallCustomCallbacks)
		{
			for (Mod mod : g_ModManager->m_loadedMods)
			{
				if (!mod.Enabled)
					continue;

				for (ModScript script : mod.Scripts)
				{
					for (ModScriptCallback modCallback : script.Callbacks)
					{
						if (modCallback.Context == realContext && modCallback.AfterCallback.length())
						{
							// REPLACE WITH LOG //spdlog::info(
							//	"Running custom {} script callback \"{}\"", GetContextName(realContext), modCallback.AfterCallback);
							ClientCallScriptInitCallback(sqvm, modCallback.AfterCallback.c_str());
						}
					}
				}
			}
		}
	}
	else if (context == ScriptContext::SERVER)
	{
		// since we don't hook arbitrary callbacks yet, make sure we're only doing callbacks on inits
		bool shouldCallCustomCallbacks = !strcmp(callback, "CodeCallback_MapSpawn");

		// run before callbacks
		// todo: we need to verify if RunOn is valid for current state before calling callbacks
		if (shouldCallCustomCallbacks)
		{
			for (Mod mod : g_ModManager->m_loadedMods)
			{
				if (!mod.Enabled)
					continue;

				for (ModScript script : mod.Scripts)
				{
					for (ModScriptCallback modCallback : script.Callbacks)
					{
						if (modCallback.Context == ScriptContext::SERVER && modCallback.BeforeCallback.length())
						{
							// REPLACE WITH LOG //spdlog::info("Running custom {} script callback \"{}\"", GetContextName(context), modCallback.BeforeCallback);
							ServerCallScriptInitCallback(sqvm, modCallback.BeforeCallback.c_str());
						}
					}
				}
			}
		}

		// REPLACE WITH LOG //spdlog::info("{} CodeCallback {} called", GetContextName(context), callback);
		if (!shouldCallCustomCallbacks)
			// REPLACE WITH LOG //spdlog::info("Not executing custom callbacks for CodeCallback {}", callback);
		ret = ServerCallScriptInitCallback(sqvm, callback);

		// run after callbacks
		if (shouldCallCustomCallbacks)
		{
			for (Mod mod : g_ModManager->m_loadedMods)
			{
				if (!mod.Enabled)
					continue;

				for (ModScript script : mod.Scripts)
				{
					for (ModScriptCallback modCallback : script.Callbacks)
					{
						if (modCallback.Context == ScriptContext::SERVER && modCallback.AfterCallback.length())
						{
							// REPLACE WITH LOG //spdlog::info("Running custom {} script callback \"{}\"", GetContextName(context), modCallback.AfterCallback);
							ServerCallScriptInitCallback(sqvm, modCallback.AfterCallback.c_str());
						}
					}
				}
			}
		}
	}

	return ret;
}

template <ScriptContext context> void ExecuteCodeCommand(const CCommand& args)
{
	if (context == ScriptContext::CLIENT)
		g_ClientSquirrelManager->ExecuteCode(args.ArgS());
	else if (context == ScriptContext::UI)
		g_UISquirrelManager->ExecuteCode(args.ArgS());
	else if (context == ScriptContext::SERVER)
		g_ServerSquirrelManager->ExecuteCode(args.ArgS());
}