#include "StdInc.h"

#include <ICoreGameInit.h>
#include <Hooking.h>

#include <ScriptEngine.h>

#include "GameValueStub.h"

static GameValueStub<float> g_vfxNitrousOverride;

static HookFunction hookFunction([]()
{
	{
		auto location = hook::get_pattern("0F 28 82 ? ? ? ? 48 8D 4D C7 0F 29 45 C7", 0x14 + 0x3);
		g_vfxNitrousOverride.Init(*hook::get_address<float*>(location));
		g_vfxNitrousOverride.SetLocation(location);
	}

	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		g_vfxNitrousOverride.Reset();
	});
});

static InitFunction initFunction([]()
{
	// GH-2003: Increase the maximum range for displaying veh_nitrous PTFX
	fx::ScriptEngine::RegisterNativeHandler("SET_VEHICLE_NITRO_PTFX_RANGE", [](fx::ScriptContext& context)
	{
		constexpr float kMaxPtfxSqrDist = 212.0f * 212.0f; // Half of OneSync maxRange

		float dist = context.GetArgument<float>(0);
		float sqrDist = dist * dist;
		if (std::isfinite(sqrDist))
		{
			float minSqrDist = g_vfxNitrousOverride.GetDefault();
			g_vfxNitrousOverride.Set(std::clamp(sqrDist, minSqrDist, kMaxPtfxSqrDist));
		}
	});
});
