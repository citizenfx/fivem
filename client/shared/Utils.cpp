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
	return L"S:\\games\\steam\\steamapps\\common\\grand theft auto iv\\gtaiv\\";
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