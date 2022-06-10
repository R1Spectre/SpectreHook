#include "pch.h"
#include "logging.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

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

    if (ConMsgOriginal != nullptr) {
        char buf[1024];
        vsprintf(buf, fmt, Args);
        ConMsgOriginal("[Info] %s\n", buf);
    }
    va_end(Args);
}
void Log::Warn(const char* fmt, ...) {
    va_list Args;
    va_start(Args, fmt);
    PrintFormattedLog("Warn", fmt, Args);

    if (ConMsgOriginal != nullptr) {
        char buf[1024];
        vsprintf(buf, fmt, Args);
        ConMsgOriginal("[Warn] %s\n", buf);
    }
    va_end(Args);
}
void Log::Critical(const char* fmt, ...) {
    va_list Args;
    va_start(Args, fmt);
    PrintFormattedLog("Critical", fmt, Args);

    if (ConMsgOriginal != nullptr) {
        char buf[1024];
        vsprintf(buf, fmt, Args);
        ConMsgOriginal("[Critical] %s\n", buf);
    }
    va_end(Args);
}
void Log::Error(const char* fmt, ...) {
    va_list Args;
    va_start(Args, fmt);
    PrintFormattedLog("Error", fmt, Args);

    if (ConMsgOriginal != nullptr) {
        char buf[1024];
        vsprintf(buf, fmt, Args);
        ConMsgOriginal("[Error] %s\n", buf);
    }
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

void InitialiseLogging() {
    fs::create_directory("R1Spectre");
    fs::create_directory("R1Spectre/logs");

    std::ofstream logStream;
    logStream.open("R1Spectre/logs/latest.log", std::ofstream::out | std::ofstream::trunc);
    logStream.close();
}

void InitialiseLogHooks(HMODULE baseAddress) {
    //HookEnabler hook;

    Log::Info("Logging Initialised");
}