#include <StdInc.h>

#include <jitasm.h>

#include <Resource.h>
#include <fxScripting.h>
#include <ScriptEngine.h>

#include <Hooking.h>
#include <GameInit.h>
#include <CoreConsole.h>

static int32_t* CTaskMelee_AttackersPrimary;
static int32_t* CTaskMelee_AttackersSecondary;
static int32_t CTaskMelee_PopulationPedAttackers = 3;

static HookFunction hookFunction([]()
{
	{
		auto location = hook::get_pattern("41 8B C6 2B 05 ? ? ? ? 79 31", 0x3);
		CTaskMelee_AttackersPrimary = hook::get_address<int32_t*>(location, 2, 6);
	}

	{
		auto location = hook::get_pattern("41 8B C6 2B 05 ? ? ? ? 78 09", 0x3);
		CTaskMelee_AttackersSecondary = hook::get_address<int32_t*>(location, 2, 6);
	}

	{
		// Replace the comparison that limits the number of population (scenario
		// and ambient) peds that can engage in combat.
		//
		// .text:00000001409E7B1C 41 83 FE 03  cmp  r14d, 3
		// .text:00000001409E7B20 7C 4C        jl   short loc_1409E7B6E
		static struct : jitasm::Frontend
		{
			uintptr_t m_success = 0;
			uintptr_t m_failure = 0;

			void Init(uintptr_t success, uintptr_t failure)
			{
				this->m_success = success;
				this->m_failure = failure;
			}

			virtual void InternalMain() override
			{
				mov(rax, reinterpret_cast<uintptr_t>(&CTaskMelee_PopulationPedAttackers));
				cmp(r14d, dword_ptr[rax]);
				jl("failure");

				mov(rax, m_success);
				jmp(rax);

				L("failure");
				mov(rax, m_failure);
				jmp(rax);
			}
		} combatStub;

		auto location = hook::get_pattern<char>("41 83 FE 03 7C 4C 0F B7 85");
		uintptr_t onSuccess = reinterpret_cast<uintptr_t>(location + 6);
		uintptr_t onFailure = reinterpret_cast<uintptr_t>(location + 0x4C + 6);

		combatStub.Init(onSuccess, onFailure);
		hook::nop(location, 0x6);
		hook::jump(location, combatStub.GetCode());
	}
});

static InitFunction initFunction([]()
{
	// CTaskMelee has three categories of combatant:
	//	1. Primary: engage in combat
	//	2. Secondary: engage in taunting
	//	3. Observers
	fx::ScriptEngine::RegisterNativeHandler("SET_PED_MELEE_COMBAT_LIMITS", [](fx::ScriptContext& context)
	{
		constexpr int32_t kMaxCombatants = 10;
		int32_t primary = std::clamp(context.GetArgument<int32_t>(0), 0, kMaxCombatants);
		int32_t secondary = std::clamp(context.GetArgument<int32_t>(1), 0, kMaxCombatants - primary);
		int32_t population = std::clamp(context.GetArgument<int32_t>(2), 0, kMaxCombatants);

		*CTaskMelee_AttackersPrimary = primary;
		*CTaskMelee_AttackersSecondary = secondary;
		CTaskMelee_PopulationPedAttackers = population;
	});

	// Native likely to be used in dynamic contexts: avoid connecting the same
	// function to fx::Resource::OnStop.
	OnKillNetworkDone.Connect([]()
	{
		*CTaskMelee_AttackersPrimary = 1;
		*CTaskMelee_AttackersSecondary = 3;
		CTaskMelee_PopulationPedAttackers = 3;
	});
});
