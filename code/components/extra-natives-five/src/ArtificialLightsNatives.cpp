#include "StdInc.h"
#include <ScriptEngine.h>
#include <scrEngine.h>
#include <EntitySystem.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <Hooking.Patterns.h>
#include <ScriptSerialization.h>
#include <MinHook.h>
#include <GameInit.h>
#include <unordered_set>
#include <shared_mutex>

// These natives allow controlling which specific entities ignore the artificial lights
// "blackout" mode. When SET_ARTIFICIAL_LIGHTS_STATE(true) is called (blackout enabled),
// entities marked with SET_ENTITY_LIGHTS_IGNORE_ARTIFICIAL_STATE will keep their lights on.
//
// Implementation hooks Lights::AddSceneLight which is called for all entity lights.
// CLightEntity has m_parentEntity at offset 0xD0 which points to the actual entity.
//

// Set of raw entity pointers that should ignore artificial lights state
static std::unordered_set<void*> g_entitiesIgnoringBlackout;
static std::shared_mutex g_entitiesMutex;

// globals
static bool* g_disableArtificialLights = nullptr;
static bool* g_disableArtificialVehLights = nullptr;

// CLightEntity::m_parentEntity offset
constexpr ptrdiff_t LIGHT_ENTITY_PARENT_OFFSET = 0xD0;

// original function
static bool (*g_origAddSceneLight)(void* sceneLight, void* lightEntity, bool addToPreviousLightList);

static bool DoesEntityIgnoreBlackout(void* entity)
{
	if (!entity)
		return false;
	std::shared_lock lock(g_entitiesMutex);
	return g_entitiesIgnoringBlackout.find(entity) != g_entitiesIgnoringBlackout.end();
}

static void* GetParentEntityFromLightEntity(void* lightEntity)
{
	if (!lightEntity)
		return nullptr;

	// m_parentEntity is at offset 0xD0, it's a rage::fwRegdRef<CEntity>
	// The actual pointer is at the start of the fwRegdRef
	void** parentPtr = (void**)((char*)lightEntity + LIGHT_ENTITY_PARENT_OFFSET);
	return *parentPtr;
}

static bool AddSceneLightHook(void* sceneLight, void* lightEntity, bool addToPreviousLightList)
{
	void* parentEntity = GetParentEntityFromLightEntity(lightEntity);
	bool shouldOverride = DoesEntityIgnoreBlackout(parentEntity);

	bool origLightsState = false;
	bool origVehLightsState = false;

	if (shouldOverride && g_disableArtificialLights && g_disableArtificialVehLights)
	{
		// save original states
		origLightsState = *g_disableArtificialLights;
		origVehLightsState = *g_disableArtificialVehLights;

		// temporarily disable blackout for this entitys lights
		*g_disableArtificialLights = false;
		*g_disableArtificialVehLights = false;
	}

	// call original function
	bool result = g_origAddSceneLight(sceneLight, lightEntity, addToPreviousLightList);

	if (shouldOverride && g_disableArtificialLights && g_disableArtificialVehLights)
	{
		// restore original states
		*g_disableArtificialLights = origLightsState;
		*g_disableArtificialVehLights = origVehLightsState;
	}

	return result;
}

static HookFunction hookFunction([]()
{
	{
		auto location = hook::get_pattern<char>("48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 4C 89 60 ? 55 41 56 41 57 48 8D 68 ? 48 81 EC ? ? ? ? 0F 29 70 ? 45 33 E4");

		// Find CRenderer::sm_disableArtificialLights and CRenderer::sm_disableArtificialVehLights
		// Both are referenced within AddSceneLight using "cmp [rip+offset], r12b" pattern: "44 38 25 ? ? ? ?"
		// Search within the first 0x80 bytes of the function
		{
			hook::range_pattern p((uintptr_t)location, (uintptr_t)location + 0x80, "44 38 25");

			auto firstMatch = p.get(0).get<char>(0);
			g_disableArtificialLights = hook::get_address<bool*>(firstMatch + 3);

			auto secondMatch = p.get(1).get<char>(0);
			g_disableArtificialVehLights = hook::get_address<bool*>(secondMatch + 3);
		}

		MH_Initialize();
		MH_CreateHook(location, AddSceneLightHook, (void**)&g_origAddSceneLight);
		MH_EnableHook(location);
	}

	fx::ScriptEngine::RegisterNativeHandler("SET_ENTITY_LIGHTS_IGNORE_ARTIFICIAL_STATE", [](fx::ScriptContext& ctx)
	{
		int entityHandle = ctx.GetArgument<int>(0);
		bool ignoreBlackout = ctx.GetArgument<bool>(1);

		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(entityHandle);
		if (!entity)
		{
			return;
		}

		std::unique_lock lock(g_entitiesMutex);
		if (ignoreBlackout)
		{
			g_entitiesIgnoringBlackout.insert(entity);
		}
		else
		{
			g_entitiesIgnoringBlackout.erase(entity);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("DOES_ENTITY_LIGHTS_IGNORE_ARTIFICIAL_STATE", [](fx::ScriptContext& ctx)
	{
		int entityHandle = ctx.GetArgument<int>(0);

		fwEntity* entity = rage::fwScriptGuid::GetBaseFromGuid(entityHandle);
		if (!entity)
		{
			ctx.SetResult<bool>(false);
			return;
		}

		ctx.SetResult<bool>(DoesEntityIgnoreBlackout(entity));
	});

	fx::ScriptEngine::RegisterNativeHandler("CLEAR_ALL_ENTITY_LIGHTS_IGNORE_ARTIFICIAL_STATE", [](fx::ScriptContext& ctx)
	{
		std::unique_lock lock(g_entitiesMutex);
		g_entitiesIgnoringBlackout.clear();
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_ALL_ENTITIES_IGNORING_ARTIFICIAL_LIGHTS_STATE", [](fx::ScriptContext& ctx)
	{
		std::vector<int> entityList;

		std::shared_lock lock(g_entitiesMutex);
		for (void* entityPtr : g_entitiesIgnoringBlackout)
		{
			int handle = rage::fwScriptGuid::GetGuidFromBase(reinterpret_cast<fwEntity*>(entityPtr));
			if (handle != 0)
			{
				entityList.push_back(handle);
			}
		}

		ctx.SetResult(fx::SerializeObject(entityList));
	});

	OnKillNetworkDone.Connect([]()
	{
		std::unique_lock lock(g_entitiesMutex);
		g_entitiesIgnoringBlackout.clear();
	});
});
