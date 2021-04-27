/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#if defined(GTA_FIVE) || defined(IS_RDR3)
#include <Hooking.h>

#if defined(IS_RDR3)
static uintptr_t g_currentStub = 0x148000000;
#else
static uintptr_t g_currentStub = 0x146000000;
#endif

extern "C"
{
	DLL_EXPORT void* AllocateFunctionStubImpl(void* function, int type)
	{
		char* code = (char*)g_currentStub + hook::baseAddressDifference;

		DWORD oldProtect;
		VirtualProtect(code, 15, PAGE_EXECUTE_READWRITE, &oldProtect);

		*(uint8_t*)code = 0x48;
		*(uint8_t*)(code + 1) = 0xb8 | type;

		*(uint64_t*)(code + 2) = (uint64_t)function;

		*(uint16_t*)(code + 10) = 0xE0FF | (type << 8);

		*(uint64_t*)(code + 12) = 0xCCCCCCCCCCCCCCCC;

		g_currentStub += 20;

		return code;
	}

	DLL_EXPORT void* AllocateStubMemoryImpl(size_t size)
	{
		char* code = (char*)g_currentStub + hook::baseAddressDifference;

		DWORD oldProtect;
		VirtualProtect(code, size, PAGE_EXECUTE_READWRITE, &oldProtect);

		g_currentStub += size;

		return code;
	}
}
#endif

#ifndef IS_FXSERVER
#include <Hooking.Patterns.h>

static std::multimap<uint64_t, uintptr_t> g_hints;

extern "C" CORE_EXPORT auto CoreGetPatternHints()
{
	return &g_hints;
}

static InitFunction initFunction([]()
{
	if (wcsstr(GetCommandLineW(), L"_ROSLauncher") || wcsstr(GetCommandLineW(), L"_ROSService"))
	{
		if (getenv("CitizenFX_ToolMode"))
		{
			g_currentStub = 0x140000000 + 0x02E23600;
		}
	}

	std::wstring hintsFile = MakeRelativeCitPath(L"citizen\\hints.dat");
	FILE* hints = _wfopen(hintsFile.c_str(), L"rb");

	if (hints)
	{
		while (!feof(hints))
		{
			uint64_t hash;
			uintptr_t hint;

			fread(&hash, 1, sizeof(hash), hints);
			fread(&hint, 1, sizeof(hint), hints);

			hook::pattern::hint(hash, hint);
		}

		fclose(hints);
	}
});
#endif
