/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"

#include <Error.h>

extern HANDLE g_rosClearedEvent;

static InitFunction initfunction([]()
{
	HMODULE rosDll = LoadLibrary(L"ros.dll");
	if (rosDll != nullptr)
	{
		auto runEarlier = ((void (*)(const wchar_t*))GetProcAddress(rosDll, "runEarlier"));
		
		if (runEarlier)
		{
			runEarlier(MakeRelativeCitPath(L"").c_str());
		}
	}
});

static HookFunction hookFunction([] ()
{
	g_rosClearedEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

	HMODULE rosDll = LoadLibrary(L"ros.dll");
	if (rosDll != nullptr)
	{
		((void (*)(const wchar_t*))GetProcAddress(rosDll, "runEarly"))(MakeRelativeCitPath(L"").c_str());
	}

	std::thread([=]()
	{
		if (rosDll != nullptr)
		{
			((void(*)(const wchar_t*))GetProcAddress(rosDll, "run"))(MakeRelativeCitPath(L"").c_str());
		}
		
		SetEvent(g_rosClearedEvent);

		if (GetModuleHandle(L"clr.dll") != nullptr)
		{
			FatalError(__FUNCTION__ " can not execute while the Common Language Runtime is loaded. Please remove any .NET-based plugins/CitizenFX components from your game installation, and try again.");
		}
	}).detach();
});
