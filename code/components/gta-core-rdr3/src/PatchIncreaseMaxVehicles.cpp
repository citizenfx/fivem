#include <StdInc.h>
#include <PoolSizesState.h>
#include "Hooking.h"

static HookFunction hookFunction([]()
{
	constexpr size_t kDefaultMaxVehicles = 40;

	int64_t increaseSize = 0;

	// We use "CNetObjVehicle" as the increase request for vehicles and all other components.
	auto sizeIncreaseEntry = fx::PoolSizeManager::GetIncreaseRequest().find("CNetObjVehicle");
	if (sizeIncreaseEntry != fx::PoolSizeManager::GetIncreaseRequest().end())
	{
		increaseSize = sizeIncreaseEntry->second;
	}

	// Set total desired vehicles.
	uint8_t patch[] = {
		0xB9,              // mov ecx, imm32
		0x00, 0x00, 0x00, 0x00, // placholder for the value
		0x90               // nop
	};

	uint32_t finalSize = static_cast<uint32_t>(kDefaultMaxVehicles + increaseSize);
	std::memcpy(&patch[1], &finalSize, sizeof(finalSize));
	memcpy(hook::get_pattern("8B 0D ? ? ? ? C7 05 ? ? ? ? ? ? ? ? 89 05"), patch, sizeof(patch));

	// Set max amount of vehicles
	auto address = hook::get_pattern<char>("44 89 0D ? ? ? ? 48 89 15");
	auto patchValue = [&](uintptr_t addrOffset) {
		auto currentAddr = address + addrOffset;
		int32_t relative = *(int32_t*)(currentAddr + 3);
		auto absolute = currentAddr + 7 + relative; // RIP + relative
		*absolute = kDefaultMaxVehicles + increaseSize;
	};

	patchValue(0);
	patchValue(20);
	patchValue(27);

	// Mission car limitation, by default is 64
	{
		auto location = hook::get_pattern<char>("83 3D ? ? ? ? ? 0F 29 70 ? 0F 28 F1");
		hook::put<uint8_t>(location, 0x81); // comp with immediate
		hook::put<uint32_t>(location + 0x6, kDefaultMaxVehicles + increaseSize + 24);
		hook::nop(location + 0xA, 0x7);

		static struct : jitasm::Frontend
		{
			uintptr_t returnLocation;
			
			virtual void InternalMain() override
			{
				// Original Instructions
				movaps(xmmword_ptr[rax-0x28], xmm6);
				movaps(xmm6, xmm1);
				unpcklps(xmm6, xmm0);

				mov(rdi, returnLocation);
				jmp(rdi);
			}
		} stub;
		stub.returnLocation = (uintptr_t)location + 0x11;
		hook::jump_reg<7>(location + 0xA, stub.GetCode());

		hook::put<uint16_t>(location + 0x1E, 0x830F); // jae loc_... (unsigned >=)
	}
	
	// Allow registration of script/mission vehicles up to and past the 40 limit.
	hook::put<uint32_t>(hook::get_pattern("BB ? ? ? ? E9 ? ? ? ? B8 ? ? ? ? 83 FF", 0x1), kDefaultMaxVehicles + increaseSize);

	// Ignore hard coded limit of CVehicle to 128
	hook::put<char>(hook::get_pattern("76 ? BA ? ? ? ? 41 B8 ? ? ? ? 83 C9 ? E8 ? ? ? ? 33 D2 8B CB"), 0xEB);
});