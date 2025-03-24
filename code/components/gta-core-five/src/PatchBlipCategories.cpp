#include "StdInc.h"

#include <Hooking.h>
#include <jitasm.h>

static char* categoryNames[122];

static void InitializeCategoryNames()
{
	for (int i = 0; i <= 121; ++i)
	{
		categoryNames[i] = new char[13];
		snprintf(categoryNames[i], 13, "BLIP_CAT_%d", i + 12);
	}
}

static const char* GetBlipCategoryName(const uint8_t category)
{
	return categoryNames[category - 1];
}

static HookFunction hookFunction([]()
{
	// Game code expects const char*, so we need to allocate static memory for the category labels
	InitializeCategoryNames();

	// Expand grouped blip categories - IDs above 11 are unused by the game, so we can modify them freely
	{
		static struct : jitasm::Frontend
		{
			intptr_t retSuccess;
			intptr_t retFail;

			void Init(const intptr_t location, const intptr_t failLocation)
			{
				this->retSuccess = location + 8;
				this->retFail = failLocation;
			}

			void InternalMain() override
			{
				// Don't modify categories that are used by the game (<= 9)
				// 10 and 11 are already grouped categories by the game, so we need to keep them
				cmp(edi, 9);
				jle("fail");
				cmp(edi, 254); // 255 is a special category, don't touch it
				jg("fail");

				// Group
				mov(rax, retSuccess);
				jmp(rax);

				// Don't group
				L("fail");
				mov(rax, retFail);
				jmp(rax);
			}
		} patchStub;

		auto location = hook::get_pattern("8D 47 ? 83 F8 ? 77 ? 48 8B 0D");

		const uint8_t jzToFailBytes = *(uint8_t*)((uintptr_t)location + 7);
		auto jzFailLocation = (void*)((uintptr_t)location + 8 + jzToFailBytes);

		patchStub.Init(reinterpret_cast<intptr_t>(location), reinterpret_cast<intptr_t>(jzFailLocation));

		hook::nop(location, 8);
		hook::jump(location, patchStub.GetCode());
	}

	// Changed control flow to inject custom category names
	{
		static struct : jitasm::Frontend
		{
			intptr_t retPlayer;
			intptr_t retFinalize;
			intptr_t retOrig;
			intptr_t retCentreBlip;
			intptr_t getBlipName;

			void Init(const intptr_t location, const intptr_t finalLocation, const intptr_t origLocation, const intptr_t centreBlipLocation, const intptr_t getBlipNameFunc)
			{
				this->retPlayer = location + 8;
				this->retFinalize = finalLocation;
				this->retOrig = origLocation;
				this->retCentreBlip = centreBlipLocation;
				this->getBlipName = getBlipNameFunc;
			}

			void InternalMain() override
			{
				// BLIP_APARTCAT, skip to original code
				dec(esi);
				jz("orig");

				// Split custom categories into two groups: Named (12-133) and Unnamed (134-254)
				cmp(esi, 1);
				jl("defaultFlow");
				cmp(esi, 122);
				jg("defaultFlow");

				// Custom flow
				mov(rcx, rdi);
				mov(rax, getBlipName); // Game function to get the blip name
				call(rax);
				mov(r8, rax);

				movsxd(rcx, esi);
				mov(rax, reinterpret_cast<uintptr_t>(&GetBlipCategoryName)); // BLIP_CAT_X
				call(rax);
				mov(rdx, rax);

				mov(rax, r8);

				// Jump to blip finalization
				mov(rcx, retFinalize);
				jmp(rcx);

				// End of custom flow

				L("defaultFlow");

				// CentreBlip
				test(cl, cl);
				jz("centreBlip");

				// PlayerBlip, also default if nothing hits
				mov(rax, retPlayer);
				jmp(rax);

				L("orig");
				mov(rax, retOrig);
				jmp(rax);

				L("centreBlip");
				mov(rax, retCentreBlip);
				jmp(rax);
			}
		} patchStub;

		auto location = hook::get_pattern("FF CE 74 ? 84 C9");

		const uint8_t jzToOrigBytes = *(uint8_t*)((uintptr_t)location + 3);
		auto jzOrigLocation = (void*)((uintptr_t)location + 4 + jzToOrigBytes);

		const uint8_t jzToCentreBlipBytes = *(uint8_t*)((uintptr_t)location + 7);
		auto jzCentreBlipLocation = (void*)((uintptr_t)location + 8 + jzToCentreBlipBytes);

		auto finalLocation = hook::get_pattern("48 8D 0D ? ? ? ? 48 8B D8 E8 ? ? ? ? 48 8D 15 ? ? ? ? 48 8D 4C 24");

		auto getBlipNameFunc = hook::get_pattern("48 83 EC ? 33 D2 38 51 ? 75");

		patchStub.Init(reinterpret_cast<intptr_t>(location), reinterpret_cast<intptr_t>(finalLocation), reinterpret_cast<intptr_t>(jzOrigLocation), reinterpret_cast<intptr_t>(jzCentreBlipLocation), reinterpret_cast<intptr_t>(getBlipNameFunc));

		hook::nop(location, 8);
		hook::jump(location, patchStub.GetCode());
	}
});
