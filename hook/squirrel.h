#pragma once

void InitialiseClientSquirrel(HMODULE baseAddress);
void InitialiseServerSquirrel(HMODULE baseAddress);

// stolen from ttf2sdk: sqvm types
typedef float SQFloat;
typedef long SQInteger;
typedef unsigned long SQUnsignedInteger;
typedef char SQChar;

typedef SQUnsignedInteger SQBool;
typedef SQInteger SQRESULT;

const SQRESULT SQRESULT_ERROR = -1;
const SQRESULT SQRESULT_NULL = 0;
const SQRESULT SQRESULT_NOTNULL = 1;

typedef SQInteger(*SQFunction)(void* sqvm);

struct CompileBufferState
{
	const SQChar* buffer;
	const SQChar* bufferPlusLength;
	const SQChar* bufferAgain;

	CompileBufferState(const std::string& code)
	{
		buffer = code.c_str();
		bufferPlusLength = code.c_str() + code.size();
		bufferAgain = code.c_str();
	}
};

struct SQFuncRegistrationInternal {
	const char* squirrelFuncName; //0x0000 
	const char* cppFuncName; //0x0008 
	const char* helpText; //0x0010 

	// stuff for flags&2
	// first parameter here is "this", params counter includes it
	// nparamscheck 0 = don't check params, number = check for exact number of params, -number = check for at least abs(number) params
	// . = any; o = null (?); i = integer; f = float; n = integer or float; s = string; t = table; a = array; u = userdata; c = closure/nativeclosure; g = generator; p = userpointer; v = thread; x = class instance; y = class; b = bool
	// at least those are supported for sure: nxbs. (number, instance, boolean, string, any)
	// [untested] #define SQ_MATCHTYPEMASKSTRING (-99999) = If SQ_MATCHTYPEMASKSTRING is passed instead of the number of parameters, the function will automatically extrapolate the number of parameters to check from the typemask(eg. if the typemask is ”.sn” is like passing 3).
	const char* szTypeMask; //0x0018 can be: .s, .ss., .., ..  
	__int64 nparamscheck_probably; //0x0020 can be: 2, -3, 2, 0  

	const char* returnValueTypeText; //0x0028 
	const char* argNamesText; //0x0030 

	__int64 UnkSeemsToAlwaysBe32; //0x0038 
	char pad_0x0040[0x28]; //0x0040 // CUtlVector of parameter types, can't be arsed to reverse this
	void* pfnBinding; //0x0068 
	void* pFunction; //0x0070 
	__int64 flags; //0x0078 // if it's 2, then the CUtlVector mentioned above will not be used

	SQFuncRegistrationInternal()
	{
		memset(this, 0, sizeof(SQFuncRegistrationInternal));
		this->UnkSeemsToAlwaysBe32 = 32;
	}
};


__int64 __fastcall SQFuncBindingFn(__int64(__fastcall* a1)(_QWORD), __int64 a2, _QWORD* a3, __int64 a4, __int64 a5);

class SQFuncRegistration
{
public:
	SQFuncRegistration(
		ScriptContext context,
		const std::string& name,
		SQFunction funcPtr,
		const std::string& szTypeMask,
		int nParamsCheck,
		const std::string& returnType,
		const std::string& argNames,
		const std::string& helpText
	) :
		m_context(context),
		m_funcName(name),
		m_szTypeMask(szTypeMask),
		m_retValueType(returnType),
		m_argNames(argNames),
		m_helpText(helpText)
	{
		m_internalReg.squirrelFuncName = m_internalReg.cppFuncName = m_funcName.c_str();
		m_internalReg.helpText = m_helpText.c_str();
		m_internalReg.szTypeMask = m_szTypeMask.c_str();
		m_internalReg.nparamscheck_probably = nParamsCheck;
		m_internalReg.returnValueTypeText = m_retValueType.c_str();
		m_internalReg.argNamesText = m_argNames.c_str();
		m_internalReg.pfnBinding = SQFuncBindingFn;
		m_internalReg.pFunction = funcPtr;
		m_internalReg.flags = 2;
	}

	ScriptContext GetContext() const
	{
		return m_context;
	}

	SQFuncRegistrationInternal* GetInternalReg()
	{
		return &m_internalReg;
	}

	const std::string& GetName() const
	{
		return m_funcName;
	}

private:
	ScriptContext m_context;
	std::string m_funcName;
	std::string m_helpText;
	std::string m_szTypeMask;
	std::string m_retValueType;
	std::string m_argNames;
	SQFuncRegistrationInternal m_internalReg;
};

