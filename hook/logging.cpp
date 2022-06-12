#include "pch.h"
#include "logging.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <minidumpapiset.h>
#include "configurables.h"
#include "sourceconsole.h"

#include <iostream>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

std::string TimeFormat()
{
    using namespace std::chrono;

    // get current time
    auto now = system_clock::now();

    // get number of milliseconds for the current second
    // (remainder after division into seconds)
    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    // convert to std::time_t in order to convert to std::tm (broken time)
    auto timer = system_clock::to_time_t(now);

    // convert to broken time
    std::tm bt = *std::localtime(&timer);

    std::ostringstream oss;

    oss << std::put_time(&bt, "%Y-%m-%d %H:%M:%S");
    oss << '.' << std::setfill('0') << std::setw(3) << ms.count();

    return oss.str();
}

void PrintFormattedLog(const char* level, const char* fmt, va_list Args) {
    char buf[1024];
    sprintf(buf, "[%s] [%s] %s\n", TimeFormat().c_str(), level, fmt);

    auto endpos = strlen(buf);
    if (buf[endpos - 2] == '\n')
        buf[endpos - 1] = '\0'; // cut off repeated newline

    char buf2[1024];
    vsprintf(buf2, buf, Args);
    printf(buf2);

    std::ofstream logFile;
    logFile.open("R1Spectre/logs/latest.log", std::ios_base::app);
    logFile << buf2;
    logFile.close();
}


typedef int(*ConMsgType)(const char*, ...);
ConMsgType ConMsgOriginal;

void Log::Info(const char* fmt, ...) {
    va_list Args;
    va_start(Args, fmt);
    PrintFormattedLog("Info", fmt, Args);

    char buf[1024];
    vsprintf(buf, fmt, Args);
	char buf2[1024];
	sprintf(buf2, "[Info] %s\n", buf);
	SC_ColorPrint(SourceColor(255, 255, 255, 255), buf2);
    va_end(Args);
}
void Log::Warn(const char* fmt, ...) {
    va_list Args;
    va_start(Args, fmt);
    PrintFormattedLog("Warn", fmt, Args);

	char buf[1024];
	vsprintf(buf, fmt, Args);
	char buf2[1024];
	sprintf(buf2, "[Warn] %s\n", buf);
	SC_ColorPrint(SourceColor(255, 255, 0, 255), buf2);
    va_end(Args);
}
void Log::Critical(const char* fmt, ...) {
    va_list Args;
    va_start(Args, fmt);
    PrintFormattedLog("Critical", fmt, Args);

	char buf[1024];
	vsprintf(buf, fmt, Args);
	char buf2[1024];
	sprintf(buf2, "[Critical] %s\n", buf);
	SC_ColorPrint(SourceColor(255, 0, 0, 255), buf2);
    va_end(Args);
}
void Log::Error(const char* fmt, ...) {
    va_list Args;
    va_start(Args, fmt);
    PrintFormattedLog("Error", fmt, Args);

	char buf[1024];
	vsprintf(buf, fmt, Args);
	char buf2[1024];
	sprintf(buf2, "[Error] %s\n", buf);
	SC_ColorPrint(SourceColor(255, 0, 0, 255), buf2);
    va_end(Args);
}

void ConMsgHook(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    PrintFormattedLog("Game] [ConMsg", fmt, args);

    // Call the real ConMsg function
    // Yes. It is hacky to format it here but it works
    char buf[1024];
    vsprintf(buf, fmt, args);
    ConMsgOriginal(buf);

    va_end(args);
}

typedef int(*ConDMsgType)(const char*, ...);
ConDMsgType ConDMsgOriginal;

void ConDMsgHook(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    PrintFormattedLog("Game] [ConDMsg", fmt, args);

    // Call the real ConDMsg function
    // same as conmsg really
    char buf[1024];
    vsprintf(buf, fmt, args);
    ConDMsgOriginal(buf);

    va_end(args);
}

typedef int(*ConLogType)(const char*, ...);
ConLogType ConLogOriginal;

void ConLogHook(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    PrintFormattedLog("Game] [ConLog", fmt, args);

    // Call the real ConLog function
    char buf[1024];
    vsprintf(buf, fmt, args);
    ConLogOriginal(buf);

    va_end(args);
}

typedef int(*LogType)(const wchar_t*, ...);
LogType LogOriginal;

void LogHook(const wchar_t* fmt_w, ...) {
    char* fmt = (char*)fmt_w;
    va_list args;
    va_start(args, fmt_w);
    PrintFormattedLog("Game] [Log", fmt, args);

    // Call the real ConMsg function
    // Yes. It is hacky to format it here but it works
    char buf[1024];
    vsprintf(buf, fmt, args);
    LogOriginal((wchar_t*)buf);

    va_end(args);
}

