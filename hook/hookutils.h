#pragma once
#include <vector>
#include <codecvt>

// Enables all hooks created with the HookEnabler object when it goes out of scope and handles hook errors
class HookEnabler
{
private:
	struct HookTarget
	{
		char* targetName;
		LPVOID targetAddress;
	};

	std::vector<HookTarget*> m_hookTargets;

public:
	void CreateHook(LPVOID ppTarget, LPVOID ppDetour, LPVOID* ppOriginal, const char* targetName = nullptr);
	~HookEnabler();
};

// macro to call HookEnabler::CreateHook with the hook's name
#define ENABLER_CREATEHOOK(enabler, ppTarget, ppDetour, ppOriginal) enabler.CreateHook(ppTarget, ppDetour, ppOriginal, #ppDetour)

template <typename T>
inline MH_STATUS MH_CreateHookEx(LPVOID pTarget, LPVOID pDetour, T** ppOriginal)
{
    return MH_CreateHook(pTarget, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
}

enum ScriptContext
{
    SCRIPT_CONTEXT_SERVER, // vm = server_local+0x108FFF0 // TODO_UPDATE
    SCRIPT_CONTEXT_CLIENT, // vm = client+0x16BBE78
    SCRIPT_CONTEXT_UI // vm = client+0x16C1FA8
};

inline std::wstring Widen(const std::string& input)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.from_bytes(input);
}

inline HMODULE GetModuleHandleOrThrow(const std::string& moduleName) {
    HMODULE hModule = GetModuleHandle(Widen(moduleName).c_str());
    if (!hModule)
    {
        Log::Error("GetModuleHandle failed for %s (Error = 0x%X)", moduleName.c_str(), GetLastError());
    }
    return hModule;
}

inline MODULEINFO GetModuleInfo(const std::string& moduleName) {
    MODULEINFO modinfo = { 0 };
    //HMODULE hModule = SafeGetModuleHandle(moduleName);
    HMODULE hModule = GetModuleHandleOrThrow(moduleName);
    GetModuleInformation(GetCurrentProcess(), hModule, &modinfo, sizeof(MODULEINFO));
    return modinfo;
}

inline DWORD64 GetModuleBaseAddress(const std::string& moduleName) {
    return (DWORD64)GetModuleInfo(moduleName).lpBaseOfDll;
}

constexpr const char* GetContextName(ScriptContext context)
{
    if (context == SCRIPT_CONTEXT_SERVER)
    {
        return "SERVER";
    }
    else if (context == SCRIPT_CONTEXT_CLIENT)
    {
        return "CLIENT";
    }
    else if (context == SCRIPT_CONTEXT_UI)
    {
        return "UI";
    }
    else
    {
        return "UNKNOWN";
    }
}

template <typename T, T, typename U, U> struct MemberWrapper;
template<typename T, T& (*pObjGet)(), typename RT, typename... Args, RT(T::* pF)(Args...)>
struct MemberWrapper<RT(T::*)(Args...), pF, T& (*)(), pObjGet>
{
    /*static RT Call(Args&&... args)
    {
        return ((pObjGet()).*pF)(std::forward<Args>(args)...);
    }*/

    static RT Call(Args... args)
    {
        //if (&pObjGet() == nullptr /*|| ((pObjGet()).*pF) == nullptr*/) return NULL;
        return ((pObjGet()).*pF)(args...);
    }
};

template<typename T, typename... Args>
class FuncStatic
{
protected:
    T(*m_func)(Args...) = nullptr;

public:
    FuncStatic(const char* moduleName, DWORD64 offset)
    {
        m_func = (T(/*__cdecl*/*)(Args...))(GetModuleBaseAddress(moduleName) + offset); // if we inserted this DLL late before modules are loaded, then we'd need to create a static func registry analogous to sigscan one and resolve them when stuff is ready (also when calling check if func is ready and throw if it's not); but thankfully we don't :) just a comment for future reference
    }

