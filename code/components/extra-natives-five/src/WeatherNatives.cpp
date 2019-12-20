#include <StdInc.h>
#include <Hooking.h>

#include <ScriptEngine.h>

#include <Resource.h>
#include <fxScripting.h>

static bool g_forceSnowPass;

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("FORCE_SNOW_PASS", [](fx::ScriptContext& context)
	{
		bool value = context.GetArgument<bool>(0);
		g_forceSnowPass = value;

		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			resource->OnStop.Connect([]()
			{
				g_forceSnowPass = false;
			});
		}
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
	auto location = hook::get_pattern<char>("C6 45 20 00 E8 ? ? ? ? 48 8D 15");
	hook::set_call(&getWeatherByName, location + 23);
	hook::call(location + 23, getWeatherByNameHook);

	g_currentWeather = hook::get_address<int*>(location + 30);
});
