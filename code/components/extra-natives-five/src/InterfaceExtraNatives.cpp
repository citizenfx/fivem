#include "StdInc.h"

#include <atArray.h>
#include <Local.h>
#include <Hooking.h>
#include <ScriptEngine.h>
#include <nutsnbolts.h>
#include <ICoreGameInit.h>
#include <scrEngine.h>

static hook::cdecl_stub<bool(int)> _resetHudComponentValues([]() // basically RESET_HUD_COMPONENT_VALUES
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 80 A4 ? ? ? ? ? DF"));
});

struct MapDataZoomLevel
{
	float zoomScale; // +0
	float zoomSpeed; // +4
	float scrollSpeed; // +8
	float tilesX; // +12
	float tilesY; // +16
};

struct MapZoomData
{
	atArray<MapDataZoomLevel> zoomLevels;
};

static bool* expandedRadar;
static bool* revealFullMap;

static float* pausemapPointerWorldX;
static float* pausemapPointerWorldY;

static MapZoomData* zoomData;
static std::vector<MapDataZoomLevel> defaultZoomLevels;

struct MinimapData
{
	char name[100];
	float posX;
	float posY;
	float sizeX;
	float sizeY;
	char alignX;
	char alignY;

private:
	char pad[2];
};

static MinimapData* minimapArray;
static constexpr int minimapEntries = 11;

static MinimapData minimapOldEntries[minimapEntries];

struct HudComponentData // see "frontend.xml"
{
	char unk0[8];

	int index;
	int depth;
	int listId;
	int listPriority;

	char name[40];
	char alignX[2];
	char alignY[2];

	float posX;
	float posY;
	float sizeX;
	float sizeY;

	float unk1[2];
	float unk2[2];
	float scriptPosX;
	float scriptPosY;
	float unk4[2];

	uint32_t color;
};

static std::map<uint32_t, HudComponentData> hudComponentOldEntries{};

static HudComponentData* hudComponentsArray;
static uint32_t hudComponentsCount;

static void EnsureHudComponentBackup(uint32_t index)
{
	if (index < hudComponentsCount)
	{
		if (auto& it = hudComponentOldEntries.find(index); it == hudComponentOldEntries.end())
		{
			auto component = hudComponentsArray[index];
			hudComponentOldEntries.emplace(std::make_pair(index, component));
		}
	}
}

static void RefreshHudComponent(uint32_t index)
{
	// We're calling RESET_HUD_COMPONENT_VALUES to push our updated data,
	// however this native also reset position override values that were set using
	// SET_HUD_COMPONENT_POSITION, so we have to hack around it to avoid
	// calling too much raw scaleform functions.

	if (index >= hudComponentsCount)
	{
		return;
	}

	auto& component = hudComponentsArray[index];

	float scriptX = component.scriptPosX;
	float scriptY = component.scriptPosY;
	bool scriptOverriden = (scriptX != -999.0f && scriptY != -999.0f);

	float origPosX = component.posX;
	float origPosY = component.posY;

	if (scriptOverriden)
	{
		component.posX = scriptX;
		component.posY = scriptY;
	}

	_resetHudComponentValues(index);

	if (scriptOverriden)
	{
		component.posX = origPosX;
		component.posY = origPosY;
		component.scriptPosX = scriptX;
		component.scriptPosY = scriptY;
	}
}

static void RestoreHudComponent(int index)
{
	if (auto& it = hudComponentOldEntries.find(index); it != hudComponentOldEntries.end())
	{
		auto component = &hudComponentsArray[index];
		component->alignX[0] = it->second.alignX[0];
		component->alignY[0] = it->second.alignY[0];
		component->sizeX = it->second.sizeX;
		component->sizeY = it->second.sizeY;
	}
}

static char(*g_origLoadZoomMapDataMeta)();
static char LoadZoomMapDataMeta()
{
	auto success = g_origLoadZoomMapDataMeta();

	auto size = zoomData->zoomLevels.GetSize();
	defaultZoomLevels.resize(size);

	for (int i = 0; i < size; ++i)
	{
		defaultZoomLevels[i] = zoomData->zoomLevels[i];
	}

	return success;
}

static void PatchResetHudComponentValues()
{
	static constexpr uint64_t nativeHash = 0x450930E616475D0D;

	auto handler = fx::ScriptEngine::GetNativeHandler(nativeHash);

	if (!handler)
	{
		trace("Couldn't find 0x%08x handler to hook!\n", nativeHash);
		return;
	}

	fx::ScriptEngine::RegisterNativeHandler(nativeHash, [=](fx::ScriptContext& ctx)
	{
		auto index = ctx.GetArgument<uint32_t>(0);

		RestoreHudComponent(index);

		return handler(ctx);
	});
}