    void SetFuncPtr(void* ptr)
    {
        m_func = reinterpret_cast<T(*)(Args...)>(ptr);
    }

    void* GetFuncPtr()
    {
        return reinterpret_cast<void*>(m_func);
    }

    __forceinline T operator()(Args... args)
    {
        return m_func(args...);
    }
};

template<typename T>
class FuncStaticWithType
{
protected:
    T m_func = nullptr;

public:
    FuncStaticWithType(const char* moduleName, DWORD64 offset)
    {
        m_func = (T)(GetModuleBaseAddress(moduleName) + offset); // if we inserted this DLL late before modules are loaded, then we'd need to create a static func registry analogous to sigscan one and resolve them when stuff is ready (also when calling check if func is ready and throw if it's not); but thankfully we don't :) just a comment for future reference
    }

    void SetFuncPtr(void* ptr)
    {
        m_func = reinterpret_cast<T>(ptr);
    }

    void* GetFuncPtr()
    {
        return reinterpret_cast<void*>(m_func);
    }

    /*__forceinline T operator()(Args... args)
    {
        return m_func(args...);
    }*/

    template<typename... Args> __forceinline auto operator()(Args... args)
    {
        return m_func(args...);
    }
};

//template<typename T, typename... Args>
template<typename T>
class HookedFuncStaticWithType
{
public:
    bool m_hooked = false;
    //T(*m_origFunc)(Args...) = nullptr;
    //T(*m_hookedFunc)(Args...) = nullptr;
    T m_origFunc = nullptr;
    T m_hookedFunc = nullptr;

private:
    const char* moduleName;
    DWORD64 offset;

public:
    HookedFuncStaticWithType(const char* moduleName, DWORD64 offset)
    {
        this->moduleName = moduleName;
        this->offset = offset;
    }

    //void Hook(T(*detourFunc)(Args...))
    void Hook(T detourFunc)
    {
        if (m_hooked)
        {
            return;
        }

        //m_hookedFunc = /*(LPVOID)*/(T(__cdecl*)(Args...))(GetModuleBaseAddress(moduleName) + offset);
        m_hookedFunc = /*(LPVOID)*/(T)(GetModuleBaseAddress(moduleName) + offset);

        MH_STATUS status = MH_CreateHookEx(m_hookedFunc, detourFunc, &m_origFunc);
        if (status != MH_OK)
        {
            Log::Critical("MH_CreateHook returned %s while trying to hook function in module %s with offset 0x%08x", status, moduleName, (void*)offset);
            throw std::exception("Failed to hook function");
        }

        status = MH_EnableHook(m_hookedFunc);
        if (status != MH_OK)
        {
            Log::Critical("MH_EnableHook returned %s", status);
            throw std::exception("Failed to enable hook");
        }

        m_hooked = true;
        Log::Info("Hooked function at 0x%08x - trampoline location: 0x%08x", (void*)m_hookedFunc, (void*)m_origFunc);
    }

    void Unhook()
    {
        if (m_hooked)
        {
            MH_RemoveHook(m_hookedFunc);
        }
    }

    ~HookedFuncStaticWithType()
    {
        if (m_hooked)
        {
            MH_RemoveHook(m_hookedFunc);
        }
    }

    /*__forceinline T operator()(Args... args)
    {
        return m_origFunc(args...);
    }*/

    template<typename... Args> __forceinline auto operator()(Args... args)
    {
        return m_origFunc(args...);
    }
};

template<typename ReturnType, typename... Args>
class HookedFuncStatic : public HookedFuncStaticWithType<ReturnType(*)(Args...)>
{
public:
    HookedFuncStatic(const char* moduleName, DWORD64 offset) : HookedFuncStaticWithType<ReturnType(*)(Args...)>(moduleName, offset)
    {}

    __forceinline ReturnType operator()(Args... args)
    {
        return this->m_origFunc(args...);
    }
};