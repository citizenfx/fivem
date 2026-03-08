#include "StdInc.h"

#include <ScriptEngine.h>

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

namespace
{

// Replace these with official generated native identifiers if available in your branch.
constexpr uint64_t kPlayEntityScriptedAnimHash = 0x77A1EEC547E7FCF1ULL;
constexpr uint64_t kTaskScriptedAnimationHash = 0x126EF75F1E17ABE5ULL;

// 8-byte script value slot layout compatible with the game-side animation init struct.
union ValueSlot
{
	int32_t Int;
	uint32_t Uns;
	float Float;
	const char* String;
	void* Pointer;
	uint64_t Raw;
};

static_assert(sizeof(ValueSlot) == 8, "ValueSlot must be 8 bytes");

struct ScriptInitSlotData
{
	ValueSlot state;

	ValueSlot dict0;
	ValueSlot clip0;
	ValueSlot phase0;
	ValueSlot rate0;
	ValueSlot weight0;

	ValueSlot dict1;
	ValueSlot clip1;
	ValueSlot phase1;
	ValueSlot rate1;
	ValueSlot weight1;

	ValueSlot dict2;
	ValueSlot clip2;
	ValueSlot phase2;
	ValueSlot rate2;
	ValueSlot weight2;

	ValueSlot filter;
	ValueSlot blendInDuration;
	ValueSlot blendOutDuration;
	ValueSlot timeToPlay;
	ValueSlot flags;
	ValueSlot ikFlags;
};

static_assert(sizeof(ScriptInitSlotData) == 0xB0, "ScriptInitSlotData must be 0xB0 bytes");


struct StoredAnimSlot
{
	ScriptInitSlotData data{};

	std::string dict0;
	std::string clip0;
	std::string dict1;
	std::string clip1;
	std::string dict2;
	std::string clip2;
};

static std::unordered_map<uint32_t, std::unique_ptr<StoredAnimSlot>> g_animSlots;
static std::mutex g_animSlotsMutex;
static uint32_t g_nextAnimSlotHandle = 1;

static void RefreshAnimSlotPointers(StoredAnimSlot& slot)
{
	slot.data.dict0.String = slot.dict0.empty() ? nullptr : slot.dict0.c_str();
	slot.data.clip0.String = slot.clip0.empty() ? nullptr : slot.clip0.c_str();

	slot.data.dict1.String = slot.dict1.empty() ? nullptr : slot.dict1.c_str();
	slot.data.clip1.String = slot.clip1.empty() ? nullptr : slot.clip1.c_str();

	slot.data.dict2.String = slot.dict2.empty() ? nullptr : slot.dict2.c_str();
	slot.data.clip2.String = slot.clip2.empty() ? nullptr : slot.clip2.c_str();
}

static void FillDefaultAnimSlot(StoredAnimSlot& slot)
{
	std::memset(&slot.data, 0, sizeof(slot.data));

	slot.data.state.Int = 0;
	slot.data.filter.Int = 0;
	slot.data.blendInDuration.Float = 0.125f;
	slot.data.blendOutDuration.Float = 0.125f;
	slot.data.timeToPlay.Int = -1;
	slot.data.flags.Int = 0;
	slot.data.ikFlags.Int = 0;

	slot.dict0.clear();
	slot.clip0.clear();
	slot.dict1.clear();
	slot.clip1.clear();
	slot.dict2.clear();
	slot.clip2.clear();

	slot.data.phase0.Float = 0.0f;
	slot.data.rate0.Float = 1.0f;
	slot.data.weight0.Float = 1.0f;

	slot.data.phase1.Float = 0.0f;
	slot.data.rate1.Float = 0.0f;
	slot.data.weight1.Float = 0.0f;

	slot.data.phase2.Float = 0.0f;
	slot.data.rate2.Float = 0.0f;
	slot.data.weight2.Float = 0.0f;

	RefreshAnimSlotPointers(slot);
}

static uint32_t AllocateAnimSlotHandleLocked()
{
	for (;;)
	{
		const uint32_t handle = g_nextAnimSlotHandle++;

		if (handle == 0)
		{
			continue;
		}

		if (g_animSlots.find(handle) == g_animSlots.end())
		{
			return handle;
		}
	}
}

static void RemoveAnimSlot(uint32_t handle)
{
	std::lock_guard<std::mutex> lock(g_animSlotsMutex);
	g_animSlots.erase(handle);
}

static bool CopyAnimSlotSnapshots(uint32_t lowHandle, uint32_t midHandle, uint32_t highHandle, StoredAnimSlot& lowOut, StoredAnimSlot& midOut, StoredAnimSlot& highOut)
{
	std::lock_guard<std::mutex> lock(g_animSlotsMutex);

	auto itLow = g_animSlots.find(lowHandle);
	auto itMid = g_animSlots.find(midHandle);
	auto itHigh = g_animSlots.find(highHandle);

	if (itLow == g_animSlots.end() || itMid == g_animSlots.end() || itHigh == g_animSlots.end())
	{
		return false;
	}

	lowOut = *itLow->second;
	midOut = *itMid->second;
	highOut = *itHigh->second;

	RefreshAnimSlotPointers(lowOut);
	RefreshAnimSlotPointers(midOut);
	RefreshAnimSlotPointers(highOut);

	return true;
}

static bool CallScriptedAnimNative(uint64_t nativeHash, int entity, ScriptInitSlotData* low, ScriptInitSlotData* mid, ScriptInitSlotData* high, float taskBlendIn, float taskBlendOut)
{
	fx::ScriptContextBuffer context;
	context.Push(entity);
	context.Push(low);
	context.Push(mid);
	context.Push(high);
	context.Push(taskBlendIn);
	context.Push(taskBlendOut);

	return fx::ScriptEngine::CallNativeHandler(nativeHash, context);
}

static bool PlayEntityScriptedAnimInternal(int entity, ScriptInitSlotData* low, ScriptInitSlotData* mid, ScriptInitSlotData* high, float taskBlendIn, float taskBlendOut)
{
	return CallScriptedAnimNative(kPlayEntityScriptedAnimHash, entity, low, mid, high, taskBlendIn, taskBlendOut);
}

static bool TaskScriptedAnimationInternal(int entity, ScriptInitSlotData* low, ScriptInitSlotData* mid, ScriptInitSlotData* high, float taskBlendIn, float taskBlendOut)
{
	return CallScriptedAnimNative(kTaskScriptedAnimationHash, entity, low, mid, high, taskBlendIn, taskBlendOut);
}

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("CREATE_SCRIPTED_ANIM_SLOT", [](fx::ScriptContext& context)
	{
		auto slot = std::make_unique<StoredAnimSlot>();
		FillDefaultAnimSlot(*slot);

		slot->data.state.Int = context.GetArgument<int>(0);

		slot->dict0 = context.GetArgument<const char*>(1) ? context.GetArgument<const char*>(1) : "";
		slot->clip0 = context.GetArgument<const char*>(2) ? context.GetArgument<const char*>(2) : "";

		slot->data.phase0.Float = context.GetArgument<float>(3);
		slot->data.rate0.Float = context.GetArgument<float>(4);
		slot->data.weight0.Float = context.GetArgument<float>(5);

		slot->data.filter.Int = context.GetArgument<int>(6);
		slot->data.blendInDuration.Float = context.GetArgument<float>(7);
		slot->data.blendOutDuration.Float = context.GetArgument<float>(8);
		slot->data.timeToPlay.Int = context.GetArgument<int>(9);
		slot->data.flags.Int = context.GetArgument<int>(10);
		slot->data.ikFlags.Int = context.GetArgument<int>(11);

		RefreshAnimSlotPointers(*slot);

		uint32_t handle = 0;

		{
			std::lock_guard<std::mutex> lock(g_animSlotsMutex);
			handle = AllocateAnimSlotHandleLocked();
			g_animSlots.emplace(handle, std::move(slot));
		}

		context.SetResult<uint32_t>(handle);
	});

