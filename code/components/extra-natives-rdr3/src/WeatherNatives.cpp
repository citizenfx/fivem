#include <StdInc.h>
#include <Hooking.h>

#include <ScriptEngine.h>
#include <CrossBuildRuntime.h>

#include <Resource.h>
#include <fxScripting.h>

// IsApplyingMultiplayerGlobalClockAndWeather
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
	fx::ScriptEngine::RegisterNativeHandler("SET_WEATHER_OWNED_BY_NETWORK", [](fx::ScriptContext& context)
	{
		*g_weatherNetFlag = context.GetArgument<bool>(0);

		AddStopCallback([]()
		{
			*g_weatherNetFlag = true;
		});
	});
});

static HookFunction hookFunction([]()
{
	{
		auto location = hook::get_call(hook::get_pattern<char>("0F 29 7C 24 40 44 0F 29 4C 24 30 0F 84", 17));
		g_weatherNetFlag = hook::get_address<bool*>(location + 2);
	}
});
