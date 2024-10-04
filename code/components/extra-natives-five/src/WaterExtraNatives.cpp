#include "StdInc.h"
#include <Hooking.h>
#include <MinHook.h>
#include <ScriptEngine.h>
#include <ICoreGameInit.h>

#include <ResourceManager.h>
#include <ResourceMetaDataComponent.h>
#include <VFSManager.h>

#include "atArray.h"
#include "GameInit.h"

struct sDepthBound
{
	__m128 min, max;
	bool isRemoved;

	sDepthBound(std::array<float, 4> min, std::array<float, 4> max)
		: min(_mm_load_ps(min.data())), max(_mm_load_ps(max.data())), isRemoved(false)
	{
	}

	bool operator==(const sDepthBound& bound) const
	{
		bool isEqualMin = ((_mm_movemask_ps(_mm_cmpeq_ps(this->min, bound.min))) & 7) == 7;
		bool isEqualMax = ((_mm_movemask_ps(_mm_cmpeq_ps(this->max, bound.max))) & 7) == 7;
		return isEqualMax && isEqualMin;
	}

	void MarkAsRemoved()
	{
		isRemoved = true;
		max = _mm_setzero_ps();
		min = _mm_setzero_ps();
	}
};

struct WaterQuad
{
	int16_t minX; // +0
	int16_t minY; // +2
	int16_t maxX; // +4
	int16_t maxY; // +6
	uint8_t a[4]; // +8
	float target; // +12
	float angle; // +16
	float level; // +20
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

struct CWaterArea
{
	// Those define maximum allowed coordinates for water quads

	static inline int32_t* sm_ClipMinX;
	static inline int32_t* sm_ClipMinY;
	static inline int32_t* sm_ClipMaxX;
	static inline int32_t* sm_ClipMaxY;
};

static atArray<WaterQuad>* g_waterQuads;
static atArray<CalmingQuad>* g_calmingQuads;
static atArray<WaveQuad>* g_waveQuads;
std::vector<sDepthBound> g_dryAreas = {}; // maybe the r* casino volume should be moved to this vector?
std::mutex g_loaderLock;

inline bool IsInBounds(__m128* entityCoords, sDepthBound bounds)
{
	if (bounds.isRemoved)
	{
		return false;
	}
	bool isInsideMin = (_mm_movemask_ps(_mm_cmple_ps(*entityCoords, bounds.max)) & 7) == 7; // Check every coordinate component to be less than the max and create a mask per component -1 = true, 0 = false
	bool isInsideMax = (_mm_movemask_ps(_mm_cmple_ps(bounds.min, *entityCoords)) & 7) == 7; // Convert the result to an integer mask, all 3 bits need to be set aka 7.

	return isInsideMin && isInsideMax;
}

static hook::cdecl_stub<void*(__m128*, bool, uint64_t)> _TestForUnderWaterVisuals([]()
{
	return hook::get_pattern("48 8B C4 48 89 58 08 48 89 68 18 48 89 70 20 57 48 81 EC ? ? ? ? 83 48 10 FF");
});

static hook::cdecl_stub<void*(void*, __m128*, uint64_t)> _TestForWaterPhysics([]()
{
	return hook::get_pattern("48 8B C4 55 53 56 57 41 54 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? F3");
});

static hook::cdecl_stub<void(const char* path)> _loadWater([]()
{
	return hook::get_pattern("48 85 C9 75 ? 8D 51 ? 48 8B 0D", xbr::IsGameBuildOrGreater<2189>() ? -20 : -22);
});

static hook::cdecl_stub<void()> _unloadWater([]()
{
	return hook::get_call(hook::get_pattern("BE 02 00 00 00 3B CE 75", 9));
});

bool ShouldVolumeBeDry(__m128* entityCoords)
{
	bool result = false;
	for (const sDepthBound& area : g_dryAreas)
	{
		result += IsInBounds(entityCoords, area);
	};
	return result;
}

void* CustomTestForWaterPhysics(void* a1, __m128* entityCoords, uint64_t a3)
{
	if (ShouldVolumeBeDry(entityCoords))
	{
		return 0;
	}
	return _TestForWaterPhysics(a1, entityCoords, a3);
}

void* CustomTestForUnderwaterVisuals(__m128* entityCoords, bool a2, uint64_t a3)
{
	if (ShouldVolumeBeDry(entityCoords))
	{
		return 0;
	}
	return _TestForUnderWaterVisuals(entityCoords, a2, a3);
}

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
		return context.SetResult<bool>(false);
	}

	if (WaterQuad* waterQuad = GetWaterQuadByIndex(context.GetArgument<int>(0)))
	{
		auto value = (context.GetArgument<bool>(1) != 0);
		waterQuad->flags = (waterQuad->flags & ~(1 << bit)) | (value << bit);
		return context.SetResult<bool>(true);
	}

	context.SetResult<bool>(false);
}

