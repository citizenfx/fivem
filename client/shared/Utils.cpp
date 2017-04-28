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

template<typename CharType>
inline const CharType* va_impl(const CharType* string, const fmt::ArgList& formatList)
{
	static thread_local int currentBuffer;
	static thread_local std::vector<CharType> buffer;

	if (!buffer.size())
	{
		buffer.resize(BUFFER_COUNT * BUFFER_LENGTH);
	}

	int thisBuffer = currentBuffer;

	auto formatted = fmt::sprintf(string, formatList);

	if (formatted.length() >= BUFFER_LENGTH)
	{
		FatalError("Exceeded buffer length in va()!");
	}

	memcpy(&buffer[thisBuffer * BUFFER_LENGTH], formatted.c_str(), (formatted.length() + 1) * sizeof(CharType));

	currentBuffer = (currentBuffer + 1) % BUFFER_COUNT;

	return &buffer[thisBuffer * BUFFER_LENGTH];
}

const char* va(const char* string, const fmt::ArgList& formatList)
{
	return va_impl(string, formatList);
}

const wchar_t* va(const wchar_t* string, const fmt::ArgList& formatList)
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
#endif

struct SharedTickCount
{
	struct Data
	{
		uint64_t tickCount;

		Data()
		{
			tickCount = GetTickCount64();
		}
	};

	SharedTickCount()
	{
		m_data = &m_fakeData;

		bool initTime = true;
		m_fileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(Data), L"CFX_SharedTickCount");

		if (m_fileMapping != nullptr)
		{
			if (GetLastError() == ERROR_ALREADY_EXISTS)
			{
				initTime = false;
			}

			m_data = (Data*)MapViewOfFile(m_fileMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Data));

			if (initTime)
			{
				m_data = new(m_data) Data();
			}
		}
	}

	inline Data& operator*()
	{
		return *m_data;
	}

	inline Data* operator->()
	{
		return m_data;
	}

private:
	HANDLE m_fileMapping;
	Data* m_data;

	Data m_fakeData;
};

static void PerformFileLog(const char* string)
{
	static std::vector<char> lineBuffer(8192);
	static size_t lineIndex;
	static std::mutex logMutex;

	static SharedTickCount initTickCount;

	{
		std::unique_lock<std::mutex> lock(logMutex);

		for (const char* p = string; *p; ++p)
		{
			if (*p == '\n')
			{
				// flush the line
				FILE* logFile = _wfopen(MakeRelativeCitPath(L"CitizenFX.log").c_str(), L"ab");

				if (logFile)
				{
					// null-terminate the string
					lineBuffer[lineIndex] = '\0';

					fprintf(logFile, "[%10lld] %s\r\n", GetTickCount64() - initTickCount->tickCount, lineBuffer.data());
					fclose(logFile);
				}

				// clear the line
				lineIndex = 0;

				// skip this char
				continue;
			}

			// append the character
			lineBuffer[lineIndex] = *p;
			++lineIndex;

			// overflow? if so, resize
			if (lineIndex >= (lineBuffer.size() - 1))
			{
				lineBuffer.resize(lineBuffer.size() * 2);
			}
		}
	}
}

static void RaiseDebugException(const char* buffer, size_t length)
{
	__try
	{
		/*EXCEPTION_RECORD record;
		record.ExceptionAddress = reinterpret_cast<PVOID>(_ReturnAddress());
		record.ExceptionCode = DBG_PRINTEXCEPTION_C;
		record.ExceptionFlags = 0;
		record.NumberParameters = 2;
		record.ExceptionInformation[0] = length + 1;
		record.ExceptionInformation[1] = reinterpret_cast<ULONG_PTR>(buffer);
		record.ExceptionRecord = &record;

		DoNtRaiseException(&record);*/
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		OutputDebugStringA(buffer);
	}
}

void trace(const char* string, const fmt::ArgList& formatList)
{
	std::string buffer;

	try
	{
		buffer = fmt::sprintf(string, formatList);
	}
	catch (fmt::FormatError& e)
	{
		buffer = fmt::sprintf("fmt::FormatError while formatting %s: %s\n", string, e.what());
	}

	static CRITICAL_SECTION dbgCritSec;

	if (!dbgCritSec.DebugInfo)
	{
		InitializeCriticalSectionAndSpinCount(&dbgCritSec, 100);
	}

#ifdef _WIN32
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

	if (!CoreIsDebuggerPresent() && !getenv("CitizenFX_ToolMode"))
	{
		printf("%s", buffer.c_str());
	}
#else
	printf("%s", buffer.c_str());
#endif

	PerformFileLog(buffer.c_str());
}

uint32_t HashRageString(const char* string)
{
	uint32_t hash = 0;
	size_t len = strlen(string);

	for (size_t i = 0; i < len; i++)
	{
		hash += string[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

uint32_t HashString(const char* string)
{
	uint32_t hash = 0;
	size_t len = strlen(string);

	for (size_t i = 0; i < len; i++)
	{
		hash += tolower(string[i]);
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
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
	static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> g_converter;
	return g_converter.to_bytes(wide);
}

std::wstring ToWide(const std::string& narrow)
{
	static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> g_converter;
	return g_converter.from_bytes(narrow);
}