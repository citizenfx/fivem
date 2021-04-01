#include <StdInc.h>
#include <Hooking.h>

#include <ScriptEngine.h>

#include <Resource.h>
#include <fxScripting.h>

#include <GameInit.h>

static float* g_pedDensity;
static float* g_pedScenarioDensity;
static float* g_ambientPedRange;
static float* g_ambientVehicleRangeMultiplier;
static float* g_parkedVehicleDensity;
static float* g_vehicleDensity;

static char* g_pedLocation;
static char* g_vehicleLocation;

static HookFunction hookFunction([]()
{
	{
		auto location = hook::get_pattern<char>("8B 05 ? ? ? ? 83 25 ? ? ? ? 00 80 3D ? ? ? ? 00");

		g_pedDensity = hook::get_address<float*>(location + 35) + 1;
		g_pedScenarioDensity = hook::get_address<float*>(location + 45) + 1;
		g_ambientPedRange = hook::get_address<float*>(location + 65) + 1;

		g_pedLocation = location;
	}

	{
		auto location = hook::get_pattern<char>("33 C0 0F 57 C0 C7 05");

		g_ambientVehicleRangeMultiplier = hook::get_address<float*>(location + 7) + 1;
		g_vehicleDensity = hook::get_address<float*>(location + 17) + 1;
		g_parkedVehicleDensity = hook::get_address<float*>(location + 27) + 1;

		g_vehicleLocation = location;
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

	fx::ScriptEngine::RegisterNativeHandler("SET_DEFAULT_PED_DENSITY_MULTIPLIER", [](fx::ScriptContext& context)
	{
		float multiplier = context.GetArgument<float>(0);
		hook::put<float>(g_pedLocation + 39, multiplier);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_DEFAULT_SCENARIO_PED_DENSITY_MULTIPLIER", [](fx::ScriptContext& context)
	{
		float multiplier = context.GetArgument<float>(0);
		hook::put<float>(g_pedLocation + 49, multiplier);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_DEFAULT_AMBIENT_PED_RANGE_MULTIPLIER", [](fx::ScriptContext& context)
	{
		float multiplier = context.GetArgument<float>(0);
		hook::put<float>(g_pedLocation + 69, multiplier);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_DEFAULT_AMBIENT_VEHICLE_RANGE_MULTIPLIER", [](fx::ScriptContext& context)
	{
		float multiplier = context.GetArgument<float>(0);
		hook::put<float>(g_vehicleLocation + 11, multiplier);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_DEFAULT_VEHICLE_DENSITY_MULTIPLIER", [](fx::ScriptContext& context)
	{
		float multiplier = context.GetArgument<float>(0);
		hook::put<float>(g_vehicleLocation + 21, multiplier);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_DEFAULT_PARKED_VEHICLE_DENSITY_MULTIPLIER", [](fx::ScriptContext& context)
	{
		float multiplier = context.GetArgument<float>(0);
		hook::put<float>(g_vehicleLocation + 31, multiplier);
	});

	OnKillNetworkDone.Connect([]
	{
		// Ped Density, Scenario Ped Density, Ped Ambient Range 
		hook::put<float>(g_pedLocation + 39, 1.0f);
		hook::put<float>(g_pedLocation + 49, 1.0f);
		hook::put<float>(g_pedLocation + 69, 1.0f);

		// Vehicle Ambient Range, Vehicle Density, Parked Vehicle Density
		hook::put<float>(g_vehicleLocation + 11, 1.0f);
		hook::put<float>(g_vehicleLocation + 21, 1.0f);
		hook::put<float>(g_vehicleLocation + 31, 1.0f);
	});
});
