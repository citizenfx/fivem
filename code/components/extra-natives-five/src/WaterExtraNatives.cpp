#include <StdInc.h>

#include <Hooking.h>
#include <Resource.h>
#include <atArray.h>
#include <bitset>

#include <ScriptEngine.h>
#include <fxScripting.h>
#include <ICoreGameInit.h>

#include <ResourceManager.h>
#include <ResourceMetaDataComponent.h>
#include <VFSManager.h>

struct WaterQuad
{
	int16_t minX; // +0
	int16_t minY; // +2
	int16_t maxX; // +4
	int16_t maxY; // +6
	uint8_t a[4]; // +8
	float target; // +12
	float angle;  // +16
	float height; // +20
	uint8_t flags; // +24
	uint8_t type; // +25
	uint16_t unk; // +26
}; // 28

struct CalmingQuad
{
	int16_t minX; // +0
	int16_t minY; // +2
	int16_t maxX; // +4
	int16_t maxY; // +6
	float dampening; // +8
}; // 12

struct WaveQuad
{
	int16_t minX; // +0
	int16_t minY; // +2
	int16_t maxX; // +4
	int16_t maxY; // +6
	int16_t amplitude; // +8
	int8_t directionX; // +10
	int8_t directionY; // +11
}; // 12

static atArray<WaterQuad>* g_waterQuads;
static atArray<CalmingQuad>* g_calmingQuads;
static atArray<WaveQuad>* g_waveQuads;

static hook::cdecl_stub<void(const char* path)> _loadWater([]()
{
	return hook::get_pattern("48 85 C9 75 ? 8D 51 ? 48 8B 0D", -22);
});

static hook::cdecl_stub<void()> _unloadWater([]()
{
	return hook::get_call(hook::get_pattern("BE 02 00 00 00 3B CE 75", 9));
});

static WaterQuad* GetWaterQuadByIndex(int index)
{
	if (index < 0 || index >= g_waterQuads->GetCount())
	{
		return nullptr;
	}

	return &(g_waterQuads->Get(index));
}

static CalmingQuad* GetCalmingQuadByIndex(int index)
{
	if (index < 0 || index >= g_calmingQuads->GetCount())
	{
		return nullptr;
	}

	return &(g_calmingQuads->Get(index));
}

static WaveQuad* GetWaveQuadByIndex(int index)
{
	if (index < 0 || index >= g_waveQuads->GetCount())
	{
		return nullptr;
	}

	return &(g_waveQuads->Get(index));
}

template<unsigned char bit>
static void WriteWaterQuadFlag(fx::ScriptContext& context)
{
	if (context.GetArgumentCount() < 2)
	{
		return;
	}

	if (WaterQuad* waterQuad = GetWaterQuadByIndex(context.GetArgument<int>(0)))
	{
		auto value = (context.GetArgument<bool>(1) != 0);
		waterQuad->flags = (waterQuad->flags & ~(1 << bit)) | (value << bit);
	}
}

template<int bit>
static void ReadWaterQuadFlag(fx::ScriptContext& context)
{
	if (WaterQuad* waterQuad = GetWaterQuadByIndex(context.GetArgument<int>(0)))
	{
		auto value = (waterQuad->flags & (1 << bit)) != 0;
		context.SetResult<bool>(value);
	}
}

