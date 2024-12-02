#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <Hooking.FlexStruct.h>

#include <jitasm.h>

uint32_t taskEntityOffset = 0;
uint32_t dynamicEntityComponentOffset = 0;
uint32_t animDirectorOffset = 0;
std::once_flag traceOnceFlag;

uint32_t parachuteObjectOffset = 0;
uint32_t drawHandlerOffset = 0;

uint32_t taskTakeOffPedVariationPropOffset = 0;

template<bool UseTaskPropEntity>
bool IsTaskFSMEntityAnimDirectorValid(hook::FlexStruct* self)
{
	hook::FlexStruct* taskEntity = self->Get<hook::FlexStruct*>(UseTaskPropEntity ? taskTakeOffPedVariationPropOffset : taskEntityOffset);

	// We only care about validating a entity's animDirector if the entity exists already
	// as entity creation might be deferred for tasks such as CTaskParachuteObject
	if (taskEntity)
	{
		hook::FlexStruct* dynamicEntityComponent = taskEntity->Get<hook::FlexStruct*>(dynamicEntityComponentOffset);

		if (!dynamicEntityComponent)
		{
			return false;
		}

		hook::FlexStruct* animDirector = dynamicEntityComponent->Get<hook::FlexStruct*>(animDirectorOffset);

		if (!animDirector)
		{
			std::call_once(traceOnceFlag, []()
			{
				trace("Fake parachute model crash prevented [CFX-1907]\n");
			});

			return false;
		}
	}

	return true;
}

uint64_t (*g_CTaskParachuteObject_UpdateFSM)(hook::FlexStruct* self, int state, int subState);
uint64_t CTaskParachuteObject_UpdateFSM(hook::FlexStruct* self, int state, int subState)
{
	if (!IsTaskFSMEntityAnimDirectorValid<false>(self))
	{
		return 0;
	}

	return g_CTaskParachuteObject_UpdateFSM(self, state, subState);
}

uint64_t (*g_CTaskTakeOffPedVariation_UpdateFSM)(hook::FlexStruct* self, int state, int subState);
uint64_t CTaskTakeOffPedVariation_UpdateFSM(hook::FlexStruct* self, int state, int subState)
{
	if (!IsTaskFSMEntityAnimDirectorValid<true>(self))
	{
		return 0;
	}

	return g_CTaskTakeOffPedVariation_UpdateFSM(self, state, subState);
}

uint64_t (*g_CTaskTakeOffPedVariation_AttachProp)(hook::FlexStruct* self);
uint64_t CTaskTakeOffPedVariation_AttachProp(hook::FlexStruct* self)
{
	if (!IsTaskFSMEntityAnimDirectorValid<true>(self))
	{
		return 0;
	}

	return g_CTaskTakeOffPedVariation_AttachProp(self);
}

void (*g_CTaskParachute_SetParachuteTintIndex)(hook::FlexStruct* self);
void CTaskParachute_SetParachuteTintIndex(hook::FlexStruct* self)
{
	hook::FlexStruct* parachuteObject = self->Get<hook::FlexStruct*>(parachuteObjectOffset);
	if (!parachuteObject)
	{
		return;
	}

	hook::FlexStruct* drawHandler = parachuteObject->Get<hook::FlexStruct*>(drawHandlerOffset);
	if (!drawHandler)
	{
		return;
	}

	g_CTaskParachute_SetParachuteTintIndex(self);
}

static HookFunction hookFunction([]
{
	g_CTaskParachuteObject_UpdateFSM = hook::trampoline(hook::get_pattern<void>("48 83 EC ? 85 D2 0F 88 ? ? ? ? 75 ? 45 85 C0 75 ? 48 8B 41"), &CTaskParachuteObject_UpdateFSM);
	g_CTaskParachute_SetParachuteTintIndex = hook::trampoline(hook::get_pattern<void>("48 89 5C 24 ? 57 48 83 EC ? 48 8B 81 ? ? ? ? 48 8B D9 48 85 C0 74 ? 48 8B 40"), &CTaskParachute_SetParachuteTintIndex);

	taskEntityOffset = *hook::get_pattern<uint8_t>("48 8B 5E ? 4C 8D 05", 3);
	dynamicEntityComponentOffset = *hook::get_pattern<uint8_t>("48 8B 43 ? 48 85 C0 74 ? 48 8B 78 ? 48 8B CF E8 ? ? ? ? 48 8B D5", 3);
	animDirectorOffset = *hook::get_pattern<uint8_t>("48 8B 78 ? 48 8B CF E8 ? ? ? ? 48 8B D5", 3);

	parachuteObjectOffset = *hook::get_pattern<uint32_t>("48 8B 81 ? ? ? ? 48 8B D9 48 85 C0 74 ? 48 8B 40 ? 48 8B 78", 3);
	drawHandlerOffset = *hook::get_pattern<uint8_t>("48 8B 40 ? 48 8B 78", 3);

	// Vehicle shader type checks for fragments using vehicle models
	{
		static struct : jitasm::Frontend
		{
			intptr_t retSuccess;
			intptr_t retFail;

			void Init(intptr_t success, intptr_t fail)
			{
				this->retSuccess = success;
				this->retFail = fail;
			}

			void InternalMain() override
			{
				cmp(byte_ptr[rbp + 0x0C8], 0);		//    if ( *(rbp + 0x0C8) )
				jz("fail");							//    {
													//
				mov(rax, qword_ptr[rsi + 0x20]);	//        void* shader = *(rsi + 0x20);;
													//
				mov(al, byte_ptr[rax + 0xA]);		//
				and(al, 0xF);						//
				cmp(al, 3);							//        if ( ((shader + 0xA) & 0xF) == 3 /* vehicle */ )
				jz("fail");							//        {
													//
				mov(rax, retSuccess);				//
				jmp(rax);							//            [run original code]
													//
				L("fail");							//        }
				mov(rax, retFail);					//
				jmp(rax);							//
			}
		} patchStub;

		auto location = hook::get_pattern<char>("80 BD ? ? ? ? ? 0F 84 ? ? ? ? 48 8B 5E");

		const auto success = reinterpret_cast<intptr_t>(location) + 13;
		const auto fail = success + *reinterpret_cast<uint32_t*>(location + 9);

		patchStub.Init(success, fail);

		hook::nop(location, 7);
		hook::jump(location, patchStub.GetCode());
	}

	taskTakeOffPedVariationPropOffset = *hook::get_pattern<uint32_t>("48 8B 81 ? ? ? ? 33 C9 48 85 C0 74 ? 48 8B 40 ? 48 85 C0", 3);

	g_CTaskTakeOffPedVariation_UpdateFSM = hook::trampoline(hook::get_pattern<void>("48 83 EC ? 4C 8B C9 85 D2 78 ? B8"), &CTaskTakeOffPedVariation_UpdateFSM);
	g_CTaskTakeOffPedVariation_AttachProp = hook::trampoline(hook::get_pattern<void>("48 89 5C 24 ? 57 48 83 EC ? 0F 29 74 24 ? 48 8B D9 E8 ? ? ? ? 84 C0"), &CTaskTakeOffPedVariation_AttachProp);
});
