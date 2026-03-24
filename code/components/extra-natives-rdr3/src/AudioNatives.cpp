#include "StdInc.h"

#include "ClientConfig.h"
#include "Hooking.Patterns.h"

static HookFunction hookFunction([]()
{
	// Disable the audDynamicMixer::StartScene calls to prevent audio from cutting out on death.
	{
		// 0x48A61B5F, ambient sounds
		{
			auto location = hook::get_pattern<char>("C7 45 ? ? ? ? ? E8 ? ? ? ? 48 8D 4D ? E8 ? ? ? ? 8A 83 ? ? ? ? 48 8B 0D ? ? ? ? BF");
			auto failLocation = location + 34;
			
			static struct : jitasm::Frontend
			{
				uintptr_t successLocation;
				uintptr_t failLocation;

				void Init(uintptr_t success, uintptr_t fail)
				{
					successLocation = success;
					failLocation = fail;
				}
				
				virtual void InternalMain() override
				{
					mov(dword_ptr[rbp+0x20], 0x48A61B5F);

					mov(rax, (uintptr_t)&g_clientConfigBits);
					bt(qword_ptr[rax], (int)ClientConfigFlag::DisableDeathAudioScene);
					jnc("success");

					mov(rax, failLocation);
					jmp(rax);

					L("success");
					mov(rax, successLocation);
					jmp(rax);
				}
			} stub;
			
			stub.Init((uintptr_t)location + 0x7, (uintptr_t)failLocation);
			hook::nop(location, 0x7);
			hook::jump(location, stub.GetCode());
		}

		// 0x8B8B8CB1, related to scripted sounds?
		{
			auto location = hook::get_pattern("BA ? ? ? ? 49 89 73");
			auto failLocation = hook::get_pattern("48 8D 55 ? 0F 29 45 ? 48 8B CB E8 ? ? ? ? 4C 8D 9C 24", 16);
			
			static struct : jitasm::Frontend
			{
				uintptr_t successLocation;
				uintptr_t failLocation;

				void Init(uintptr_t success, uintptr_t fail)
				{
					successLocation = success;
					failLocation = fail;
				}
				
				virtual void InternalMain() override
				{
					mov(edx, 0x8B8B8CB1);

					mov(rax, (uintptr_t)&g_clientConfigBits);
					bt(qword_ptr[rax], (int)ClientConfigFlag::DisableDeathAudioScene);
					jnc("success");

					mov(rax, failLocation);
					jmp(rax);

					L("success");
					mov(rax, successLocation);
					jmp(rax);
				}
			} stub;
			
			stub.Init((uintptr_t)location + 0x5, (uintptr_t)failLocation);
			hook::nop(location, 0x5);
			hook::jump(location, stub.GetCode());
		}
	}
});