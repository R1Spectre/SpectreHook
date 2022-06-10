#pragma once

void InitialiseLogging();
void InitialiseLogHooks(HMODULE baseAdrress);

class Log {
public:
	static void Info(const char* fmt, ...);
	static void Warn(const char* fmt, ...);
	static void Critical(const char* fmt, ...);
	static void Error(const char* fmt, ...);
};