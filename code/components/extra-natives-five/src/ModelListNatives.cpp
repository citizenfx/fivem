#include <StdInc.h>
#include <Hooking.h>

#include <ScriptEngine.h>
#include <ScriptSerialization.h>

static uint32_t(*initVehicleArchetype)(const char* name, bool a2, unsigned int a3);

static void(*g_origArchetypeDtor)(fwArchetype* at);

// Use a map of hashes and names for optimization purposes
// We just hash it on registration instead of needing to rehash it when checking for it every deregistration
static std::unordered_map<uint32_t, std::string> g_vehicles;

static uint32_t initVehicleArchetype_stub(const char* name, bool a2, unsigned int a3)
{
	g_vehicles.insert({ HashString(name), name });
	return initVehicleArchetype(name, a2, a3);
}

static void ArchetypeDtorHook(fwArchetype* at)
{
	g_vehicles.erase(at->hash);
	g_origArchetypeDtor(at);
}

static HookFunction hookFunction([]()
{
	//hook for vehicle registering
	{
		auto location = hook::get_pattern<char>("E8 ? ? ? ? 48 8B 4D E0 48 8B 11");
		hook::set_call(&initVehicleArchetype, location);
		hook::call(location, initVehicleArchetype_stub);
	}
	// hook to catch deregistration of vehicles
	{
		auto location = hook::get_pattern("E8 ? ? ? ? 80 7B 60 01 74 39"); // Taken from StreamingFreeTests.cpp
		hook::set_call(&g_origArchetypeDtor, location);
		hook::call(location, ArchetypeDtorHook);
	}
	fx::ScriptEngine::RegisterNativeHandler("GET_ALL_VEHICLE_MODELS", [](fx::ScriptContext& context)
	{
		// its redundant and possibly confusing to give both hashes and names, so just give the names.
		std::vector<std::string_view> models;
		for (const auto& pair : g_vehicles)
		{
			models.push_back(pair.second);
		}
		context.SetResult(fx::SerializeObject(models));
	});
});
