#include <StdInc.h>
#include <Hooking.h>

#include <ScriptEngine.h>

#include <Resource.h>
#include <fxScripting.h>

#include "CrossBuildRuntime.h"

static bool g_forceSnowPass;
static bool* g_weatherNetFlag;

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
});

static uint32_t(*getWeatherByName)(void*, const char*);
static int* g_currentWeather;

static uint32_t getWeatherByNameHook(void* a1, const char* a2)
{
	if (g_forceSnowPass)
	{
		return *g_currentWeather;
	}

	return getWeatherByName(a1, a2);
}

static HookFunction hookFunction([]()
{
	{
		auto location = (xbr::IsGameBuildOrGreater<3095>()) ? hook::get_pattern<char>("44 88 7D 30 E8 ? ? ? ? 48 8D 15") : hook::get_pattern<char>("C6 45 20 00 E8 ? ? ? ? 48 8D 15");

		hook::set_call(&getWeatherByName, location + 23);
		hook::call(location + 23, getWeatherByNameHook);

		g_currentWeather = hook::get_address<int*>(location + 30);
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
