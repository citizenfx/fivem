#include <StdInc.h>
#include <Hooking.h>

#include <ScriptEngine.h>

#include <Resource.h>
#include <fxScripting.h>

#include "CrossBuildRuntime.h"

static int32_t (*g_weatherGetTypeIndex)(void*, const char*);

struct CWeatherType
{
	uint32_t hashName;
	char pad4[0xC0];
};

template <int Build>
struct CWeather
{
	uint32_t numTypes;
	CWeatherType types[(Build >= 3258) ? 32 : 16];
	uint32_t numCycleEntries;
	uint8_t cycle[256];
	uint8_t cycleTimeMult[256];
	uint32_t msPerCycle;
	uint32_t currentMsPerCycle;
	uint32_t cycleTimer;
	uint32_t networkInitNetworkTime;
	uint32_t networkInitCycleTime;
	uint32_t lastSysTime;
	uint32_t systemTime;
	float changeCloudOnSameCloudType;
	float changeCloundOnSameWeatherType;
	uint32_t prevTypeIndex;
	uint32_t nextTypeIndex;
	// and a bunch of other fields...

	int32_t GetTypeIndex(const char* name)
	{
		return g_weatherGetTypeIndex(this, name);
	}
};

template <int Build>
static CWeather<Build>* g_weather;

static bool g_forceSnowPass;
static bool* g_weatherNetFlag;

static uint8_t g_customWeatherCycles[256];
static uint8_t g_customWeatherCycleTimeMults[256];

template<typename T>
static void AddStopCallback(T&& stopCb)
{
	fx::OMPtr<IScriptRuntime> runtime;

	if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
	{
		fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

		resource->OnStop.Connect(stopCb);
	}
}

template <int Build>
static void RegisterExtraWeatherNatives()
{
	fx::ScriptEngine::RegisterNativeHandler("SET_WEATHER_CYCLE_ENTRY", [](fx::ScriptContext& context)
	{
		int index = context.GetArgument<int>(0);
		const char* weatherName = context.CheckArgument<const char*>(1);
		int timeMult = context.GetArgument<int>(2);

		auto& weather = *g_weather<Build>;

		int32_t typeIndex = weather.GetTypeIndex(weatherName);

		if (index < 0 || index > 255 || typeIndex < 0 || typeIndex >= weather.numTypes || timeMult < 1 || timeMult > 255)
		{
			return context.SetResult(false);
		}

		g_customWeatherCycles[index] = (uint8_t)typeIndex;
		g_customWeatherCycleTimeMults[index] = (uint8_t)timeMult;

		return context.SetResult(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("APPLY_WEATHER_CYCLES", [](fx::ScriptContext& context)
	{
		int numCycleEntries = context.GetArgument<int>(0);
		int msPerCycle = context.GetArgument<int>(1);

		auto& weather = *g_weather<Build>;

		if (numCycleEntries < 1 || numCycleEntries > 255 || msPerCycle < 1000 || msPerCycle > 86400000)
		{
			return context.SetResult(false);
		}

		int systemTime = 0;

		for (int i = 0; i < numCycleEntries; i++)
		{
			uint8_t cycleType = g_customWeatherCycles[i];
			uint8_t cycleTimeMult = g_customWeatherCycleTimeMults[i];

			if (cycleType >= weather.numTypes || cycleTimeMult < 1)
			{
				return context.SetResult(false);
			}

			systemTime += msPerCycle * cycleTimeMult;
		}

		weather.numCycleEntries = numCycleEntries;
		weather.msPerCycle = msPerCycle;
		weather.systemTime = systemTime;

		for (int i = 0; i < numCycleEntries; i++)
		{
			weather.cycle[i] = g_customWeatherCycles[i];
			weather.cycleTimeMult[i] = g_customWeatherCycleTimeMults[i];
		}

		return context.SetResult(true);
	});
}

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("FORCE_SNOW_PASS", [](fx::ScriptContext& context)
	{
		bool value = context.GetArgument<bool>(0);
		g_forceSnowPass = value;

		AddStopCallback([]()
		{
			g_forceSnowPass = false;
		});
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_WEATHER_OWNED_BY_NETWORK", [](fx::ScriptContext& context)
	{
		*g_weatherNetFlag = context.GetArgument<bool>(0);

		AddStopCallback([]()
		{
			*g_weatherNetFlag = true;
		});
	});

	if (xbr::IsGameBuildOrGreater<3258>())
	{
		RegisterExtraWeatherNatives<3258>();
	}
	else
	{
		RegisterExtraWeatherNatives<0>();
	}
});

template <int Build>
static uint32_t weatherGetTypeIndexSnowHook(void* a1, const char* a2)
{
	if (g_forceSnowPass)
	{
		return g_weather<Build>->prevTypeIndex;
	}

	return g_weatherGetTypeIndex(a1, a2);
}

static HookFunction hookFunction([]()
{
	{
		auto location1 = hook::get_pattern<char>("48 8D 35 ? ? ? ? 3B C8");
		auto location2 = (xbr::IsGameBuildOrGreater<3095>()) ? hook::get_pattern<char>("44 88 7D 30 E8 ? ? ? ? 48 8D 15") : hook::get_pattern<char>("C6 45 20 00 E8 ? ? ? ? 48 8D 15");

		hook::set_call(&g_weatherGetTypeIndex, location2 + 23);

		if (xbr::IsGameBuildOrGreater<3258>())
		{
			g_weather<3258> = hook::get_address<CWeather<3258>*>(location1 + 3);
			hook::call(location2 + 23, weatherGetTypeIndexSnowHook<3258>);
		}
		else
		{
			g_weather<0> = hook::get_address<CWeather<0>*>(location1 + 3);
			hook::call(location2 + 23, weatherGetTypeIndexSnowHook<0>);
		}
	}

	{
		auto location = hook::get_call(hook::get_pattern<char>("E8 ? ? ? ? 84 C0 75 4F EB 3C"));

		// in 2060+ this is obfuscated a bit
		if (xbr::IsGameBuildOrGreater<2060>())
		{
			g_weatherNetFlag = hook::get_address<bool*>(hook::get_call(location) + 2);
		}
		else
		{
			g_weatherNetFlag = hook::get_address<bool*>(location + 2);
		}
	}
});