template<int bit>
static void ReadWaterQuadFlag(fx::ScriptContext& context)
{
	bool found = false;
	bool value = false;
	if (WaterQuad* waterQuad = GetWaterQuadByIndex(context.GetArgument<int>(0)))
	{
		found = true;
		value = (waterQuad->flags & (1 << bit)) != 0;
	}
	context.SetResult<bool>(found);
	*context.GetArgument<bool*>(1) = value;
}

static void (*g_orig_RenderWaterNearQuads)(int32_t, int32_t);
static void RenderWaterNearQuads(int32_t effectIndex, int32_t oceanHeightIndex)
{
	std::lock_guard _(g_loaderLock);
	g_orig_RenderWaterNearQuads(effectIndex, oceanHeightIndex);
}

static void (*g_orig_RenderWaterTessellation)(int32_t, bool, int32_t);
static void RenderWaterTessellation(int32_t effectIndex, bool a2, int32_t oceanHeightIndex)
{
	std::lock_guard _(g_loaderLock);
	g_orig_RenderWaterTessellation(effectIndex, a2, oceanHeightIndex);
}

static void (*g_orig_RenderWaterFarQuads)(int32_t, int32_t);
static void RenderWaterFarQuads(int32_t effectIndex, int32_t oceanHeightIndex)
{
	std::lock_guard _(g_loaderLock);
	g_orig_RenderWaterFarQuads(effectIndex, oceanHeightIndex);
}

static void (*g_orig_RenderWaterCubemap)();
static void RenderWaterCubemap()
{
	std::lock_guard _(g_loaderLock);
	g_orig_RenderWaterCubemap();
}

static bool checkWaterBounds(std::array<float, 4> targetCoords, std::array<float, 4> min, std::array<float, 4> max)
{
	sDepthBound waterQuadBounds = {
		min,
		max
	};

	auto packedTargetCoords = _mm_load_ps(targetCoords.data());

	return IsInBounds(&packedTargetCoords, waterQuadBounds);
}

template<bool ignoreZ, class WaterType, class IndexFunction>
static int IterateWaveType(WaterType waterType, IndexFunction getIndex, float x, float y, float z = 0.f)
{
	for (int i = 0; i < waterType->GetCount(); i++)
	{
		auto quad = getIndex(i);
		if (quad)
		{
			float level = 0.f;
			if constexpr (!ignoreZ)
			{
				level = quad->level;
			}

			std::array<float, 4> targetCoords = { x, y, ignoreZ ? 0.f : z, 0.f };
			std::array<float, 4> min = { quad->minX, quad->minY, -9999.f, 0.f };
			std::array<float, 4> max = { quad->maxX, quad->maxY, level, 0.f };

			if (checkWaterBounds(targetCoords, min, max))
			{
				return i;
			}
		}
	}
	return -1;
}

