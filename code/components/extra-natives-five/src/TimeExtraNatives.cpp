#include "StdInc.h"

#include <Local.h>
#include <Hooking.h>
#include <ScriptEngine.h>
#include <nutsnbolts.h>
#include <ICoreGameInit.h>
#include <gameSkeleton.h>

static uint32_t* msPerMinute;
static uint32_t msPerMinuteDefault = 2000;

static HookFunction initFunction([]()
{
	msPerMinute = hook::get_address<uint32_t*>(hook::get_pattern("66 0F 6E 05 ? ? ? ? 0F 57 F6", 4));

	rage::OnInitFunctionEnd.Connect([](rage::InitFunctionType type)
	{
		if (type == rage::INIT_CORE)
		{
			msPerMinuteDefault = *(uint32_t*)msPerMinute;
		}
	});

	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		*(uint32_t*)msPerMinute = msPerMinuteDefault;
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_MILLISECONDS_PER_GAME_MINUTE", [=](fx::ScriptContext& context)
	{
		auto value = context.GetArgument<int>(0);

		if (value > 0)
		{
			*(uint32_t*)msPerMinute = value;
		}
	});
});
