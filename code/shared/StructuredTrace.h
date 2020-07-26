#pragma once

#ifdef IS_FXSERVER
#include <json.hpp>

#ifdef COMPILING_CORE
extern "C" bool DLL_EXPORT StructuredTraceEnabled();
extern "C" void DLL_EXPORT StructuredTraceReal(const char* channel, const char* func, const char* file, int line, const nlohmann::json & j);
#elif defined(_WIN32)
inline bool StructuredTraceEnabled()
{
	using TCoreFunc = decltype(&StructuredTraceEnabled);

	static TCoreFunc func;

	if (!func)
	{
		func = (TCoreFunc)GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "StructuredTraceEnabled");
	}

	return (func) ? func() : false;
}

inline void StructuredTraceReal(const char* channel, const char* fn, const char* file, int line, const nlohmann::json& j)
{
	using TCoreFunc = decltype(&StructuredTraceReal);

	static TCoreFunc func;

	if (!func)
	{
		func = (TCoreFunc)GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "StructuredTraceReal");
	}

	(func) ? func(channel, fn, file, line, j) : (void)0;
}
#else
extern "C" bool StructuredTraceEnabled();
extern "C" void StructuredTraceReal(const char* channel, const char* func, const char* file, int line, const nlohmann::json& j);
#endif

#define StructuredTrace(...) do { \
	if (StructuredTraceEnabled()) { \
		try { \
		StructuredTraceReal(_CFX_NAME_STRING(_CFX_COMPONENT_NAME), _CFX_TRACE_FUNC, _CFX_TRACE_FILE, __LINE__, nlohmann::json::object({ __VA_ARGS__ })); \
		} catch (const std::exception& e) { \
		} \
	} \
} while (false);

#else
#define StructuredTrace(...)
#endif
