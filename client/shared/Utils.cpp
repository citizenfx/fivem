#include "StdInc.h"
#include "Utils.h"

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
	// hacky for now :)
	if (!_stricmp(getenv("COMPUTERNAME"), "fallarbor"))
	{
		return L"S:\\games\\steam\\steamapps\\common\\grand theft auto iv\\gtaiv\\";
	}
	else if (!_stricmp(getenv("COMPUTERNAME"), "avail"))
	{
		return L"D:\\program files\\steam\\steamapps\\common\\grand theft auto iv\\gtaiv\\";
	}
	else if (!_stricmp(getenv("COMPUTERNAME"), "snowpoint"))
	{
		return L"E:\\steam\\steamapps\\common\\grand theft auto iv\\gtaiv\\";
	}
	else
	{
		static wchar_t buffer[512];

		if (!buffer[0])
		{
			GetFullPathName(L"..\\gtaiv\\", _countof(buffer), buffer, nullptr);
		}

		return buffer;
	}
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
	static char buffer[BUFFER_COUNT][BUFFER_LENGTH];
	static int currentBuffer;
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

void GlobalError(const char* string, ...)
{
	static char buffer[BUFFER_LENGTH];

	va_list ap;
	va_start(ap, string);
	int length = _vsnprintf_s(buffer, BUFFER_LENGTH, string, ap);
	va_end(ap);

	if (length >= BUFFER_LENGTH)
	{
		GlobalError("Attempted to overrun string in call to GlobalError()!");
	}

	HWND wnd = NULL;

	// TODO: get the game window

	MessageBoxA(wnd, buffer, "CitizenFX Fatal Error", MB_OK | MB_ICONSTOP);
	ExitProcess(1);
}