static HookFunction hookFunction([]()
{
	static_assert(sizeof(WaterQuad) == 28, "Wrong WaveQuad struct size");
	static_assert(sizeof(CalmingQuad) == 12, "Wrong CalmingQuad struct size");
	static_assert(sizeof(WaveQuad) == 12, "Wrong WaveQuad struct size");

	{
		auto location = hook::get_pattern<char>("83 F9 04 75 ? 48 8B 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D");
		g_waterQuads = hook::get_address<atArray<WaterQuad>*>(location + 8);
		g_calmingQuads = hook::get_address<atArray<CalmingQuad>*>(location + 20);
		g_waveQuads = hook::get_address<atArray<WaveQuad>*>(location + 45);
	}

	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		_unloadWater();
		_loadWater(nullptr);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_WATER_QUAD_BOUNDS", [](fx::ScriptContext& context)
	{
		if (auto waterQuad = GetWaterQuadByIndex(context.GetArgument<int>(0)))
		{
			waterQuad->minX = context.GetArgument<int16_t>(1);
			waterQuad->minY = context.GetArgument<int16_t>(2);
			waterQuad->maxX = context.GetArgument<int16_t>(3);
			waterQuad->maxY = context.GetArgument<int16_t>(4);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WATER_QUAD_BOUNDS", [](fx::ScriptContext& context)
	{
		int minX = 0,
			minY = 0,
			maxX = 0,
			maxY = 0;

		if (auto waterQuad = GetWaterQuadByIndex(context.GetArgument<int>(0)))
		{
			minX = waterQuad->minX;
			minY = waterQuad->minY;
			maxX = waterQuad->maxX;
			maxY = waterQuad->maxY;
		}

		*context.GetArgument<int*>(1) = minX;
		*context.GetArgument<int*>(2) = minY;
		*context.GetArgument<int*>(3) = maxX;
		*context.GetArgument<int*>(4) = maxY;
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_WATER_QUAD_HEIGHT", [](fx::ScriptContext& context)
	{
		if (auto waterQuad = GetWaterQuadByIndex(context.GetArgument<int>(0)))
		{
			waterQuad->height = context.GetArgument<float>(1);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WATER_QUAD_HEIGHT", [](fx::ScriptContext& context)
	{
		auto result = 0.0f;

		if (auto waterQuad = GetWaterQuadByIndex(context.GetArgument<int>(0)))
		{
			result = waterQuad->height;
		}

		context.SetResult<float>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_WATER_QUAD_TYPE", [](fx::ScriptContext& context)
	{
		if (auto waterQuad = GetWaterQuadByIndex(context.GetArgument<int>(0)))
		{
			waterQuad->type = context.GetArgument<int>(1);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WATER_QUAD_TYPE", [](fx::ScriptContext& context)
	{
		auto result = 0;

		if (auto waterQuad = GetWaterQuadByIndex(context.GetArgument<int>(0)))
		{
			result = waterQuad->type;
		}

		context.SetResult<int>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_WATER_QUAD_HAS_LIMITED_DEPTH", WriteWaterQuadFlag<1>);
	fx::ScriptEngine::RegisterNativeHandler("IS_WATER_QUAD_HAS_LIMITED_DEPTH", ReadWaterQuadFlag<1>);

	fx::ScriptEngine::RegisterNativeHandler("SET_WATER_QUAD_INVISIBLE", WriteWaterQuadFlag<3>);
	fx::ScriptEngine::RegisterNativeHandler("IS_WATER_QUAD_INVISIBLE", ReadWaterQuadFlag<3>);

	fx::ScriptEngine::RegisterNativeHandler("SET_WATER_QUAD_NO_STENCIL", WriteWaterQuadFlag<4>);
	fx::ScriptEngine::RegisterNativeHandler("IS_WATER_QUAD_NO_STENCIL", ReadWaterQuadFlag<4>);

	fx::ScriptEngine::RegisterNativeHandler("SET_WATER_QUAD_ALPHA", [](fx::ScriptContext& context)
	{
		if (auto waterQuad = GetWaterQuadByIndex(context.GetArgument<int>(0)))
		{
			waterQuad->a[0] = context.GetArgument<int>(1);
			waterQuad->a[1] = context.GetArgument<int>(2);
			waterQuad->a[2] = context.GetArgument<int>(3);
			waterQuad->a[3] = context.GetArgument<int>(4);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WATER_QUAD_ALPHA", [](fx::ScriptContext& context)
	{
		int a0 = 0,
			a1 = 0,
			a2 = 0,
			a3 = 0;

		if (auto waterQuad = GetWaterQuadByIndex(context.GetArgument<int>(0)))
		{
			a0 = waterQuad->a[0];
			a1 = waterQuad->a[1];
			a2 = waterQuad->a[2];
			a3 = waterQuad->a[3];
		}

		*context.GetArgument<int*>(1) = a0;
		*context.GetArgument<int*>(2) = a1;
		*context.GetArgument<int*>(3) = a2;
		*context.GetArgument<int*>(4) = a3;
	});


	fx::ScriptEngine::RegisterNativeHandler("SET_CALMING_QUAD_BOUNDS", [](fx::ScriptContext& context)
	{
		if (auto calmingQuad = GetCalmingQuadByIndex(context.GetArgument<int>(0)))
		{
			calmingQuad->minX = context.GetArgument<int16_t>(1);
			calmingQuad->minY = context.GetArgument<int16_t>(2);
			calmingQuad->maxX = context.GetArgument<int16_t>(3);
			calmingQuad->maxY = context.GetArgument<int16_t>(4);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_CALMING_QUAD_BOUNDS", [](fx::ScriptContext& context)
	{
		int minX = 0,
			minY = 0,
			maxX = 0,
			maxY = 0;

		if (auto calmingQuad = GetCalmingQuadByIndex(context.GetArgument<int>(0)))
		{
			minX = calmingQuad->minX;
			minY = calmingQuad->minY;
			maxX = calmingQuad->maxX;
			maxY = calmingQuad->maxY;
		}

		*context.GetArgument<int*>(1) = minX;
		*context.GetArgument<int*>(2) = minY;
		*context.GetArgument<int*>(3) = maxX;
		*context.GetArgument<int*>(4) = maxY;
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_CALMING_QUAD_DAMPENING", [](fx::ScriptContext& context)
	{
		if (auto calmingQuad = GetCalmingQuadByIndex(context.GetArgument<int>(0)))
		{
			calmingQuad->dampening = context.GetArgument<float>(1);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_CALMING_QUAD_DAMPENING", [](fx::ScriptContext& context)
	{
		auto result = 0.0f;

		if (auto calmingQuad = GetCalmingQuadByIndex(context.GetArgument<int>(0)))
		{
			result = calmingQuad->dampening;
		}

		context.SetResult<float>(result);
	});


	fx::ScriptEngine::RegisterNativeHandler("SET_WAVE_QUAD_BOUNDS", [](fx::ScriptContext& context)
	{
		if (auto waveQuad = GetWaveQuadByIndex(context.GetArgument<int>(0)))
		{
			waveQuad->minX = context.GetArgument<int16_t>(1);
			waveQuad->minY = context.GetArgument<int16_t>(2);
			waveQuad->maxX = context.GetArgument<int16_t>(3);
			waveQuad->maxY = context.GetArgument<int16_t>(4);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WAVE_QUAD_BOUNDS", [](fx::ScriptContext& context)
	{
		int minX = 0,
			minY = 0,
			maxX = 0,
			maxY = 0;

		if (auto waveQuad = GetWaveQuadByIndex(context.GetArgument<int>(0)))
		{
			minX = waveQuad->minX;
			minY = waveQuad->minY;
			maxX = waveQuad->maxX;
			maxY = waveQuad->maxY;
		}

		*context.GetArgument<int*>(1) = minX;
		*context.GetArgument<int*>(2) = minY;
		*context.GetArgument<int*>(3) = maxX;
		*context.GetArgument<int*>(4) = maxY;
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_WAVE_QUAD_AMPLITUDE", [](fx::ScriptContext& context)
	{
		if (auto waveQuad = GetWaveQuadByIndex(context.GetArgument<int>(0)))
		{
			auto value = context.GetArgument<float>(1);
			waveQuad->amplitude = (uint16_t)floor(value * 255.0f);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WAVE_QUAD_AMPLITUDE", [](fx::ScriptContext& context)
	{
		auto result = 0.0f;

		if (auto waveQuad = GetWaveQuadByIndex(context.GetArgument<int>(0)))
		{
			result = ((float)waveQuad->amplitude / 255.0f);
		}

		context.SetResult<float>(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_WAVE_QUAD_DIRECTION", [](fx::ScriptContext& context)
	{
		if (auto waveQuad = GetWaveQuadByIndex(context.GetArgument<int>(0)))
		{
			auto valueX = context.GetArgument<float>(1);
			auto valueY = context.GetArgument<float>(2);

			waveQuad->directionX = (uint8_t)floor(valueX * 127.0f);
			waveQuad->directionY = (uint8_t)floor(valueX * 127.0f);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WAVE_QUAD_DIRECTION", [](fx::ScriptContext& context)
	{
		auto directionX = 0.0f;
		auto directionY = 0.0f;

		if (auto waveQuad = GetWaveQuadByIndex(context.GetArgument<int>(0)))
		{
			directionX = (waveQuad->directionX / 127.0f);
			directionY = (waveQuad->directionY / 127.0f);
		}

		*context.GetArgument<float*>(1) = directionX;
		*context.GetArgument<float*>(2) = directionY;
	});


	fx::ScriptEngine::RegisterNativeHandler("GET_WATER_QUAD_COUNT", [](fx::ScriptContext& context)
	{
		context.SetResult<int>(g_waterQuads->GetCount());
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_CALMING_QUAD_COUNT", [](fx::ScriptContext& context)
	{
		context.SetResult<int>(g_calmingQuads->GetCount());
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WAVE_QUAD_COUNT", [](fx::ScriptContext& context)
	{
		context.SetResult<int>(g_waveQuads->GetCount());
	});


	fx::ScriptEngine::RegisterNativeHandler("RESET_WATER", [](fx::ScriptContext& context)
	{
		_unloadWater();
		_loadWater(nullptr);
	});

	// Citizen.InvokeNative(`LOAD_WATER_FROM_PATH` & 0xFFFFFFFF, "watertest", "water.xml")
	fx::ScriptEngine::RegisterNativeHandler("LOAD_WATER_FROM_PATH", [](fx::ScriptContext& context)
	{
		fx::ResourceManager* resourceManager = fx::ResourceManager::GetCurrent();
		fwRefContainer<fx::Resource> resource = resourceManager->GetResource(context.GetArgument<const char*>(0));

		if (!resource.GetRef())
		{
			context.SetResult(false);
			return;
		}

		auto filePath = resource->GetPath() + "/" + context.GetArgument<const char*>(1);
		fwRefContainer<vfs::Stream> stream = vfs::OpenRead(filePath);

		if (!stream.GetRef())
		{
			context.SetResult(false);
			return;
		}

		_unloadWater();
		_loadWater(filePath.c_str());

		context.SetResult(true);
	});
});