static HookFunction initFunction([]()
{
	// Physics calls
	hook::call(hook::get_pattern("E8 ? ? ? ? 48 8B CB E8 ? ? ? ? 44 38 3D ? ? ? ? 74 10 8A 83"), CustomTestForWaterPhysics);
	hook::call(hook::get_pattern("E8 ? ? ? ? EB 07 83 A1 ? ? ? ? ? 48 83 C4 38 C3"), CustomTestForWaterPhysics);
	hook::call(hook::get_pattern("E8 ? ? ? ? 0F 28 CE 48 8B CF E8 ? ? ? ? 66 89 B7 ? ? ? ? 89 B7 ? ? ? ? 8A 8F ? ? ? ? 3A 8F"), CustomTestForWaterPhysics);

	// Visual calls
	hook::call(hook::get_pattern("E8 ? ? ? ? 33 F6 84 C0 0F 84 ? ? ? ? F3 0F 10 4F ? F3 0F 10 07 E8"), CustomTestForUnderwaterVisuals);

	// Water level
	if (xbr::IsGameBuildOrGreater<2189>())
	{
		// This function processes water file set request by LOAD_GLOBAL_WATER_FILE native
		uintptr_t updateWaterData = (uintptr_t)hook::get_pattern("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 20 8A 05 ? ? ? ? 33");

		// Loads water file & sets hardcoded clip rect for LosSantos / Heist Island
		uintptr_t setWaterAreaType = hook::get_call<uintptr_t>(updateWaterData + 0x59);

		CWaterArea::sm_ClipMinX = hook::get_address<int32_t*>(setWaterAreaType + 0x29, 2, 6);
		CWaterArea::sm_ClipMinY = hook::get_address<int32_t*>(setWaterAreaType + 0x2F, 2, 6);
		CWaterArea::sm_ClipMaxX = hook::get_address<int32_t*>(setWaterAreaType + 0x1F, 2, 10);
		CWaterArea::sm_ClipMaxY = hook::get_address<int32_t*>(setWaterAreaType + 0x35, 2, 10);

		int32_t defaultClipMinX = *CWaterArea::sm_ClipMinX;
		int32_t defaultClipMinY = *CWaterArea::sm_ClipMinY;
		int32_t defaultClipMaxX = *CWaterArea::sm_ClipMaxX;
		int32_t defaultClipMaxY = *CWaterArea::sm_ClipMaxY;

		// Reset water to default on disconnect
		OnKillNetworkDone.Connect([=]
		{
			*CWaterArea::sm_ClipMinX = defaultClipMinX;
			*CWaterArea::sm_ClipMinY = defaultClipMinY;
			*CWaterArea::sm_ClipMaxX = defaultClipMaxX;
			*CWaterArea::sm_ClipMaxY = defaultClipMaxY;
		});
	}

	// Water quads
	{
		static_assert(sizeof(WaterQuad) == 28, "Wrong WaveQuad struct size");
		static_assert(sizeof(CalmingQuad) == 12, "Wrong CalmingQuad struct size");
		static_assert(sizeof(WaveQuad) == 12, "Wrong WaveQuad struct size");

		auto location = hook::get_pattern<char>("83 F9 04 75 ? 48 8B 0D ? ? ? ? E8 ? ? ? ? 48 8B 0D");
		g_waterQuads = hook::get_address<atArray<WaterQuad>*>(location + 8);
		g_calmingQuads = hook::get_address<atArray<CalmingQuad>*>(location + 20);
		g_waveQuads = hook::get_address<atArray<WaveQuad>*>(location + 45);

		location = hook::get_pattern<char>("E8 ? ? ? ? 8B D7 8B CE E8 ? ? ? ? 44 8B C7");
		MH_CreateHook(hook::get_call(location), RenderWaterFarQuads, (void**)&g_orig_RenderWaterFarQuads);
		MH_CreateHook(hook::get_call(location + 0x9), RenderWaterNearQuads, (void**)&g_orig_RenderWaterNearQuads);
		MH_CreateHook(hook::get_call(location + 0x17), RenderWaterTessellation, (void**)&g_orig_RenderWaterTessellation);

		location = hook::get_pattern<char>("83 FB 04 74 ? 48 8D 0D", 5);
		MH_CreateHook(hook::get_address<void**>(location, 3, 7), RenderWaterCubemap, (void**)&g_orig_RenderWaterCubemap);
		MH_EnableHook(MH_ALL_HOOKS);
	}

	fx::ScriptEngine::RegisterNativeHandler("SET_WATER_AREA_CLIP_RECT", [](fx::ScriptContext& context)
	{
		if (!xbr::IsGameBuildOrGreater<2189>())
			return;

		int32_t minX = context.GetArgument<int>(0);
		int32_t minY = context.GetArgument<int>(1);
		int32_t maxX = context.GetArgument<int>(2);
		int32_t maxY = context.GetArgument<int>(3);

		// Inverted BB will crash game
		if (maxX < minX || maxY < minY)
		{
			trace("Given clip rectangle is inverted, make sure that min coordinate is less or equal to max one.");
			return;
		}

		*CWaterArea::sm_ClipMinX = minX;
		*CWaterArea::sm_ClipMinY = minY;
		*CWaterArea::sm_ClipMaxX = maxX;
		*CWaterArea::sm_ClipMaxY = maxY;
	});

	fx::ScriptEngine::RegisterNativeHandler("CREATE_DRY_VOLUME", [=](fx::ScriptContext& context)
	{
		auto newVolume = sDepthBound(
		{ context.GetArgument<float>(0), context.GetArgument<float>(1), context.GetArgument<float>(2), 0.f },
		{ context.GetArgument<float>(3), context.GetArgument<float>(4), context.GetArgument<float>(5), 0.f });

		std::vector<sDepthBound>::iterator v = std::find(g_dryAreas.begin(), g_dryAreas.end(), newVolume);
		if (v != g_dryAreas.end())
		{
			int index = v - g_dryAreas.begin();
			context.SetResult<int>(index);
			trace("A duplicate dry volume was attempted to be created, the existing handle has been returned instead.\n");
		}
		else
		{
			context.SetResult<int>(g_dryAreas.size());
			g_dryAreas.push_back(newVolume);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("REMOVE_DRY_VOLUME", [=](fx::ScriptContext& context)
	{
		int index = context.GetArgument<int>(0);
		if (index >= g_dryAreas.size() || index < 0)
		{
			trace("Attempting to delete an invalid dry volume handle (%d)\n", index);
			return;
		}
		g_dryAreas[index].MarkAsRemoved();
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WATER_QUAD_AT_COORDS_3D", [](fx::ScriptContext& context)
	{
		float x = context.GetArgument<float>(0);
		float y = context.GetArgument<float>(1);
		float z = context.GetArgument<float>(2);

		context.SetResult<int>(IterateWaveType<false>(g_waterQuads, GetWaterQuadByIndex, x, y, z));
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WATER_QUAD_AT_COORDS", [](fx::ScriptContext& context)
	{
		float x = context.GetArgument<float>(0);
		float y = context.GetArgument<float>(1);

		context.SetResult<int>(IterateWaveType<true>(g_waterQuads, GetWaterQuadByIndex, x, y));
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_CALMING_QUAD_AT_COORDS", [](fx::ScriptContext& context)
	{
		float x = context.GetArgument<float>(0);
		float y = context.GetArgument<float>(1);

		context.SetResult<int>(IterateWaveType<true>(g_calmingQuads, GetCalmingQuadByIndex, x, y));
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WAVE_QUAD_AT_COORDS", [](fx::ScriptContext& context)
	{
		float x = context.GetArgument<float>(0);
		float y = context.GetArgument<float>(1);

		context.SetResult<int>(IterateWaveType<true>(g_waveQuads, GetWaveQuadByIndex, x, y));
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_WATER_QUAD_BOUNDS", [](fx::ScriptContext& context)
	{
		bool found = false;
		if (auto waterQuad = GetWaterQuadByIndex(context.GetArgument<int>(0)))
		{
			waterQuad->minX = context.GetArgument<int16_t>(1);
			waterQuad->minY = context.GetArgument<int16_t>(2);
			waterQuad->maxX = context.GetArgument<int16_t>(3);
			waterQuad->maxY = context.GetArgument<int16_t>(4);
			found = true;
		}
		context.SetResult<bool>(found);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WATER_QUAD_BOUNDS", [](fx::ScriptContext& context)
	{
		bool found = false;
		int minX = 0,
			minY = 0,
			maxX = 0,
			maxY = 0;

		if (auto waterQuad = GetWaterQuadByIndex(context.GetArgument<int>(0)))
		{
			found = true;
			minX = waterQuad->minX;
			minY = waterQuad->minY;
			maxX = waterQuad->maxX;
			maxY = waterQuad->maxY;
		}
		context.SetResult<bool>(found);
		*context.GetArgument<int*>(1) = minX;
		*context.GetArgument<int*>(2) = minY;
		*context.GetArgument<int*>(3) = maxX;
		*context.GetArgument<int*>(4) = maxY;
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_WATER_QUAD_LEVEL", [](fx::ScriptContext& context)
	{
		bool found = false;
		if (auto waterQuad = GetWaterQuadByIndex(context.GetArgument<int>(0)))
		{
			found = true;
			waterQuad->level = context.GetArgument<float>(1);
		}
		context.SetResult<bool>(found);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WATER_QUAD_LEVEL", [](fx::ScriptContext& context)
	{
		bool found = false;
		auto result = 0.0f;

		if (auto waterQuad = GetWaterQuadByIndex(context.GetArgument<int>(0)))
		{
			found = true;
			result = waterQuad->level;
		}
		context.SetResult<bool>(found);
		*context.GetArgument<float*>(1) = result;
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_WATER_QUAD_TYPE", [](fx::ScriptContext& context)
	{
		if (auto waterQuad = GetWaterQuadByIndex(context.GetArgument<int>(0)))
		{
			waterQuad->type = context.GetArgument<int>(1);
			return context.SetResult<bool>(true);
		}
		context.SetResult<bool>(false);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WATER_QUAD_TYPE", [](fx::ScriptContext& context)
	{
		bool found = false;
		auto result = -1;

		if (auto waterQuad = GetWaterQuadByIndex(context.GetArgument<int>(0)))
		{
			found = true;
			result = waterQuad->type;
		}
		context.SetResult<bool>(found);
		*context.GetArgument<int*>(1) = result;
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_WATER_QUAD_HAS_LIMITED_DEPTH", WriteWaterQuadFlag<1>);

	fx::ScriptEngine::RegisterNativeHandler("GET_WATER_QUAD_HAS_LIMITED_DEPTH", ReadWaterQuadFlag<1>);

	fx::ScriptEngine::RegisterNativeHandler("SET_WATER_QUAD_IS_INVISIBLE", WriteWaterQuadFlag<3>);

	fx::ScriptEngine::RegisterNativeHandler("GET_WATER_QUAD_IS_INVISIBLE", ReadWaterQuadFlag<3>);

	fx::ScriptEngine::RegisterNativeHandler("SET_WATER_QUAD_NO_STENCIL", WriteWaterQuadFlag<4>);

	fx::ScriptEngine::RegisterNativeHandler("GET_WATER_QUAD_NO_STENCIL", ReadWaterQuadFlag<4>);

	fx::ScriptEngine::RegisterNativeHandler("SET_WATER_QUAD_ALPHA", [](fx::ScriptContext& context)
	{
		if (auto waterQuad = GetWaterQuadByIndex(context.GetArgument<int>(0)))
		{
			waterQuad->a[0] = context.GetArgument<int>(1);
			waterQuad->a[1] = context.GetArgument<int>(2);
			waterQuad->a[2] = context.GetArgument<int>(3);
			waterQuad->a[3] = context.GetArgument<int>(4);
			return context.SetResult<bool>(true);
		}
		context.SetResult<bool>(false);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WATER_QUAD_ALPHA", [](fx::ScriptContext& context)
	{
		bool found = false;
		int a0 = 0,
			a1 = 0,
			a2 = 0,
			a3 = 0;

		if (auto waterQuad = GetWaterQuadByIndex(context.GetArgument<int>(0)))
		{
			found = true;
			a0 = waterQuad->a[0];
			a1 = waterQuad->a[1];
			a2 = waterQuad->a[2];
			a3 = waterQuad->a[3];
		}
		context.SetResult<bool>(found);
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
			return context.SetResult<bool>(true);
		}
		context.SetResult<bool>(false);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_CALMING_QUAD_BOUNDS", [](fx::ScriptContext& context)
	{
		bool found = false;
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
			found = true;
		}
		context.SetResult<bool>(found);
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
			return context.SetResult<bool>(true);
		}
		context.SetResult<bool>(false);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_CALMING_QUAD_DAMPENING", [](fx::ScriptContext& context)
	{
		bool found = false;
		auto result = -1.0f;

		if (auto calmingQuad = GetCalmingQuadByIndex(context.GetArgument<int>(0)))
		{
			found = true;
			result = calmingQuad->dampening;
		}

		context.SetResult<bool>(found);
		*context.GetArgument<float*>(1) = result;
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_WAVE_QUAD_BOUNDS", [](fx::ScriptContext& context)
	{
		if (auto waveQuad = GetWaveQuadByIndex(context.GetArgument<int>(0)))
		{
			waveQuad->minX = context.GetArgument<int16_t>(1);
			waveQuad->minY = context.GetArgument<int16_t>(2);
			waveQuad->maxX = context.GetArgument<int16_t>(3);
			waveQuad->maxY = context.GetArgument<int16_t>(4);
			return context.SetResult<bool>(true);
		}
		context.SetResult<bool>(false);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WAVE_QUAD_BOUNDS", [](fx::ScriptContext& context)
	{
		bool found = false;
		int minX = 0,
			minY = 0,
			maxX = 0,
			maxY = 0;

		if (auto waveQuad = GetWaveQuadByIndex(context.GetArgument<int>(0)))
		{
			found = true;
			minX = waveQuad->minX;
			minY = waveQuad->minY;
			maxX = waveQuad->maxX;
			maxY = waveQuad->maxY;
		}

		context.SetResult<bool>(found);
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
			return context.SetResult<bool>(true);
		}
		context.SetResult<bool>(false);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WAVE_QUAD_AMPLITUDE", [](fx::ScriptContext& context)
	{
		bool found = false;
		auto result = -1.0f;

		if (auto waveQuad = GetWaveQuadByIndex(context.GetArgument<int>(0)))
		{
			found = true;
			result = ((float)waveQuad->amplitude / 255.0f);
		}
		context.SetResult<bool>(found);
		*context.GetArgument<float*>(1) = result;
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_WAVE_QUAD_DIRECTION", [](fx::ScriptContext& context)
	{
		if (auto waveQuad = GetWaveQuadByIndex(context.GetArgument<int>(0)))
		{
			auto valueX = context.GetArgument<float>(1);
			auto valueY = context.GetArgument<float>(2);

			waveQuad->directionX = (uint8_t)floor(valueX * 127.0f);
			waveQuad->directionY = (uint8_t)floor(valueY * 127.0f);
			return context.SetResult<bool>(true);
		}
		context.SetResult<bool>(false);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_WAVE_QUAD_DIRECTION", [](fx::ScriptContext& context)
	{
		bool found = false;
		auto directionX = 0.0f;
		auto directionY = 0.0f;

		if (auto waveQuad = GetWaveQuadByIndex(context.GetArgument<int>(0)))
		{
			found = true;
			directionX = (waveQuad->directionX / 127.0f);
			directionY = (waveQuad->directionY / 127.0f);
		}

		context.SetResult<bool>(found);
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
		std::lock_guard _(g_loaderLock);
		_unloadWater();
		_loadWater(nullptr);
	});

	fx::ScriptEngine::RegisterNativeHandler("LOAD_WATER_FROM_PATH", [](fx::ScriptContext& context)
	{
		fx::ResourceManager* resourceManager = fx::ResourceManager::GetCurrent();
		fwRefContainer<fx::Resource> resource = resourceManager->GetResource(context.GetArgument<const char*>(0));

		if (!resource.GetRef())
		{
			context.SetResult(false);
			return;
		}

		std::string filePath = resource->GetPath();

		// Make sure path separator exists or add it before combining path with file name
		char c = filePath[filePath.length() - 1];
		if (c != '/' && c != '\\')
			filePath += '/';

		filePath += context.GetArgument<const char*>(1);

		fwRefContainer<vfs::Stream> stream = vfs::OpenRead(filePath);
		if (!stream.GetRef())
		{
			trace("unable to find water file at %s\n", filePath.c_str());
			context.SetResult(false);
			return;
		}

		std::lock_guard _(g_loaderLock);
		_unloadWater();
		_loadWater(filePath.c_str());

		context.SetResult(true);
	});

	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		g_dryAreas = {};
		std::lock_guard _(g_loaderLock);
		_unloadWater();
		_loadWater(nullptr);
	});
});
