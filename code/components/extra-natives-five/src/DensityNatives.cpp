#include <StdInc.h>
#include <Hooking.h>

#include <ScriptEngine.h>

#include <Resource.h>
#include <fxScripting.h>

static float* g_pedDensity;
static float* g_pedScenarioDensity;
static float* g_ambientPedRange;
static float* g_ambientVehicleRangeMultiplier;
static float* g_parkedVehicleDensity;
static float* g_vehicleDensity;

static HookFunction hookFunction([]()
{
	{
		auto location = hook::get_pattern<char>("8B 05 ? ? ? ? 83 25 ? ? ? ? 00 80 3D ? ? ? ? 00");

		g_pedDensity = hook::get_address<float*>(location + 35) + 1;
		g_pedScenarioDensity = hook::get_address<float*>(location + 45) + 1;
		g_ambientPedRange = hook::get_address<float*>(location + 65) + 1;
	}

	{
		auto location = hook::get_pattern<char>("33 C0 0F 57 C0 C7 05");

		g_ambientVehicleRangeMultiplier = hook::get_address<float*>(location + 7) + 1;
		g_vehicleDensity = hook::get_address<float*>(location + 17) + 1;
		g_parkedVehicleDensity = hook::get_address<float*>(location + 27) + 1;
	}

	fx::ScriptEngine::RegisterNativeHandler("GET_PED_DENSITY_MULTIPLIER", [](fx::ScriptContext& context)
	{
		context.SetResult<float>(*g_pedDensity);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_SCENARIO_PED_DENSITY_MULTIPLIER", [](fx::ScriptContext& context)
	{
		context.SetResult<float>(*g_pedScenarioDensity);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_AMBIENT_PED_RANGE_MULTIPLIER", [](fx::ScriptContext& context)
	{
		context.SetResult<float>(*g_ambientPedRange);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_AMBIENT_VEHICLE_RANGE_MULTIPLIER", [](fx::ScriptContext& context)
	{
		context.SetResult<float>(*g_ambientVehicleRangeMultiplier);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PARKED_VEHICLE_DENSITY_MULTIPLIER", [](fx::ScriptContext& context)
	{
		context.SetResult<float>(*g_parkedVehicleDensity);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_VEHICLE_DENSITY_MULTIPLIER", [](fx::ScriptContext& context)
	{
		context.SetResult<float>(*g_vehicleDensity);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_RANDOM_VEHICLE_DENSITY_MULTIPLIER", [](fx::ScriptContext& context)
	{
		context.SetResult<float>(*g_vehicleDensity);
	});
});
