/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <jitasm.h>

#include "Hooking.h"
#include <Error.h>

struct GameErrorData
{
	std::string errorName;
	std::string errorDescription;

	GameErrorData()
	{
	}

	GameErrorData(const std::string& errorName, const std::string& errorDescription)
		: errorName(errorName), errorDescription(errorDescription)
	{
	}
};

static GameErrorData LookupError(uint32_t hash)
{
#ifdef IS_RDR3
	FILE* f = _wfopen(fmt::format(L"{}/x64/data/errorcodes/american.txt", MakeRelativeGamePath(L"")).c_str(), L"r");
#else
	FILE* f = _wfopen(fmt::format(L"{}/update/x64/data/errorcodes/american.txt", MakeRelativeGamePath(L"")).c_str(), L"r");
#endif

	if (f)
	{
		static char line[8192] = { 0 };

		while (fgets(line, 8191, f))
		{
			if (line[0] == '[')
			{
				strrchr(line, ']')[0] = '\0';

				if (HashString(&line[1]) == hash)
				{
					char data[8192] = { 0 };
					fgets(data, 8191, f);

					return GameErrorData{ &line[1], data };
				}
			}
		}
	}

	return GameErrorData{};
}

static void ErrorDo(uint32_t error
#ifdef IS_RDR3
	, uint32_t fileHash, uint32_t fileLine, bool shouldTerminate
#endif
)
{
	if (error == HashString("ERR_GEN_INVALID"))
	{
		FatalError("Invalid rage::fiPackfile encryption type specified. If you have any modified game files, please remove or verify them. See http://rsg.ms/verify for more information.");
	}

#ifdef IS_RDR3
	if (error == 0xFFFFFFFF)
	{
		FatalError("RAGE error: 0x%X:%d", fileHash, fileLine);
	}
#endif

	auto errData = LookupError(error);
	uint64_t retAddr = (uint64_t)_ReturnAddress();

	if (errData.errorName.empty())
	{
		errData.errorName = "UNKNOWN";
		errData.errorDescription = "";
	}

	// save error pickup data for the error
	FatalErrorNoExcept("RAGE error: %s\nA game error (at %016llx) caused the game to stop working.\n\n%s", errData.errorName, hook::get_unadjusted(retAddr), errData.errorDescription);

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
#ifdef IS_RDR3
	char* errorFunc = reinterpret_cast<char*>(hook::get_call(hook::pattern("BA 0A 72 60 85 83 C9 FF E8").count(1).get(0).get<void>(8)));
	hook::jump(hook::get_call(errorFunc + 7), ErrorDo);
#else
	char* errorFunc = reinterpret_cast<char*>(hook::get_call(hook::pattern("B9 84 EC F4 C6 E8").count(1).get(0).get<void>(5)));
	hook::jump(hook::get_call(errorFunc + 6), ErrorDo);
	hook::jump(errorFunc, ErrorDo);
#endif
});