// core sqvm funcs
typedef SQRESULT(*sq_compilebufferType)(void* sqvm, const SQChar* code, SQInteger length, const SQChar* a4, SQBool a5);
extern sq_compilebufferType ClientSq_compilebuffer;
extern sq_compilebufferType ServerSq_compilebuffer;

typedef void (*sq_pushroottableType)(void* sqvm);
extern sq_pushroottableType ClientSq_pushroottable;
extern sq_pushroottableType ServerSq_pushroottable;

typedef SQRESULT(*sq_callType)(void* sqvm, SQInteger s1, SQBool a2, SQBool a3);
extern sq_callType ClientSq_call;
extern sq_callType ServerSq_call;

typedef int64_t(*RegisterSquirrelFuncType)(void* sqvm, SQFuncRegistrationInternal* funcReg);
extern RegisterSquirrelFuncType ClientRegisterSquirrelFunc;
extern RegisterSquirrelFuncType ServerRegisterSquirrelFunc;

// sq stack array funcs
typedef void (*sq_newarrayType)(void* sqvm, SQInteger stackpos);
extern sq_newarrayType ClientSq_newarray;
extern sq_newarrayType ServerSq_newarray;

typedef SQRESULT(*sq_arrayappendType)(void* sqvm, SQInteger stackpos);
extern sq_arrayappendType ClientSq_arrayappend;
extern sq_arrayappendType ServerSq_arrayappend;

// sq stack push funcs
typedef void (*sq_pushstringType)(void* sqvm, const SQChar* str, SQInteger stackpos);
extern sq_pushstringType ClientSq_pushstring;
extern sq_pushstringType ServerSq_pushstring;

// weird how these don't take a stackpos arg?
typedef void (*sq_pushintegerType)(void* sqvm, SQInteger i);
extern sq_pushintegerType ClientSq_pushinteger;
extern sq_pushintegerType ServerSq_pushinteger;

typedef void (*sq_pushfloatType)(void* sqvm, SQFloat f);
extern sq_pushfloatType ClientSq_pushfloat;
extern sq_pushfloatType ServerSq_pushfloat;

typedef void (*sq_pushboolType)(void* sqvm, SQBool b);
extern sq_pushboolType ClientSq_pushbool;
extern sq_pushboolType ServerSq_pushbool;

typedef SQInteger(*sq_pusherrorType)(void* sqvm, const SQChar* error);
extern sq_pusherrorType ClientSq_pusherror;
extern sq_pusherrorType ServerSq_pusherror;

// sq stack get funcs
typedef const SQChar* (*sq_getstringType)(void* sqvm, SQInteger stackpos);
extern sq_getstringType ClientSq_getstring;
extern sq_getstringType ServerSq_getstring;

typedef SQInteger(*sq_getintegerType)(void* sqvm, SQInteger stackpos);
extern sq_getintegerType ClientSq_getinteger;
extern sq_getintegerType ServerSq_getinteger;

typedef SQFloat(*sq_getfloatType)(void*, SQInteger stackpos);
extern sq_getfloatType ClientSq_getfloat;
extern sq_getfloatType ServerSq_getfloat;

typedef SQBool(*sq_getboolType)(void*, SQInteger stackpos);
extern sq_getboolType ClientSq_getbool;
extern sq_getboolType ServerSq_getbool;

typedef SQRESULT(*sq_getType)(void* sqvm, SQInteger idx);
extern sq_getType ServerSq_sq_get;
extern sq_getType ClientSq_sq_get;

typedef struct SQVM* HSQUIRRELVM;
typedef void(*SQPRINTFUNCTION)(HSQUIRRELVM, const SQChar*, ...);

#pragma pack(push,1)
struct SQSharedState
{
	unsigned char unknownData[0x4158];
	SQPRINTFUNCTION _printfunc;
};

struct SQVM
{
	unsigned char unknownData[0xE8];
	SQSharedState* _sharedstate;
};
#pragma pack(pop)

#define _ss(_vm_) (_vm_)->_sharedstate

