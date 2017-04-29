/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"
#include "base64.h"

#include <Error.h>

#include <botan/auto_rng.h>
#include <botan/rsa.h>
#include <botan/sha160.h>
#include <botan/pubkey.h>

#include "RSAKey.h"

static HookFunction hookFunction([] ()
{
	HMODULE rosDll = LoadLibrary(L"ros.dll");

	if (rosDll != nullptr)
	{
		((void(*)(const wchar_t*))GetProcAddress(rosDll, "run"))(MakeRelativeCitPath(L"").c_str());
	}

	if (GetModuleHandle(L"clr.dll") != nullptr)
	{
		FatalError(__FUNCTION__ " can not execute while the Common Language Runtime is loaded. Please remove any .NET-based plugins/CitizenFX components from your game installation, and try again.");
	}
});