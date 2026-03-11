#include "StdInc.h"

#include <ScriptEngine.h>
#include <Hooking.h>
#include <GameInit.h>
#include <GameValueStub.h>

static GameValueStub<float> MediumRange;

static HookFunction initFunction([]()
{
	{
		auto location = hook::get_pattern("F3 0F 10 3D ? ? ? ? 48 8D 55 ? 48 8D 0D", 0x04);
		MediumRange.Init(*hook::get_address<float*>(location));
		MediumRange.SetLocation(location);
	}

	OnKillNetworkDone.Connect([]()
	{
		MediumRange.Reset();
	});

	fx::ScriptEngine::RegisterNativeHandler("DISABLE_INFLATED_MAP_DATA_RANGE", [=](fx::ScriptContext& context)
	{
		const bool disabled = context.GetArgument<bool>(0);
		if (disabled)
		{
			MediumRange.Set(0.0f);
		}
		else
		{
			MediumRange.Reset();
		}
	});
});
