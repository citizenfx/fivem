#include "StdInc.h"

#include <atArray.h>
#include <Local.h>
#include <Hooking.h>
#include <ScriptEngine.h>
#include <nutsnbolts.h>
#include <ICoreGameInit.h>

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

static uint64_t* expandedRadar;
static uint64_t* revealFullMap;

static MapZoomData* zoomData;
static std::vector<MapDataZoomLevel> defaultZoomLevels;

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

static HookFunction initFunction([]()
{
	{
		auto location = hook::get_pattern("33 C0 0F 57 C0 ? 0D", 0);

		expandedRadar = hook::get_address<uint64_t*>((char*)location + 7);
		revealFullMap = hook::get_address<uint64_t*>((char*)location + 37);
	}

	{
		auto location = hook::get_pattern("E8 ? ? ? ? 0F 2F F8 0F 86 D4 00 00 00 0F B7 0D", 17);
		auto addr = hook::get_address<char*>((char*)location);

		zoomData = (MapZoomData*)(addr - 8);
	}

	{
		auto location = hook::get_pattern("C7 44 24 28 08 00 00 00 C7 45 DC 0D", 36);
		hook::set_call(&g_origLoadZoomMapDataMeta, location);
		hook::call(location, LoadZoomMapDataMeta);
	}

	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		for (int i = 0; i < zoomData->zoomLevels.GetSize(); ++i)
		{
			zoomData->zoomLevels[i] = defaultZoomLevels[i];
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_BIGMAP_ACTIVE", [=](fx::ScriptContext& context)
	{
		auto result = *(uint8_t*)expandedRadar == 1;
		context.SetResult<bool>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_BIGMAP_FULL", [=](fx::ScriptContext& context)
	{
		auto result = *(uint8_t*)revealFullMap == 1;
		context.SetResult<bool>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_MAP_ZOOM_DATA_LEVEL", [=](fx::ScriptContext& context)
	{
		auto index = context.GetArgument<int>(0);

		if (index > zoomData->zoomLevels.GetCount())
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

		if (index > zoomData->zoomLevels.GetCount())
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

		if (index > zoomData->zoomLevels.GetCount())
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
});
