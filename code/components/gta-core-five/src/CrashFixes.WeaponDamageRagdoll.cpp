#include "StdInc.h"

#include <jitasm.h>
#include "Hooking.Patterns.h"

#include "CrossBuildRuntime.h"

static HookFunction hookFunction([]()
{
	// CWeaponDamage::_CreateRagdollTask
	//
	// There is an edge case where the code compares the current active task's type against CTaskNMControl
	// but the player may not have an active task at all
	auto location = hook::get_pattern<char>("B8 ? ? ? ? 66 41 39 44 24 ? 0F 84 ? ? ? ? 48 8B 85");

	auto taskType = *(uint16_t*)(location + 1);
	auto taskTypeOffset = *(uint8_t*)(location + 10);

	// `jz <equalLocation>` handles the case where the task should be used
	auto equalLocation = *(int32_t*)(location + 13) + (intptr_t)(location + 17);
	// The case where the task should not be used (the patch will jump to here if null)
	auto notEqualLocation = (intptr_t)(location + 17);

	static struct : jitasm::Frontend
	{
		uint16_t taskType;
		uint32_t taskTypeOffset;
		intptr_t successLocation;
		intptr_t failLocation;

		void Init(uint16_t taskType, uint32_t taskTypeOffset, intptr_t successLocation, intptr_t failLocation)
		{
			this->taskType = taskType;
			this->taskTypeOffset = taskTypeOffset;
			this->successLocation = successLocation;
			this->failLocation = failLocation;
		}

		virtual void InternalMain() override
		{
			// r12 = active task, which may be null here
			test(r12, r12);
			jz("fail");

			// Original code
			cmp(word_ptr[r12 + taskTypeOffset], taskType);
			jne("fail");

			mov(rax, successLocation);
			jmp(rax);

			L("fail");
			mov(rax, failLocation);
			jmp(rax);
		}
	} stub;

	stub.Init(taskType, taskTypeOffset, equalLocation, notEqualLocation);

	// Patch over `cmp [r12+34h], ax; jz <equalLocation>`
	hook::nop(location + 5, 12);
	hook::jump(location + 5, stub.GetCode());
});
