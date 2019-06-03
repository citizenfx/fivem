#pragma once

using ConsoleChannel = std::string;

namespace console
{
#ifdef COMPILING_CORE
extern "C" DLL_EXPORT void Printf(ConsoleChannel channel, const char* format, const fmt::ArgList& argumentList);
extern "C" DLL_EXPORT void DPrintf(ConsoleChannel, const char* format, const fmt::ArgList& argumentList);
extern "C" DLL_EXPORT void PrintWarning(ConsoleChannel, const char* format, const fmt::ArgList& argumentList);
extern "C" DLL_EXPORT void PrintError(ConsoleChannel, const char* format, const fmt::ArgList& argumentList);
extern "C" DLL_EXPORT void CoreSetPrintFunction(void(*function)(const char*));
#elif defined(_WIN32)
inline void Printf(ConsoleChannel channel, const char* format, const fmt::ArgList& argumentList)
{
	using TCoreFunc = decltype(&Printf);

	static TCoreFunc func;

	if (!func)
	{
		func = (TCoreFunc)GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "Printf");
	}

	(func) ? func(channel, format, argumentList) : (void)0;
}

inline void DPrintf(ConsoleChannel channel, const char* format, const fmt::ArgList& argumentList)
{
	using TCoreFunc = decltype(&Printf);

	static TCoreFunc func;

	if (!func)
	{
		func = (TCoreFunc)GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "DPrintf");
	}

	(func) ? func(channel, format, argumentList) : (void)0;
}

inline void PrintWarning(ConsoleChannel channel, const char* format, const fmt::ArgList& argumentList)
{
	using TCoreFunc = decltype(&Printf);

	static TCoreFunc func;

	if (!func)
	{
		func = (TCoreFunc)GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "PrintWarning");
	}

	(func) ? func(channel, format, argumentList) : (void)0;
}

inline void PrintError(ConsoleChannel channel, const char* format, const fmt::ArgList& argumentList)
{
	using TCoreFunc = decltype(&Printf);

	static TCoreFunc func;

	if (!func)
	{
		func = (TCoreFunc)GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "PrintError");
	}

	(func) ? func(channel, format, argumentList) : (void)0;
}

inline void CoreAddPrintListener(void(*function)(ConsoleChannel, const char*))
{
	using TCoreFunc = decltype(&CoreAddPrintListener);

	static TCoreFunc func;

	if (!func)
	{
		func = (TCoreFunc)GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "CoreAddPrintListener");
	}

	(func) ? func(function) : (void)0;
}

inline void CoreSetPrintFunction(void(*function)(const char*))
{
	using TCoreFunc = decltype(&CoreSetPrintFunction);

	static TCoreFunc func;

	if (!func)
	{
		func = (TCoreFunc)GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "CoreSetPrintFunction");
	}

	(func) ? func(function) : (void)0;
}
#else
extern "C" void Printf(ConsoleChannel channel, const char* format, const fmt::ArgList& argumentList);
extern "C" void DPrintf(ConsoleChannel, const char* format, const fmt::ArgList& argumentList);
extern "C" void PrintWarning(ConsoleChannel, const char* format, const fmt::ArgList& argumentList);
extern "C" void PrintError(ConsoleChannel, const char* format, const fmt::ArgList& argumentList);
extern "C" void CoreAddPrintListener(void(*function)(ConsoleChannel, const char*));
extern "C" void CoreSetPrintFunction(void(*function)(const char*));
#endif

FMT_VARIADIC(void, Printf, ConsoleChannel, const char*);

FMT_VARIADIC(void, DPrintf, ConsoleChannel, const char*);

FMT_VARIADIC(void, PrintWarning, ConsoleChannel, const char*);

FMT_VARIADIC(void, PrintError, ConsoleChannel, const char*);
}

namespace sys
{
void InitializeConsole();

const char* GetConsoleInput();
}