typedef int(*ConWarningType)(const char*, ...);
ConWarningType ConWarningOriginal;

void ConWarningHook(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    PrintFormattedLog("Game] [ConWarning", fmt, args);

    // Call the real ConMsg function
    // Yes. It is hacky to format it here but it works
    char buf[1024];
    vsprintf(buf, fmt, args);
    ConWarningOriginal(buf);

    va_end(args);
}

typedef int(*MsgType)(const wchar_t*, ...);
MsgType MsgOriginal;
MsgType WarningOriginal;
MsgType Warning_SpewCallStackOriginal;
MsgType ErrorOriginal;
MsgType Error_SpewCallStackOriginal;

void MsgHook(const wchar_t* fmt_w, ...) {
    char* fmt = (char*)fmt_w;
    va_list args;
    va_start(args, fmt_w);
    PrintFormattedLog("Game] [Msg", fmt, args);

    // Call the real ConMsg function
    // Yes. It is hacky to format it here but it works
    char buf[1024];
    vsprintf(buf, fmt, args);
    MsgOriginal((wchar_t*)buf);

    va_end(args);
}
void WarningHook(const wchar_t* fmt_w, ...) {
    char* fmt = (char*)fmt_w;
    va_list args;
    va_start(args, fmt_w);
    PrintFormattedLog("Game] [Warning", fmt, args);

    // Call the real ConMsg function
    // Yes. It is hacky to format it here but it works
    char buf[1024];
    vsprintf(buf, fmt, args);
    WarningOriginal((wchar_t*)buf);

    va_end(args);
}
void Warning_SpewCallStackHook(const wchar_t* fmt_w, ...) {
    char* fmt = (char*)fmt_w;
    va_list args;
    va_start(args, fmt_w);
    PrintFormattedLog("Game] [Warning] [SPEW", fmt, args);

    // Call the real ConMsg function
    // Yes. It is hacky to format it here but it works
    char buf[1024];
    vsprintf(buf, fmt, args);
    Warning_SpewCallStackOriginal((wchar_t*)buf);

    va_end(args);
}
void ErrorHook(const wchar_t* fmt_w, ...) {
    char* fmt = (char*)fmt_w;
    va_list args;
    va_start(args, fmt_w);
    PrintFormattedLog("Game] [Error", fmt, args);

    // Call the real ConMsg function
    // Yes. It is hacky to format it here but it works
    char buf[1024];
    vsprintf(buf, fmt, args);
    ErrorOriginal((wchar_t*)buf);

    va_end(args);
}
void Error_SpewCallStackHook(const wchar_t* fmt_w, ...) {
    char* fmt = (char*)fmt_w;
    va_list args;
    va_start(args, fmt_w);
    PrintFormattedLog("Game] [Error] [SPEW", fmt, args);

    // Call the real ConMsg function
    // Yes. It is hacky to format it here but it works
    char buf[1024];
    vsprintf(buf, fmt, args);
    Error_SpewCallStackOriginal((wchar_t*)buf);

    va_end(args);
}

