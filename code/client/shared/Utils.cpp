/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Utils.h"
#include <sstream>
#include <iomanip>
#include <mutex>
#include <utf8.h>
#include "Error.h"

static STATIC InitFunctionBase* g_initFunctions;

InitFunctionBase::InitFunctionBase(int order /* = 0 */)
	: m_order(order)
{

}

void InitFunctionBase::Register()
{
	if (!g_initFunctions)
	{
		m_next = nullptr;
		g_initFunctions = this;
	}
	else
	{
		InitFunctionBase* cur = g_initFunctions;
		InitFunctionBase* last = nullptr;

		while (cur && m_order >= cur->m_order)
		{
			last = cur;
			cur = cur->m_next;
		}

		m_next = cur;

		(!last ? g_initFunctions : last->m_next) = this;
	}
}

void InitFunctionBase::RunAll()
{
	for (InitFunctionBase* func = g_initFunctions; func; func = func->m_next)
	{
		func->Run();
	}
}

#define BUFFER_COUNT 8
#define BUFFER_LENGTH 32768

template<typename CharType, typename TArgs>
inline const CharType* va_impl(std::basic_string_view<CharType> string, const TArgs& formatList)
{
	static thread_local int currentBuffer;
	static thread_local std::vector<CharType> buffer;

	if (!buffer.size())
	{
		buffer.resize(BUFFER_COUNT * BUFFER_LENGTH);
	}

	int thisBuffer = currentBuffer;

	auto formatted = fmt::vsprintf(string, formatList);

	if (formatted.length() >= BUFFER_LENGTH)
	{
		FatalError("Exceeded buffer length in va()!");
	}

	memcpy(&buffer[thisBuffer * BUFFER_LENGTH], formatted.c_str(), (formatted.length() + 1) * sizeof(CharType));

	currentBuffer = (currentBuffer + 1) % BUFFER_COUNT;

	return &buffer[thisBuffer * BUFFER_LENGTH];
}

const char* vva(std::string_view string, fmt::printf_args formatList)
{
	return va_impl(string, formatList);
}

const wchar_t* vva(std::wstring_view string, fmt::wprintf_args formatList)
{
	return va_impl(string, formatList);
}

#ifdef _WIN32
#include <winternl.h>

void DoNtRaiseException(EXCEPTION_RECORD* record)
{
	typedef NTSTATUS(*NtRaiseExceptionType)(PEXCEPTION_RECORD record, PCONTEXT context, BOOL firstChance);

	bool threw = false;

	CONTEXT context;
	RtlCaptureContext(&context);

	static NtRaiseExceptionType NtRaiseException = (NtRaiseExceptionType)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtRaiseException");

	// where will this function return to? hint: it'll return RIGHT AFTER RTLCAPTURECONTEXT, as that's what's in context->Rip!
	// therefore, check if 'threw' is false first...
	if (!threw)
	{
		threw = true;
		NtRaiseException(record, &context, TRUE);

		// force 'threw' to be stack-allocated by messing with it from here (where it won't execute)
		OutputDebugStringA((char*)&threw);
	}
}

static void RaiseDebugException(const char* buffer, size_t length)
{
	__try
	{
		EXCEPTION_RECORD record;
		record.ExceptionAddress = reinterpret_cast<PVOID>(_ReturnAddress());
		record.ExceptionCode = DBG_PRINTEXCEPTION_C;
		record.ExceptionFlags = 0;
		record.NumberParameters = 2;
		record.ExceptionInformation[0] = length + 1;
		record.ExceptionInformation[1] = reinterpret_cast<ULONG_PTR>(buffer);
		record.ExceptionRecord = &record;

		DoNtRaiseException(&record);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		OutputDebugStringA(buffer);
	}
}
#endif

