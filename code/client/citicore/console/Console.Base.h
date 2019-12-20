#pragma once

using ConsoleChannel = std::string;

namespace console
{
#ifdef COMPILING_CORE
extern "C" DLL_EXPORT void Printfv(ConsoleChannel channel, std::string_view format, fmt::printf_args argumentList);
extern "C" DLL_EXPORT void DPrintfv(ConsoleChannel, std::string_view format, fmt::printf_args argumentList);
extern "C" DLL_EXPORT void PrintWarningv(ConsoleChannel, std::string_view format, fmt::printf_args argumentList);
extern "C" DLL_EXPORT void PrintErrorv(ConsoleChannel, std::string_view format, fmt::printf_args argumentList);
extern "C" DLL_EXPORT void CoreSetPrintFunction(void(*function)(const char*));
#elif defined(_WIN32)
inline void Printfv(ConsoleChannel channel, std::string_view format, fmt::printf_args argumentList)
{
	using TCoreFunc = decltype(&Printfv);

	static TCoreFunc func;

	if (!func)
	{
		func = (TCoreFunc)GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "Printfv");
	}

	(func) ? func(channel, format, argumentList) : (void)0;
}

inline void DPrintfv(ConsoleChannel channel, std::string_view format, fmt::printf_args argumentList)
{
	using TCoreFunc = decltype(&DPrintfv);

	static TCoreFunc func;

	if (!func)
	{
		func = (TCoreFunc)GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "DPrintfv");
	}

	(func) ? func(channel, format, argumentList) : (void)0;
}

inline void PrintWarningv(ConsoleChannel channel, std::string_view format, fmt::printf_args argumentList)
{
	using TCoreFunc = decltype(&PrintWarningv);

	static TCoreFunc func;

	if (!func)
	{
		func = (TCoreFunc)GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "PrintWarningv");
	}

	(func) ? func(channel, format, argumentList) : (void)0;
}

inline void PrintErrorv(ConsoleChannel channel, std::string_view format, fmt::printf_args argumentList)
{
	using TCoreFunc = decltype(&PrintErrorv);

	static TCoreFunc func;

	if (!func)
	{
		func = (TCoreFunc)GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "PrintErrorv");
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
extern "C" void Printfv(ConsoleChannel channel, std::string_view format, fmt::printf_args argumentList);
extern "C" void DPrintfv(ConsoleChannel, std::string_view format, fmt::printf_args argumentList);
extern "C" void PrintWarningv(ConsoleChannel, std::string_view format, fmt::printf_args argumentList);
extern "C" void PrintErrorv(ConsoleChannel, std::string_view format, fmt::printf_args argumentList);
extern "C" void CoreAddPrintListener(void(*function)(ConsoleChannel, const char*));
extern "C" void CoreSetPrintFunction(void(*function)(const char*));
#endif

template<typename... TArgs>
inline void Printf(ConsoleChannel channel, std::string_view format, const TArgs&... args)
{
	return Printfv(channel, format, fmt::make_printf_args(args...));
}

template<typename... TArgs>
inline void DPrintf(ConsoleChannel channel, std::string_view format, const TArgs&... args)
{
	return DPrintfv(channel, format, fmt::make_printf_args(args...));
}

template<typename... TArgs>
inline void PrintWarning(ConsoleChannel channel, std::string_view format, const TArgs&... args)
{
	return PrintWarningv(channel, format, fmt::make_printf_args(args...));
}

template<typename... TArgs>
inline void PrintError(ConsoleChannel channel, std::string_view format, const TArgs&... args)
{
	return PrintErrorv(channel, format, fmt::make_printf_args(args...));
}
}

namespace sys
{
void InitializeConsole();

const char* GetConsoleInput();
}