long __stdcall ExceptionFilter(EXCEPTION_POINTERS* exceptionInfo)
{
	static bool logged = false;
	if (logged)
		return EXCEPTION_CONTINUE_SEARCH;

	if (!IsDebuggerPresent())
	{
		const DWORD exceptionCode = exceptionInfo->ExceptionRecord->ExceptionCode;
		if (exceptionCode != EXCEPTION_ACCESS_VIOLATION && exceptionCode != EXCEPTION_ARRAY_BOUNDS_EXCEEDED &&
			exceptionCode != EXCEPTION_DATATYPE_MISALIGNMENT && exceptionCode != EXCEPTION_FLT_DENORMAL_OPERAND &&
			exceptionCode != EXCEPTION_FLT_DIVIDE_BY_ZERO && exceptionCode != EXCEPTION_FLT_INEXACT_RESULT &&
			exceptionCode != EXCEPTION_FLT_INVALID_OPERATION && exceptionCode != EXCEPTION_FLT_OVERFLOW &&
			exceptionCode != EXCEPTION_FLT_STACK_CHECK && exceptionCode != EXCEPTION_FLT_UNDERFLOW &&
			exceptionCode != EXCEPTION_ILLEGAL_INSTRUCTION && exceptionCode != EXCEPTION_IN_PAGE_ERROR &&
			exceptionCode != EXCEPTION_INT_DIVIDE_BY_ZERO && exceptionCode != EXCEPTION_INT_OVERFLOW &&
			exceptionCode != EXCEPTION_INVALID_DISPOSITION && exceptionCode != EXCEPTION_NONCONTINUABLE_EXCEPTION &&
			exceptionCode != EXCEPTION_PRIV_INSTRUCTION && exceptionCode != EXCEPTION_STACK_OVERFLOW)
			return EXCEPTION_CONTINUE_SEARCH;

		std::stringstream exceptionCause;
		exceptionCause << "Cause: ";
		switch (exceptionCode)
		{
		case EXCEPTION_ACCESS_VIOLATION:
		case EXCEPTION_IN_PAGE_ERROR:
		{
			exceptionCause << "Access Violation" << std::endl;

			auto exceptionInfo0 = exceptionInfo->ExceptionRecord->ExceptionInformation[0];
			auto exceptionInfo1 = exceptionInfo->ExceptionRecord->ExceptionInformation[1];

			if (!exceptionInfo0)
				exceptionCause << "Attempted to read from: 0x" << (void*)exceptionInfo1;
			else if (exceptionInfo0 == 1)
				exceptionCause << "Attempted to write to: 0x" << (void*)exceptionInfo1;
			else if (exceptionInfo0 == 8)
				exceptionCause << "Data Execution Prevention (DEP) at: 0x" << (void*)std::hex << exceptionInfo1;
			else
				exceptionCause << "Unknown access violation at: 0x" << (void*)exceptionInfo1;

			break;
		}
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			exceptionCause << "Array bounds exceeded";
			break;
		case EXCEPTION_DATATYPE_MISALIGNMENT:
			exceptionCause << "Datatype misalignment";
			break;
		case EXCEPTION_FLT_DENORMAL_OPERAND:
			exceptionCause << "Denormal operand";
			break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			exceptionCause << "Divide by zero (float)";
			break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			exceptionCause << "Divide by zero (int)";
			break;
		case EXCEPTION_FLT_INEXACT_RESULT:
			exceptionCause << "Inexact result";
			break;
		case EXCEPTION_FLT_INVALID_OPERATION:
			exceptionCause << "Invalid operation";
			break;
		case EXCEPTION_FLT_OVERFLOW:
		case EXCEPTION_INT_OVERFLOW:
			exceptionCause << "Numeric overflow";
			break;
		case EXCEPTION_FLT_UNDERFLOW:
			exceptionCause << "Numeric underflow";
			break;
		case EXCEPTION_FLT_STACK_CHECK:
			exceptionCause << "Stack check";
			break;
		case EXCEPTION_ILLEGAL_INSTRUCTION:
			exceptionCause << "Illegal instruction";
			break;
		case EXCEPTION_INVALID_DISPOSITION:
			exceptionCause << "Invalid disposition";
			break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			exceptionCause << "Noncontinuable exception";
			break;
		case EXCEPTION_PRIV_INSTRUCTION:
			exceptionCause << "Priviledged instruction";
			break;
		case EXCEPTION_STACK_OVERFLOW:
			exceptionCause << "Stack overflow";
			break;
		default:
			exceptionCause << "Unknown";
			break;
		}

		void* exceptionAddress = exceptionInfo->ExceptionRecord->ExceptionAddress;

		HMODULE crashedModuleHandle;
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, static_cast<LPCSTR>(exceptionAddress), &crashedModuleHandle);

		MODULEINFO crashedModuleInfo;
		GetModuleInformation(GetCurrentProcess(), crashedModuleHandle, &crashedModuleInfo, sizeof(crashedModuleInfo));

		char crashedModuleFullName[MAX_PATH];
		GetModuleFileNameExA(GetCurrentProcess(), crashedModuleHandle, crashedModuleFullName, MAX_PATH);
		char* crashedModuleName = strrchr(crashedModuleFullName, '\\') + 1;

		DWORD64 crashedModuleOffset = ((DWORD64)exceptionAddress) - ((DWORD64)crashedModuleInfo.lpBaseOfDll);
		CONTEXT* exceptionContext = exceptionInfo->ContextRecord;

		Log::Error("Spectre has crashed! a minidump has been written and exception info is available below:");
		Log::Error(exceptionCause.str().c_str());
		Log::Error("At: %s + 0x%08x", crashedModuleName, (void*)crashedModuleOffset);

		PVOID framesToCapture[62];
		int frames = RtlCaptureStackBackTrace(0, 62, framesToCapture, NULL);
		for (int i = 0; i < frames; i++)
		{
			HMODULE backtraceModuleHandle;
			GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, static_cast<LPCSTR>(framesToCapture[i]), &backtraceModuleHandle);

			char backtraceModuleFullName[MAX_PATH];
			GetModuleFileNameExA(GetCurrentProcess(), backtraceModuleHandle, backtraceModuleFullName, MAX_PATH);
			char* backtraceModuleName = strrchr(backtraceModuleFullName, '\\') + 1;

			void* actualAddress = (void*)framesToCapture[i];
			void* relativeAddress = (void*)(uintptr_t(actualAddress) - uintptr_t(backtraceModuleHandle));

			Log::Error("    %s + 0x%08x (0x%08x)", backtraceModuleName, relativeAddress, actualAddress);
		}

		Log::Error("RAX: 0x%08x", exceptionContext->Rax);
		Log::Error("RBX: 0x%08x", exceptionContext->Rbx);
		Log::Error("RCX: 0x%08x", exceptionContext->Rcx);
		Log::Error("RDX: 0x%08x", exceptionContext->Rdx);
		Log::Error("RSI: 0x%08x", exceptionContext->Rsi);
		Log::Error("RDI: 0x%08x", exceptionContext->Rdi);
		Log::Error("RBP: 0x%08x", exceptionContext->Rbp);
		Log::Error("RSP: 0x%08x", exceptionContext->Rsp);
		Log::Error("R8: 0x%08x", exceptionContext->R8);
		Log::Error("R9: 0x%08x", exceptionContext->R9);
		Log::Error("R10: 0x%08x", exceptionContext->R10);
		Log::Error("R11: 0x%08x", exceptionContext->R11);
		Log::Error("R12: 0x%08x", exceptionContext->R12);
		Log::Error("R13: 0x%08x", exceptionContext->R13);
		Log::Error("R14: 0x%08x", exceptionContext->R14);
		Log::Error("R15: 0x%08x", exceptionContext->R15);

		time_t time = std::time(nullptr);
		tm currentTime = *std::localtime(&time);
		std::stringstream stream;
		stream << std::put_time(&currentTime, (GetSpectrePrefix() + "/logs/r1sdump%Y-%m-%d %H-%M-%S.dmp").c_str());

		auto hMinidumpFile = CreateFileA(stream.str().c_str(), GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (hMinidumpFile)
		{
			MINIDUMP_EXCEPTION_INFORMATION dumpExceptionInfo;
			dumpExceptionInfo.ThreadId = GetCurrentThreadId();
			dumpExceptionInfo.ExceptionPointers = exceptionInfo;
			dumpExceptionInfo.ClientPointers = false;

			MiniDumpWriteDump(
				GetCurrentProcess(),
				GetCurrentProcessId(),
				hMinidumpFile,
				MINIDUMP_TYPE(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory),
				&dumpExceptionInfo,
				nullptr,
				nullptr);
			CloseHandle(hMinidumpFile);
		}
		else
			Log::Error("Failed to write minidump file %s!", stream.str().c_str());

		//if (!IsDedicated())
			MessageBoxA(
				0, "Spectre has crashed! Crash info can be found in R1Spectre/logs", "Spectre has crashed!", MB_ICONERROR | MB_OK);
	}

	logged = true;
	return EXCEPTION_EXECUTE_HANDLER;
}

HANDLE hExceptionFilter;

BOOL WINAPI ConsoleHandlerRoutine(DWORD eventCode)
{
	switch (eventCode)
	{
	case CTRL_CLOSE_EVENT:
		// User closed console, shut everything down
		Log::Info("Exiting due to console close...");
		RemoveVectoredExceptionHandler(hExceptionFilter);
		exit(EXIT_SUCCESS);
		return FALSE;
	}

	return TRUE;
}

void AllowExitToCrash()
{
	Log::Info("Exiting...");
	RemoveVectoredExceptionHandler(hExceptionFilter);
}


void InitialiseLogging() {
	hExceptionFilter = AddVectoredExceptionHandler(TRUE, ExceptionFilter);
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	SetConsoleCtrlHandler(ConsoleHandlerRoutine, true);

    fs::create_directory("R1Spectre");
    fs::create_directory("R1Spectre/logs");

    std::ofstream logStream;
    logStream.open("R1Spectre/logs/latest.log", std::ofstream::out | std::ofstream::trunc);
    logStream.close();

	SetConsoleTitleA("R1Spectre");
}

void InitialiseLogHooks(HMODULE baseAddress) {
    //HookEnabler hook;

    Log::Info("Logging Initialised");
}