#if defined(_WIN32) && !defined(IS_FXSERVER)
inline void AsyncTrace(const char* string)
{
	using TCoreTraceFunc = decltype(&AsyncTrace);

	static TCoreTraceFunc func;

	if (!func)
	{
		func = (TCoreTraceFunc)GetProcAddress(GetModuleHandle(NULL), "AsyncTrace");

		if (!func)
		{
			// try getting function proxy from CoreRT, could be we've already loaded a game
			func = (TCoreTraceFunc)GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "AsyncTrace");
		}
	}

	(func) ? func(string) : (void)0;
}
#endif

void TraceRealV(const char* channel, const char* func, const char* file, int line, std::string_view string, fmt::printf_args formatList)
{
	std::string buffer;

	try
	{
		buffer = fmt::vsprintf(string, formatList);
	}
	catch (fmt::format_error& e)
	{
		buffer = fmt::sprintf("fmt::format_error while formatting %s: %s\n", string, e.what());
	}

	CoreTrace(channel, func, file, line, buffer.data());

#ifdef _WIN32
	static CRITICAL_SECTION dbgCritSec;

	if (!dbgCritSec.DebugInfo)
	{
		InitializeCriticalSectionAndSpinCount(&dbgCritSec, 100);
	}

	if (CoreIsDebuggerPresent())
	{
		// thanks to anti-debug workarounds (IsBeingDebugged == FALSE), we'll have to raise the exception to the debugger ourselves.
		// sadly, RaiseException (and, by extension, RtlRaiseException) won't raise a first-chance exception, so we'll have to do such by hand...
		// again, it may appear things 'work' if using a non-first-chance exception (i.e. the debugger will catch it), but VS doesn't like that and lets some
		// cases of the exception fall through.

		RaiseDebugException(buffer.c_str(), buffer.length());
	}
	else
	{
		OutputDebugStringA(buffer.c_str());
	}

#ifndef IS_FXSERVER
	AsyncTrace(buffer.data());
#endif
#endif
}

fwString url_encode(const fwString &value)
{
	std::ostringstream escaped;
	escaped.fill('0');
	escaped << std::hex;

	for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i)
	{
		std::string::value_type c = (*i);
		if (((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '-' || c == '_' || c == '.' || c == '~') && c != '+')
		{
			escaped << c;
		}
		else if (c == ' ')
		{
			escaped << '+';
		}
		else
		{
			escaped << '%' << std::setw(2) << ((int)(uint8_t)c) << std::setw(0);
		}
	}

	return fwString(escaped.str().c_str());
}

bool UrlDecode(const std::string& in, std::string& out)
{
	out.clear();
	out.reserve(in.size());
	for (std::size_t i = 0; i < in.size(); ++i)
	{
		if (in[i] == '%')
		{
			if (i + 3 <= in.size())
			{
				int value = 0;
				std::istringstream is(in.substr(i + 1, 2));
				if (is >> std::hex >> value)
				{
					out += static_cast<char>(value);
					i += 2;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		else if (in[i] == '+')
		{
			out += ' ';
		}
		else
		{
			out += in[i];
		}
	}
	return true;
}

std::string ToNarrow(const std::wstring& wide)
{
	// TODO: replace with something faster if needed
	std::vector<char> outVec;
	outVec.reserve(wide.size());

#ifdef _WIN32
	utf8::utf16to8(wide.begin(), wide.end(), std::back_inserter(outVec));
#else
	utf8::utf32to8(wide.begin(), wide.end(), std::back_inserter(outVec));
#endif
	
	return std::string(outVec.begin(), outVec.end());
}

std::wstring ToWide(const std::string& narrow)
{
	std::vector<uint8_t> cleanVec;
	cleanVec.reserve(narrow.size());

	std::vector<wchar_t> outVec;
	outVec.reserve(cleanVec.size());

	try
	{
		utf8::replace_invalid(narrow.begin(), narrow.end(), std::back_inserter(cleanVec));

#ifdef _WIN32
		utf8::utf8to16(cleanVec.begin(), cleanVec.end(), std::back_inserter(outVec));
#else
		utf8::utf8to32(cleanVec.begin(), cleanVec.end(), std::back_inserter(outVec));
#endif
	}
	catch (utf8::exception& e)
	{

	}

	return std::wstring(outVec.begin(), outVec.end());
}
