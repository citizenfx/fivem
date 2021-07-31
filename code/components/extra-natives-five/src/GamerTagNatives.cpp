#include "StdInc.h"

#include <Hooking.h>
#include <ScriptEngine.h>
#include <ICoreGameInit.h>
#include <jitasm.h>

static int PedCurrentVehicleOffset;

static float* g_gamerTagsVisibleDistance;
static bool g_gamerTagsUseVehicleBehavior = true;

static HookFunction initFunction([]()
{
	g_gamerTagsVisibleDistance = (float*)hook::AllocateStubMemory(sizeof(float));
	*g_gamerTagsVisibleDistance = 100.0f;

	{
		// Replace the default value used for distance calculations (100.0f) with our own 
		auto location = hook::get_pattern("7C A8 45 84 FF 75", 18);
		hook::put<uint32_t>(location, (uintptr_t)g_gamerTagsVisibleDistance - (uintptr_t)location - 4);
	}

	fx::ScriptEngine::RegisterNativeHandler("SET_MP_GAMER_TAGS_VISIBLE_DISTANCE", [](fx::ScriptContext& context)
	{
		float distance = context.GetArgument<float>(0);
		*g_gamerTagsVisibleDistance = distance;
	});

	static struct : jitasm::Frontend
	{
		static void* GetPedVehicle(char* ped)
		{
			if (g_gamerTagsUseVehicleBehavior)
			{
				return *(char**)(ped + PedCurrentVehicleOffset);
			}
			else
			{
				return nullptr;
			}
		}

		virtual void InternalMain() override
		{
			push(rcx);
			sub(rsp, 0x28);

			mov(rcx, rbx);
			mov(rax, (uintptr_t)GetPedVehicle);
			call(rax);

			add(rsp, 0x28);
			pop(rcx);

			mov(rsi, rax);
			mov(rax, rbx);

			ret();
		}
	} asmfunc;

	{
		// Allow player tags to not use 'vehicle behavior', if disabled by the native.
		char* location;
		if (xbr::IsGameBuild<1604>())
		{
			location = hook::get_pattern<char>("4C 89 65 88 8B 8B", 10);
		}
		else
		{
			location = hook::get_pattern<char>("48 8B B0 ? ? ? ? 48 89 45 B0");
		}

		PedCurrentVehicleOffset = *(int*)(location + 3);
		hook::nop(location, 7);
		hook::call(location, asmfunc.GetCode());
	}

	fx::ScriptEngine::RegisterNativeHandler("SET_MP_GAMER_TAGS_USE_VEHICLE_BEHAVIOR", [](fx::ScriptContext& context)
	{
		bool enabled = context.GetArgument<bool>(0);
		g_gamerTagsUseVehicleBehavior = enabled;
	});

	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		*g_gamerTagsVisibleDistance = 100.0f;
		g_gamerTagsUseVehicleBehavior = true;
	});
});
