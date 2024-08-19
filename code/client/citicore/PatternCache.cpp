/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#if defined(GTA_FIVE) || defined(IS_RDR3)
#include <Hooking.h>

static uintptr_t g_currentStub = hook::exe_end();

extern "C"
{
	DLL_EXPORT void* AllocateStubMemoryImpl(size_t size)
	{
		// Try and pick a sensible alignment
		size_t alignMask = ((size > 64) ? 16 : 8) - 1;

		g_currentStub = (g_currentStub + alignMask) & ~alignMask;

		char* code = (char*)g_currentStub + hook::baseAddressDifference;

		DWORD oldProtect;
		VirtualProtect(code, size, PAGE_EXECUTE_READWRITE, &oldProtect);

		g_currentStub += size;

		return code;
	}

	DLL_EXPORT void* AllocateFunctionStubImpl(void* function, int type)
	{
		char* code = (char*) AllocateStubMemoryImpl(20);

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


}
#endif

#ifndef IS_FXSERVER
#include "CrossBuildRuntime.h"
#include <Hooking.Patterns.h>

static std::multimap<uint64_t, uintptr_t> g_hints;

extern "C" CORE_EXPORT auto CoreGetPatternHints()
{
	return &g_hints;
}

static InitFunction initFunction([]()
{
#ifndef GTA_NY
	if (wcsstr(GetCommandLineW(), L"_ROSLauncher") || wcsstr(GetCommandLineW(), L"_ROSService"))
	{
		if (getenv("CitizenFX_ToolMode"))
		{
			g_currentStub = 0x140000000 + 0x0339A600;
		}
	}
#endif

	// don't load hints for chromebrowser subprocess
	if (wcsstr(GetCommandLineW(), L"_ChromeBrowser"))
	{
		return;
	}

	std::wstring hintsFile = MakeRelativeCitPath(ToWide(fmt::sprintf("data\\cache\\hints_%s.dat", xbr::GetCurrentGameBuildString())));
	FILE* hints = _wfopen(hintsFile.c_str(), L"rb");
	size_t numHints = 0;

	if (hints)
	{
		while (!feof(hints))
		{
			uint64_t hash;
			uintptr_t hint;

			fread(&hash, 1, sizeof(hash), hints);
			fread(&hint, 1, sizeof(hint), hints);

			hook::pattern::hint(hash, hint);

			numHints++;
		}

		fclose(hints);
	}

	// over 48k hints? that might be anomalous: regenerate them
	if (numHints > (48 * 1024))
	{
		_wunlink(hintsFile.c_str());
	}
});
#endif
