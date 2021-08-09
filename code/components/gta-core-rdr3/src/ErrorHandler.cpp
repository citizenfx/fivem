/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"
#include <Error.h>

static void ErrorDo(uint32_t error, uint32_t fileHash, uint32_t fileLine, bool shouldTerminate)
{
	if (error == 0xFFFFFFFF)
	{
		FatalError("RAGE error: 0x%X:%d", fileHash, fileLine);
	}

	// provide pickup file for minidump handler to use
	FILE* dbgFile = _wfopen(MakeRelativeCitPath(L"data\\cache\\error_out").c_str(), L"wb");

	if (dbgFile)
	{
		fwrite(&error, 1, 4, dbgFile);

		uint64_t retAddr = (uint64_t)_ReturnAddress();
		fwrite(&retAddr, 1, 8, dbgFile);

		fclose(dbgFile);
	}

	// overwrite the CALL address with a marker containing the error code, then move the return address ahead
	// this should lead to crash dumps showing the exact location of the CALL as the exception address
	{
		DWORD oldProtect;

		static struct : jitasm::Frontend
		{
			uint32_t error;

			virtual void InternalMain() override
			{
				mov(rax, 0x1000000000 | error);
				mov(dword_ptr[rax], 0xDEADBADE);
			}
		} code;

		code.error = error;

		// assemble *first* as GetCodeSize does not automatically call Assemble
		code.Assemble();

		// 5: CALL size
		// 6: size of mov dword_ptr[rax], ...
		char* retAddr = (char*)_ReturnAddress() - 5 - code.GetCodeSize() + 6;

		VirtualProtect(retAddr, code.GetCodeSize(), PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy(retAddr, code.GetCode(), code.GetCodeSize());

		// for good measure
		FlushInstructionCache(GetCurrentProcess(), retAddr, code.GetCodeSize());

		// jump to the new return address
		*(void**)_AddressOfReturnAddress() = retAddr;
	}
}

static HookFunction hookFunction([]()
{
	char* errorFunc = reinterpret_cast<char*>(hook::get_call(hook::pattern("BA 0A 72 60 85 83 C9 FF E8").count(1).get(0).get<void>(8)));
	hook::jump(hook::get_call(errorFunc + 7), ErrorDo);
});
