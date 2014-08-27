#include "StdInc.h"
#include "Utils.h"
#include "GameInit.h"
#include <sstream>
#include <iomanip>

std::wstring GetAbsoluteCitPath()
{
	static std::wstring citizenPath;

	if (!citizenPath.size())
	{
		wchar_t modulePath[512];
		GetModuleFileName(GetModuleHandle(nullptr), modulePath, sizeof(modulePath) / sizeof(wchar_t));

		wchar_t* dirPtr = wcsrchr(modulePath, L'\\');

		// we do not check if dirPtr happens to be 0, as any valid absolute Win32 file path contains at least one backslash
		dirPtr[1] = '\0';

		citizenPath = modulePath;
	}

	return citizenPath;
}

std::wstring GetAbsoluteGamePath()
{
	static std::wstring gamePath;

	if (!gamePath.size())
	{
		std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");

		if (GetFileAttributes(fpath.c_str()) == INVALID_FILE_ATTRIBUTES)
		{
			return L"null";
		}

		wchar_t path[512];

		GetPrivateProfileString(L"Game", L"IVPath", L"", path, _countof(path), fpath.c_str());

		gamePath = path;
		gamePath += L"\\";
	}

	return gamePath;
}

static InitFunctionBase* g_initFunctions;

void InitFunctionBase::Register()
{
	m_next = g_initFunctions;
	g_initFunctions = this;
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
	static __declspec(thread) char buffer[BUFFER_COUNT][BUFFER_LENGTH];
	static __declspec(thread) int currentBuffer;
	int thisBuffer = currentBuffer;

	va_list ap;
	va_start(ap, string);
	int length = _vsnprintf_s(buffer[thisBuffer], BUFFER_LENGTH, string, ap);
	va_end(ap);

	if (length >= BUFFER_LENGTH)
	{
		GlobalError("Attempted to overrun string in call to va()!");
	}

	buffer[thisBuffer][BUFFER_LENGTH - 1] = '\0';

	currentBuffer = (currentBuffer + 1) % BUFFER_COUNT;

	return buffer[thisBuffer];
}

const wchar_t* va(const wchar_t* string, ...)
{
	static __declspec(thread) wchar_t buffer[BUFFER_COUNT][BUFFER_LENGTH];
	static __declspec(thread) int currentBuffer;
	int thisBuffer = currentBuffer;

	va_list ap;
	va_start(ap, string);
	int length = _vsnwprintf_s(buffer[thisBuffer], BUFFER_LENGTH, string, ap);
	va_end(ap);

	if (length >= BUFFER_LENGTH)
	{
		GlobalError("Attempted to overrun string in call to va()!");
	}

	buffer[thisBuffer][BUFFER_LENGTH - 1] = '\0';

	currentBuffer = (currentBuffer + 1) % BUFFER_COUNT;

	return buffer[thisBuffer];
}

void trace(const char* string, ...)
{
	static char buffer[BUFFER_LENGTH];

	va_list ap;
	va_start(ap, string);
	int length = _vsnprintf_s(buffer, BUFFER_LENGTH, string, ap);
	va_end(ap);

	if (length >= BUFFER_LENGTH)
	{
		GlobalError("Attempted to overrun string in call to trace()!");
	}

	OutputDebugStringA(buffer);
	printf(buffer);

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

std::string url_encode(const std::string &value)
{
	std::ostringstream escaped;
	escaped.fill('0');
	escaped << std::hex;

	for (std::string::const_iterator i = value.begin(), n = value.end(); i != n; ++i)
	{
		std::string::value_type c = (*i);
		if ((isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') && c != '+')
		{
			escaped << c;
		}
		else if (c == ' ')
		{
			escaped << '+';
		}
		else
		{
			escaped << '%' << std::setw(2) << ((int)c) << std::setw(0);
		}
	}

	return escaped.str();
}

#include <direct.h>
#include <io.h>

void CreateDirectoryAnyDepth(const char *path)
{
	char opath[MAX_PATH];
	char *p;
	size_t len;
	strncpy_s(opath, path, sizeof(opath));
	len = strlen(opath);
	if (opath[len - 1] == L'/')
		opath[len - 1] = L'\0';

	for (p = opath; *p; p++)
	{
		if (*p == L'/' || *p == L'\\')
		{
			*p = L'\0';
			if (_access(opath, 0))
				_mkdir(opath);
			*p = L'\\';
		}
	}
	if (_access(opath, 0))
		_mkdir(opath);
}