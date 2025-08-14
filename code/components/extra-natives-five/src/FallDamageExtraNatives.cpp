#include "StdInc.h"

#include <ScriptEngine.h>
#include <atArray.h>
#include <Local.h>
#include <Hooking.h>
#include <GameInit.h>
#include <GameValueStub.h>

static GameValueStub<float> KillFallHeight;
static GameValueStub<float> KillFallHeightForPlayers;

static GameValueStub<float> FallDamageMultiplier;
static GameValueStub<float> FallDamageMultiplierLandOnFoot;

static HookFunction initFunction([]()
{
	{
		auto location = hook::get_pattern("F3 44 0F 10 2D ? ? ? ? 48 39 BE");
		KillFallHeight.Init(*hook::get_address<float*>(location, 5, 9));
		KillFallHeight.SetLocation(location, 5, 9);
	}

	{
		auto location = hook::get_pattern("F3 44 0F 10 2D ? ? ? ? 48 39 BE", 0x12);
		KillFallHeightForPlayers.Init(*hook::get_address<float*>(location, 5, 9));
		KillFallHeightForPlayers.SetLocation(location, 5, 9);
	}


	if (xbr::IsGameBuildOrGreater<2802>())
	{
		{
			auto location = hook::get_pattern("F3 44 0F 59 25 ? ? ? ? EB");
			FallDamageMultiplierLandOnFoot.Init(*hook::get_address<float*>(location, 5, 9));
			FallDamageMultiplierLandOnFoot.SetLocation(location, 5, 9);
		}

		{
			auto location = hook::get_pattern("F3 44 0F 59 25 ? ? ? ? 45 0F 28 CC");
			FallDamageMultiplier.Init(*hook::get_address<float*>(location, 5, 9));
			FallDamageMultiplier.SetLocation(location, 5, 9);
		}
	}
	else // 1604-2699
	{
		{
			auto location = hook::get_pattern("F3 44 0F 59 1D ? ? ? ? EB ? F3 44 0F 59");
			FallDamageMultiplierLandOnFoot.Init(*hook::get_address<float*>(location, 5, 9));
			FallDamageMultiplierLandOnFoot.SetLocation(location, 5, 9);
		}

		{
			auto location = hook::get_pattern("F3 44 0F 59 1D ? ? ? ? 45 0F 28 CB");
			FallDamageMultiplier.Init(*hook::get_address<float*>(location, 5, 9));
			FallDamageMultiplier.SetLocation(location, 5, 9);
		}
	}

	OnKillNetworkDone.Connect([]()
	{
		KillFallHeight.Reset();
		KillFallHeightForPlayers.Reset();
		FallDamageMultiplierLandOnFoot.Reset();
		FallDamageMultiplier.Reset();
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_KILL_FALL_HEIGHT", [=](fx::ScriptContext& context)
	{
		context.SetResult<float>(KillFallHeight.Get());
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_KILL_FALL_HEIGHT", [=](fx::ScriptContext& context)
	{
		const float newValue = context.GetArgument<float>(0);
		KillFallHeight.Set(std::isnan(newValue) ? 0.0f : std::max(0.0f, newValue));
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_KILL_FALL_HEIGHT", [=](fx::ScriptContext& context)
	{
		context.SetResult<float>(KillFallHeightForPlayers.Get());
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_PLAYER_KILL_FALL_HEIGHT", [=](fx::ScriptContext& context)
	{
		const float newValue = context.GetArgument<float>(0);
		KillFallHeightForPlayers.Set(std::isnan(newValue) ? 0.0f : std::max(0.0f, newValue));
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_FALL_DAMAGE_MULTIPLIER", [=](fx::ScriptContext& context)
	{
		context.SetResult<float>(FallDamageMultiplier.Get());
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_FALL_DAMAGE_MULTIPLIER", [=](fx::ScriptContext& context)
	{
		const float newValue = context.GetArgument<float>(0);
		FallDamageMultiplier.Set(std::isnan(newValue) ? 0.0f : std::max(0.0f, newValue));
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_FALL_DAMAGE_LAND_ON_FOOT_MULTIPLIER", [=](fx::ScriptContext& context)
	{
		context.SetResult<float>(FallDamageMultiplierLandOnFoot.Get());
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_FALL_DAMAGE_LAND_ON_FOOT_MULTIPLIER", [=](fx::ScriptContext& context)
	{
		const float newValue = context.GetArgument<float>(0);
		FallDamageMultiplierLandOnFoot.Set(std::isnan(newValue) ? 0.0f : std::max(0.0f, newValue));
	});
});