static HookFunction initFunction([]()
{
	{
		auto location = hook::get_pattern<char>("33 C0 0F 57 C0 ? 0D", 0);

		expandedRadar = hook::get_address<bool*>(location + 7);
		revealFullMap = hook::get_address<bool*>(location + 37);
	}

	{
		auto location = hook::get_pattern("8B C1 48 8D 0C 80 48 8B 05 ? ? ? ? F3 0F 10 04 88", -0xB + 0x3);
		auto addr = hook::get_address<char*>((char*)location);

		zoomData = (MapZoomData*)(addr - 8);
	}

	{
		auto location = hook::get_pattern("C7 44 24 28 08 00 00 00 C7 45 DC 0D", 36);
		hook::set_call(&g_origLoadZoomMapDataMeta, location);
		hook::call(location, LoadZoomMapDataMeta);
	}

	{
		minimapArray = hook::get_address<MinimapData*>(hook::get_pattern("48 8D 54 24 38 41 B8 64 00 00 00 48 8B 48 08 48 8D 05", 18));
	}

	{
		auto location = hook::get_pattern<char>("48 83 C7 78 81 FB ? ? ? ? 7C");
		hudComponentsArray = hook::get_address<HudComponentData*>(location - 49);
		hudComponentsCount = *(uint32_t*)(location + 6);
	}

	{
		auto location = hook::get_pattern<char>("F3 0F 5C 05 ? ? ? ? 0F 28 CF");
		pausemapPointerWorldX = hook::get_address<float*>(location + 4);
		pausemapPointerWorldY = hook::get_address<float*>(location + 15);
	}

	fx::ScriptEngine::RegisterNativeHandler("GET_PAUSE_MAP_POINTER_WORLD_POSITION", [=](fx::ScriptContext& context)
	{
		scrVector pointerVector = {};

		if (pausemapPointerWorldX && pausemapPointerWorldY)
		{
			pointerVector.x = *pausemapPointerWorldX;
			pointerVector.y = *pausemapPointerWorldY;
		}
		else
		{
			pointerVector.x = 0.0f;
			pointerVector.y = 0.0f;
		}

		context.SetResult<scrVector>(pointerVector);
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_BIGMAP_ACTIVE", [=](fx::ScriptContext& context)
	{
		context.SetResult<bool>(*expandedRadar);
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_BIGMAP_FULL", [=](fx::ScriptContext& context)
	{
		context.SetResult<bool>(*revealFullMap);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_MAP_ZOOM_DATA_LEVEL", [=](fx::ScriptContext& context)
	{
		auto index = context.GetArgument<int>(0);

		if (index >= zoomData->zoomLevels.GetCount())
		{
			context.SetResult<bool>(false);
			return false;
		}

		MapDataZoomLevel* data = &(zoomData->zoomLevels[index]);
		if (data == nullptr)
		{
			context.SetResult<bool>(false);
			return false;
		}

		*context.GetArgument<float*>(1) = data->zoomScale;
		*context.GetArgument<float*>(2) = data->zoomSpeed;
		*context.GetArgument<float*>(3) = data->scrollSpeed;
		*context.GetArgument<float*>(4) = data->tilesX;
		*context.GetArgument<float*>(5) = data->tilesY;

		context.SetResult<bool>(true);
		return true;
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_MAP_ZOOM_DATA_LEVEL", [=](fx::ScriptContext& context)
	{
		auto index = context.GetArgument<int>(0);

		if (index >= zoomData->zoomLevels.GetCount())
		{
			return;
		}

		MapDataZoomLevel* data = &(zoomData->zoomLevels[index]);
		if (data == nullptr)
		{
			return;
		}

		data->zoomScale = context.GetArgument<float>(1);
		data->zoomSpeed = context.GetArgument<float>(2);
		data->scrollSpeed = context.GetArgument<float>(3);
		data->tilesX = context.GetArgument<float>(4);
		data->tilesY = context.GetArgument<float>(5);
	});

	fx::ScriptEngine::RegisterNativeHandler("RESET_MAP_ZOOM_DATA_LEVEL", [=](fx::ScriptContext& context)
	{
		auto index = context.GetArgument<int>(0);

		if (index >= zoomData->zoomLevels.GetCount())
		{
			return;
		}

		MapDataZoomLevel* data = &(zoomData->zoomLevels[index]);
		if (data == nullptr)
		{
			return;
		}

		data->zoomScale = defaultZoomLevels[index].zoomScale;
		data->zoomSpeed = defaultZoomLevels[index].zoomSpeed;
		data->scrollSpeed = defaultZoomLevels[index].scrollSpeed;
		data->tilesX = defaultZoomLevels[index].tilesX;
		data->tilesY = defaultZoomLevels[index].tilesY;
	});

	static auto minimapIsRect = hook::get_address<bool*>(hook::get_pattern("8A 15 ? ? ? ? F3 0F 10 15", 2));

	fx::ScriptEngine::RegisterNativeHandler("SET_MINIMAP_CLIP_TYPE", [](fx::ScriptContext& context)
	{
		int type = context.GetArgument<int>(0);
		
		*minimapIsRect = type == 0;
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_MINIMAP_COMPONENT_POSITION", [](fx::ScriptContext& context)
	{
		const char* name = context.CheckArgument<const char*>(0);
		const char* alignX = context.CheckArgument<const char*>(1);
		const char* alignY = context.CheckArgument<const char*>(2);

		float posX = context.GetArgument<float>(3);
		float posY = context.GetArgument<float>(4);
		float sizeX = context.GetArgument<float>(5);
		float sizeY = context.GetArgument<float>(6);

		for (int i = 0; i < minimapEntries; i++)
		{
			auto entry = &minimapArray[i];

			if (stricmp(entry->name, name) == 0)
			{
				entry->alignX = *alignX;
				entry->alignY = *alignY;
				entry->posX = posX;
				entry->posY = posY;
				entry->sizeX = sizeX;
				entry->sizeY = sizeY;

				break;
			}
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_HUD_COMPONENT_NAME", [](fx::ScriptContext& context)
	{
		char* result = "";
		auto index = context.GetArgument<uint32_t>(0);

		if (index < hudComponentsCount)
		{
			auto& component = hudComponentsArray[index];
			result = component.name;
		}

		context.SetResult<char*>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_HUD_COMPONENT_SIZE", [](fx::ScriptContext& context)
	{
		scrVector result{};
		auto index = context.GetArgument<uint32_t>(0);

		if (index < hudComponentsCount)
		{
			auto& component = hudComponentsArray[index];
			result.x = component.sizeX;
			result.y = component.sizeY;
		}

		context.SetResult<scrVector>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_HUD_COMPONENT_SIZE", [](fx::ScriptContext& context)
	{
		auto index = context.GetArgument<uint32_t>(0);
		auto sizeX = context.GetArgument<float>(1);
		auto sizeY = context.GetArgument<float>(2);

		if (index < hudComponentsCount)
		{
			EnsureHudComponentBackup(index);

			auto& component = hudComponentsArray[index];
			component.sizeX = sizeX;
			component.sizeY = sizeY;

			RefreshHudComponent(index);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_HUD_COMPONENT_ALIGN", [](fx::ScriptContext& context)
	{
		auto index = context.GetArgument<uint32_t>(0);

		if (index < hudComponentsCount)
		{
			auto& component = hudComponentsArray[index];
			*context.GetArgument<uint8_t*>(1) = component.alignX[0];
			*context.GetArgument<uint8_t*>(2) = component.alignY[0];
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_HUD_COMPONENT_ALIGN", [](fx::ScriptContext& context)
	{
		auto index = context.GetArgument<uint32_t>(0);
		auto alignX = context.CheckArgument<uint8_t>(1);
		auto alignY = context.CheckArgument<uint8_t>(2);

		if (index < hudComponentsCount)
		{
			EnsureHudComponentBackup(index);

			auto& component = hudComponentsArray[index];
			component.alignX[0] = alignX;
			component.alignY[0] = alignY;

			RefreshHudComponent(index);
		}
	});

	Instance<ICoreGameInit>::Get()->OnGameFinalizeLoad.Connect([]()
	{
		memcpy(minimapOldEntries, minimapArray, sizeof(minimapOldEntries));
	});

	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		for (int i = 0; i < zoomData->zoomLevels.GetSize(); ++i)
		{
			zoomData->zoomLevels[i] = defaultZoomLevels[i];
		}

		memcpy(minimapArray, minimapOldEntries, sizeof(minimapOldEntries));

		for (auto& entry : hudComponentOldEntries)
		{
			RestoreHudComponent(entry.first);
			_resetHudComponentValues(entry.first);
		}

		hudComponentOldEntries.clear();

		*minimapIsRect = true;
	});

	rage::scrEngine::OnScriptInit.Connect([]()
	{
		// Patch RESET_HUD_COMPONENT_VALUES to support SIZE and ALIGN overrides.
		PatchResetHudComponentValues();
	});
});