template <ScriptContext context> class SquirrelManager
{
private:
	std::vector<SQFuncRegistration*> m_funcRegistrations;

public:
	SQVM* sqvm;
	SQVM* sqvm2;

public:
	SquirrelManager() : sqvm(nullptr) {}

	void VMCreated(SQVM* newSqvm)
	{
		sqvm = newSqvm;
		sqvm2 = *((SQVM**)((char*)sqvm + 8)); // honestly not 100% sure on what this is, but alot of functions take it

		for (SQFuncRegistration* funcReg : m_funcRegistrations)
		{
			Log::Info("Registering %s function %s", GetContextName(context), funcReg->GetName().c_str());

			if (context == ScriptContext::CLIENT || context == ScriptContext::UI)
				ClientRegisterSquirrelFunc(sqvm, funcReg->GetInternalReg());
			else
				ServerRegisterSquirrelFunc(sqvm, funcReg->GetInternalReg());
		}
	}

	void VMDestroyed()
	{
		sqvm = nullptr;
	}

	void ExecuteCode(const char* code)
	{
		// ttf2sdk checks ThreadIsInMainThread here, might be good to do that? doesn't seem like an issue rn tho

		if (!sqvm)
		{
			Log::Error("Cannot execute code, %s squirrel vm is not initialised", GetContextName(context));
			return;
		}

		Log::Info("Executing %s script code %s ", GetContextName(context), code);

		std::string strCode(code);
		CompileBufferState bufferState = CompileBufferState(strCode);

		SQRESULT compileResult = ClientSq_compilebuffer(sqvm2, strCode.c_str(), static_cast<SQInteger>(strCode.length()), "console", 1);

		Log::Info("sq_compilebuffer returned %s", compileResult);
		if (compileResult >= 0)
		{
			if (context == ScriptContext::CLIENT || context == ScriptContext::UI)
			{
				ClientSq_pushroottable(sqvm2);
				SQRESULT callResult = ClientSq_call(sqvm2, 1, false, false);
				Log::Info("sq_call returned %s", callResult);
			}
			else if (context == ScriptContext::SERVER)
			{
				ServerSq_pushroottable(sqvm2);
				SQRESULT callResult = ServerSq_call(sqvm2, 1, false, false);
				Log::Info("sq_call returned %s", callResult);
			}
		}
	}

	int setupfunc(const char* funcname)
	{
		int result = -2;
		if (context == ScriptContext::CLIENT || context == ScriptContext::UI)
		{
			ClientSq_pushroottable(sqvm2);
			ClientSq_pushstring(sqvm2, funcname, -1);
			result = ClientSq_sq_get(sqvm2, -2);
			if (result != SQRESULT_ERROR)
			{
				ClientSq_pushroottable(sqvm2);
			}
		}
		else if (context == ScriptContext::SERVER)
		{
			ServerSq_pushroottable(sqvm2);
			ServerSq_pushstring(sqvm2, funcname, -1);
			result = ServerSq_sq_get(sqvm2, -2);
			if (result != SQRESULT_ERROR)
			{
				ServerSq_pushroottable(sqvm2);
			}
		}
		return result;
	}

	void pusharg(int arg)
	{
		if (context == ScriptContext::CLIENT || context == ScriptContext::UI)
			ClientSq_pushinteger(sqvm2, arg);
		else if (context == ScriptContext::SERVER)
			ServerSq_pushinteger(sqvm2, arg);
	}
	void pusharg(const char* arg)
	{
		if (context == ScriptContext::CLIENT || context == ScriptContext::UI)
			ClientSq_pushstring(sqvm2, arg, -1);
		else if (context == ScriptContext::SERVER)
			ServerSq_pushstring(sqvm2, arg, -1);
	}
	void pusharg(float arg)
	{
		if (context == ScriptContext::CLIENT || context == ScriptContext::UI)
			ClientSq_pushfloat(sqvm2, arg);
		else if (context == ScriptContext::SERVER)
			ServerSq_pushfloat(sqvm2, arg);
	}
	void pusharg(bool arg)
	{
		if (context == ScriptContext::CLIENT || context == ScriptContext::UI)
			ClientSq_pushbool(sqvm2, arg);
		else if (context == ScriptContext::SERVER)
			ServerSq_pushbool(sqvm2, arg);
	}

	int call(int args)
	{
		int result = -2;
		if (context == ScriptContext::CLIENT || context == ScriptContext::UI)
			result = ClientSq_call(sqvm2, args + 1, false, false);
		else if (context == ScriptContext::SERVER)
			result = ServerSq_call(sqvm2, args + 1, false, false);

		return result;
	}

	void AddFuncRegistration(std::string returnType, std::string name, std::string argTypes, std::string helpText, SQFunction func)
	{
		std::string szTypeMask;
		int nParamsCheck = 0;

		SQFuncRegistration* reg = new SQFuncRegistration(
			context,
			name,
			func,
			szTypeMask,
			nParamsCheck,
			returnType,
			argTypes,
			helpText
		);

		m_funcRegistrations.push_back(reg);
	}
};

extern SquirrelManager<ScriptContext::CLIENT>* g_ClientSquirrelManager;
extern SquirrelManager<ScriptContext::SERVER>* g_ServerSquirrelManager;
extern SquirrelManager<ScriptContext::UI>* g_UISquirrelManager;
