/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

void DLL_EXPORT Citizen_PatternSaveHint(uint64_t hash, uintptr_t hint)
{
	fwPlatformString hintsFile = MakeRelativeCitPath(L"citizen\\hints.dat");
	FILE* hints = _pfopen(hintsFile.c_str(), _P("ab"));

	if (hints)
	{
		fwrite(&hash, 1, sizeof(hash), hints);
		fwrite(&hint, 1, sizeof(hint), hints);

		fclose(hints);
	}
}

#ifdef GTA_FIVE
static uintptr_t g_currentStub = 0x146000000;

extern "C"
{
	DLL_EXPORT void* AllocateFunctionStubImpl(void* function)
	{
		char* code = (char*)g_currentStub;

		DWORD oldProtect;
		VirtualProtect(code, 15, PAGE_EXECUTE_READWRITE, &oldProtect);

		*(uint8_t*)code = 0x48;
		*(uint8_t*)(code + 1) = 0xb8;

		*(uint64_t*)(code + 2) = (uint64_t)function;

		*(uint16_t*)(code + 10) = 0xE0FF;

		*(uint64_t*)(code + 12) = 0xCCCCCCCCCCCCCCCC;

		g_currentStub += 20;

		return code;
	}
}
#endif