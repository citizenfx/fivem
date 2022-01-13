#include "StdInc.h"

#include <Hooking.h>
#include <ScriptEngine.h>
#include <ICoreGameInit.h>

#include <CrossBuildRuntime.h>

static uint32_t* msPerMinute;
static std::optional<uint32_t> customMsPerMinute;
static constexpr uint32_t msPerMinuteDefault = 2000;

static uint32_t GetDefaultMillisecondsPerGameMinute()
{
	if (customMsPerMinute)
	{
		return *customMsPerMinute;
	}

	return msPerMinuteDefault;
}

static void SetMillisecondsPerGameMinute(fx::ScriptContext& context)
{
	auto value = context.GetArgument<int>(0);

	if (value > 0)
	{
		*(uint32_t*)msPerMinute = value;
		customMsPerMinute = value;
	}
	else
	{
		*(uint32_t*)msPerMinute = msPerMinuteDefault;
		customMsPerMinute = {};
	}
}

static HookFunction hookFunction([]()
{
	msPerMinute = hook::get_address<uint32_t*>(hook::get_pattern("66 0F 6E 05 ? ? ? ? 0F 57 F6", 4));

	// in b2189 or above (where there's native support for this - NetworkOverrideClockMillisecondsPerGameMinute), this value gets reset whenever
	// NetworkOverrideClockTime gets used. this sadly occurs in obfuscated code, but the default is fetched using a call, which happens to have a lucky pattern
	if (xbr::IsGameBuildOrGreater<2189>())
	{
		// `return 2000;`, this is *only* used for this as of 2545, but that may change in newer builds
		hook::jump(hook::get_pattern("B8 D0 07 00 00 C3"), GetDefaultMillisecondsPerGameMinute);
	}
});

static InitFunction initFunction([]()
{
	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		*(uint32_t*)msPerMinute = msPerMinuteDefault;
		customMsPerMinute = {};
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_MILLISECONDS_PER_GAME_MINUTE", [](fx::ScriptContext& context)
	{
		SetMillisecondsPerGameMinute(context);
	});

	// b2189+ _NETWORK_OVERRIDE_CLOCK_MILLISECONDS_PER_GAME_MINUTE
	if (xbr::IsGameBuildOrGreater<2189>())
	{
		// the actual native works a *bit* differently from ours, but this is safe enough for our use case
		fx::ScriptEngine::RegisterNativeHandler(0x42BF1D2E723B6D7E, [](fx::ScriptContext& context)
		{
			SetMillisecondsPerGameMinute(context);
		});
	}
});