	fx::ScriptEngine::RegisterNativeHandler("CREATE_EMPTY_SCRIPTED_ANIM_SLOT", [](fx::ScriptContext& context)
	{
		auto slot = std::make_unique<StoredAnimSlot>();
		FillDefaultAnimSlot(*slot);

		uint32_t handle = 0;

		{
			std::lock_guard<std::mutex> lock(g_animSlotsMutex);
			handle = AllocateAnimSlotHandleLocked();
			g_animSlots.emplace(handle, std::move(slot));
		}

		context.SetResult<uint32_t>(handle);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_SCRIPTED_ANIM_SLOT_CLIP", [](fx::ScriptContext& context)
	{
		const uint32_t handle = context.GetArgument<uint32_t>(0);
		const int clipIndex = context.GetArgument<int>(1);
		const char* dict = context.GetArgument<const char*>(2);
		const char* clip = context.GetArgument<const char*>(3);
		const float phase = context.GetArgument<float>(4);
		const float rate = context.GetArgument<float>(5);
		const float weight = context.GetArgument<float>(6);

		std::lock_guard<std::mutex> lock(g_animSlotsMutex);

		auto it = g_animSlots.find(handle);
		if (it == g_animSlots.end())
		{
			context.SetResult<bool>(false);
			return;
		}

		StoredAnimSlot& slot = *it->second;

		switch (clipIndex)
		{
			case 0:
				slot.dict0 = dict ? dict : "";
				slot.clip0 = clip ? clip : "";
				slot.data.phase0.Float = phase;
				slot.data.rate0.Float = rate;
				slot.data.weight0.Float = weight;
				break;

			case 1:
				slot.dict1 = dict ? dict : "";
				slot.clip1 = clip ? clip : "";
				slot.data.phase1.Float = phase;
				slot.data.rate1.Float = rate;
				slot.data.weight1.Float = weight;
				break;

			case 2:
				slot.dict2 = dict ? dict : "";
				slot.clip2 = clip ? clip : "";
				slot.data.phase2.Float = phase;
				slot.data.rate2.Float = rate;
				slot.data.weight2.Float = weight;
				break;

			default:
				context.SetResult<bool>(false);
				return;
		}

		RefreshAnimSlotPointers(slot);
		context.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_SCRIPTED_ANIM_SLOT_PARAMS", [](fx::ScriptContext& context)
	{
		const uint32_t handle = context.GetArgument<uint32_t>(0);

		std::lock_guard<std::mutex> lock(g_animSlotsMutex);

		auto it = g_animSlots.find(handle);
		if (it == g_animSlots.end())
		{
			context.SetResult<bool>(false);
			return;
		}

		StoredAnimSlot& slot = *it->second;
		slot.data.state.Int = context.GetArgument<int>(1);
		slot.data.filter.Int = context.GetArgument<int>(2);
		slot.data.blendInDuration.Float = context.GetArgument<float>(3);
		slot.data.blendOutDuration.Float = context.GetArgument<float>(4);
		slot.data.timeToPlay.Int = context.GetArgument<int>(5);
		slot.data.flags.Int = context.GetArgument<int>(6);
		slot.data.ikFlags.Int = context.GetArgument<int>(7);

		context.SetResult<bool>(true);
	});


	fx::ScriptEngine::RegisterNativeHandler("RESET_SCRIPTED_ANIM_SLOT", [](fx::ScriptContext& context)
	{
		const uint32_t handle = context.GetArgument<uint32_t>(0);

		std::lock_guard<std::mutex> lock(g_animSlotsMutex);

		auto it = g_animSlots.find(handle);
		if (it == g_animSlots.end())
		{
			context.SetResult<bool>(false);
			return;
		}

		FillDefaultAnimSlot(*it->second);
		context.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("DELETE_SCRIPTED_ANIM_SLOT", [](fx::ScriptContext& context)
	{
		RemoveAnimSlot(context.GetArgument<uint32_t>(0));
		context.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("CLEAR_SCRIPTED_ANIM_SLOTS", [](fx::ScriptContext& context)
	{
		size_t count = 0;

		{
			std::lock_guard<std::mutex> lock(g_animSlotsMutex);
			count = g_animSlots.size();
			g_animSlots.clear();
			g_nextAnimSlotHandle = 1;
		}

		context.SetResult<bool>(true);
	});

	fx::ScriptEngine::RegisterNativeHandler("PLAY_ENTITY_SCRIPTED_ANIM_EXTENDED", [](fx::ScriptContext& context)
	{
		const int entity = context.GetArgument<int>(0);
		const uint32_t lowHandle = context.GetArgument<uint32_t>(1);
		const uint32_t midHandle = context.GetArgument<uint32_t>(2);
		const uint32_t highHandle = context.GetArgument<uint32_t>(3);
		const float taskBlendIn = context.GetArgument<float>(4);
		const float taskBlendOut = context.GetArgument<float>(5);

		StoredAnimSlot lowSlot;
		StoredAnimSlot midSlot;
		StoredAnimSlot highSlot;

		if (!CopyAnimSlotSnapshots(lowHandle, midHandle, highHandle, lowSlot, midSlot, highSlot))
		{
			context.SetResult<bool>(false);
			return;
		}

		context.SetResult<bool>(CallScriptedAnimNative(
		kPlayEntityScriptedAnimHash,
		entity,
		&lowSlot.data,
		&midSlot.data,
		&highSlot.data,
		taskBlendIn,
		taskBlendOut));
	});

	fx::ScriptEngine::RegisterNativeHandler("TASK_SCRIPTED_ANIMATION_EXTENDED", [](fx::ScriptContext& context)
	{
		const int entity = context.GetArgument<int>(0);
		const uint32_t lowHandle = context.GetArgument<uint32_t>(1);
		const uint32_t midHandle = context.GetArgument<uint32_t>(2);
		const uint32_t highHandle = context.GetArgument<uint32_t>(3);
		const float taskBlendIn = context.GetArgument<float>(4);
		const float taskBlendOut = context.GetArgument<float>(5);

		StoredAnimSlot lowSlot;
		StoredAnimSlot midSlot;
		StoredAnimSlot highSlot;

		if (!CopyAnimSlotSnapshots(lowHandle, midHandle, highHandle, lowSlot, midSlot, highSlot))
		{
			context.SetResult<bool>(false);
			return;
		}

		context.SetResult<bool>(CallScriptedAnimNative(
		kTaskScriptedAnimationHash,
		entity,
		&lowSlot.data,
		&midSlot.data,
		&highSlot.data,
		taskBlendIn,
		taskBlendOut));
	});
});
} // namespace
