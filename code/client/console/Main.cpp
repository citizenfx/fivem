/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

int wmain(int argc, const wchar_t** argv)
{
	SetEnvironmentVariable(L"CitizenFX_ToolMode", L"1");
	_putenv("CitizenFX_ToolMode=1");

	// path environment appending of our primary directories
	static wchar_t pathBuf[32768];
	GetEnvironmentVariable(L"PATH", pathBuf, sizeof(pathBuf));

	std::wstring newPath = MakeRelativeCitPath(L"bin") + L";" + MakeRelativeCitPath(L"") + L";" + std::wstring(pathBuf);

	SetEnvironmentVariable(L"PATH", newPath.c_str());

	SetDllDirectory(MakeRelativeCitPath(L"bin").c_str()); // to prevent a) current directory DLL search being disabled and b) xlive.dll being taken from system if not overridden

	// initializing toolmode
	HMODULE coreRT = LoadLibrary(MakeRelativeCitPath(L"CoreRT.dll").c_str());

	if (coreRT)
	{
		auto toolProc = (void(*)())GetProcAddress(coreRT, "ToolMode_Init");

		if (toolProc)
		{
			toolProc();
		}
		else
		{
			printf("Couldn't load ToolMode_Init from CoreRT.dll.\n");
			return 1;
		}
	}
	else
	{
		printf("Couldn't load CoreRT.dll.\n");
		return 1;
	}

	return 0;
}