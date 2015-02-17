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

static InitFunctionBase* g_initFunctions;

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

const char* va(const char* string, ...)
{
	static __thread int currentBuffer;
	static __thread char* buffer;

	if (!buffer)
	{
		buffer = new char[BUFFER_COUNT * BUFFER_LENGTH];
	}

	int thisBuffer = currentBuffer;

	va_list ap;
	va_start(ap, string);
	int length = vsnprintf(&buffer[thisBuffer * BUFFER_LENGTH], BUFFER_LENGTH, string, ap);
	va_end(ap);

	if (length >= BUFFER_LENGTH)
	{
		//GlobalError("Attempted to overrun string in call to va()!");
		exit(1);
	}

	buffer[(thisBuffer * BUFFER_LENGTH) + BUFFER_LENGTH - 1] = '\0';

	currentBuffer = (currentBuffer + 1) % BUFFER_COUNT;

	return &buffer[thisBuffer * BUFFER_LENGTH];
}

const wchar_t* va(const wchar_t* string, ...)
{
	static __thread int currentBuffer;
	static __thread wchar_t* buffer;

	if (!buffer)
	{
		buffer = new wchar_t[BUFFER_COUNT * BUFFER_LENGTH];
	}

	int thisBuffer = currentBuffer;

	va_list ap;
	va_start(ap, string);
	int length = vswprintf(&buffer[thisBuffer * BUFFER_LENGTH], BUFFER_LENGTH, string, ap);
	va_end(ap);

	if (length >= BUFFER_LENGTH)
	{
		//GlobalError("Attempted to overrun string in call to va()!");
		exit(1);
	}

	buffer[(thisBuffer * BUFFER_LENGTH) + BUFFER_LENGTH - 1] = '\0';

	currentBuffer = (currentBuffer + 1) % BUFFER_COUNT;

	return &buffer[thisBuffer * BUFFER_LENGTH];
}

void trace(const char* string, ...)
{
	static char buffer[BUFFER_LENGTH];

	va_list ap;
	va_start(ap, string);
	int length = vsnprintf(buffer, BUFFER_LENGTH, string, ap);
	va_end(ap);

	if (length >= BUFFER_LENGTH)
	{
		//GlobalError("Attempted to overrun string in call to trace()!");
		exit(1);
	}

#ifdef _WIN32
	OutputDebugStringA(buffer);
#endif

	printf("%s", buffer);

	// TODO: write to a log file too, if enabled?
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