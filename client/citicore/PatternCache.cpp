/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#ifdef GTA_FIVE
static uintptr_t g_currentStub = 0x146000000;

extern "C"
{
	DLL_EXPORT void* AllocateFunctionStubImpl(void* function, int type)
	{
		char* code = (char*)g_currentStub;

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