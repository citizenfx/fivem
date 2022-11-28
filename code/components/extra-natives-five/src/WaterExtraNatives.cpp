#include "StdInc.h"
#include <Hooking.h>
#include <ScriptEngine.h>
#include <ICoreGameInit.h>

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

std::vector<sDepthBound> g_dryAreas = {}; // maybe the r* casino volume should be moved to this vector?

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

static HookFunction initFunction([]()
{
	// Physics calls
	hook::call(hook::get_pattern("E8 ? ? ? ? 48 8B CB E8 ? ? ? ? 44 38 3D ? ? ? ? 74 10 8A 83"), CustomTestForWaterPhysics);
	hook::call(hook::get_pattern("E8 ? ? ? ? EB 07 83 A1 ? ? ? ? ? 48 83 C4 38 C3"), CustomTestForWaterPhysics);
	hook::call(hook::get_pattern("E8 ? ? ? ? 0F 28 CE 48 8B CF E8 ? ? ? ? 66 89 B7 ? ? ? ? 89 B7 ? ? ? ? 8A 8F ? ? ? ? 3A 8F"), CustomTestForWaterPhysics);

	// Visual calls
	hook::call(hook::get_pattern("E8 ? ? ? ? 33 F6 84 C0 0F 84 ? ? ? ? F3 0F 10 4F ? F3 0F 10 07 E8"), CustomTestForUnderwaterVisuals);

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

	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		g_dryAreas = {};
	});
});
