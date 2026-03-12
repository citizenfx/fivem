#include "StdInc.h"

#include <map>
#include <string>
#include <cstring>
#include <cstdint>

#include <ScriptEngine.h>
#include <scrEngine.h>
#include <ScriptSerialization.h>
#include "ScriptWarnings.h"

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

struct ScriptInitSlotStorage
{
	ScriptInitSlotData data{};

	std::string dict0;
	std::string clip0;
	std::string dict1;
	std::string clip1;
	std::string dict2;
	std::string clip2;
};

static void FillDefaultScriptInitSlotData(ScriptInitSlotStorage& slot)
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

	slot.data.dict0.String = nullptr;
	slot.data.clip0.String = nullptr;
	slot.data.phase0.Float = 0.0f;
	slot.data.rate0.Float = 1.0f;
	slot.data.weight0.Float = 1.0f;

	slot.data.dict1.String = nullptr;
	slot.data.clip1.String = nullptr;
	slot.data.phase1.Float = 0.0f;
	slot.data.rate1.Float = 0.0f;
	slot.data.weight1.Float = 0.0f;

	slot.data.dict2.String = nullptr;
	slot.data.clip2.String = nullptr;
	slot.data.phase2.Float = 0.0f;
	slot.data.rate2.Float = 0.0f;
	slot.data.weight2.Float = 0.0f;
}

static int GetIntOrDefault(const std::map<std::string, msgpack::object>& data, const char* key, int defaultValue)
{
	auto it = data.find(key);
	if (it == data.end())
	{
		return defaultValue;
	}

	return it->second.as<int>();
}

static float GetFloatOrDefault(const std::map<std::string, msgpack::object>& data, const char* key, float defaultValue)
{
	auto it = data.find(key);
	if (it == data.end())
	{
		return defaultValue;
	}

	return it->second.as<float>();
}

static std::string GetStringOrDefault(const std::map<std::string, msgpack::object>& data, const char* key, const std::string& defaultValue = {})
{
	auto it = data.find(key);
	if (it == data.end())
	{
		return defaultValue;
	}

	if (it->second.is_nil())
	{
		return {};
	}

	return it->second.as<std::string>();
}

static void RefreshScriptInitSlotPointers(ScriptInitSlotStorage& slot)
{
	slot.data.dict0.String = slot.dict0.empty() ? nullptr : slot.dict0.c_str();
	slot.data.clip0.String = slot.clip0.empty() ? nullptr : slot.clip0.c_str();

	slot.data.dict1.String = slot.dict1.empty() ? nullptr : slot.dict1.c_str();
	slot.data.clip1.String = slot.clip1.empty() ? nullptr : slot.clip1.c_str();

	slot.data.dict2.String = slot.dict2.empty() ? nullptr : slot.dict2.c_str();
	slot.data.clip2.String = slot.clip2.empty() ? nullptr : slot.clip2.c_str();
}

static ScriptInitSlotStorage MakeScriptInitSlotDataFromObject(const fx::scrObject& object)
{
	ScriptInitSlotStorage slot;
	FillDefaultScriptInitSlotData(slot);

	if (!object.data || object.length == 0)
	{
		return slot;
	}

	try
	{
		auto unpacked = msgpack::unpack(object.data, object.length);
		auto data = unpacked->as<std::map<std::string, msgpack::object>>();

		if (data.empty())
		{
			return slot;
		}

		slot.data.state.Int = GetIntOrDefault(data, "state", 0);

		slot.dict0 = GetStringOrDefault(data, "dict0");
		slot.clip0 = GetStringOrDefault(data, "clip0");
		slot.data.phase0.Float = GetFloatOrDefault(data, "phase0", 0.0f);
		slot.data.rate0.Float = GetFloatOrDefault(data, "rate0", 1.0f);
		slot.data.weight0.Float = GetFloatOrDefault(data, "weight0", 1.0f);

		slot.dict1 = GetStringOrDefault(data, "dict1");
		slot.clip1 = GetStringOrDefault(data, "clip1");
		slot.data.phase1.Float = GetFloatOrDefault(data, "phase1", 0.0f);
		slot.data.rate1.Float = GetFloatOrDefault(data, "rate1", 0.0f);
		slot.data.weight1.Float = GetFloatOrDefault(data, "weight1", 0.0f);

		slot.dict2 = GetStringOrDefault(data, "dict2");
		slot.clip2 = GetStringOrDefault(data, "clip2");
		slot.data.phase2.Float = GetFloatOrDefault(data, "phase2", 0.0f);
		slot.data.rate2.Float = GetFloatOrDefault(data, "rate2", 0.0f);
		slot.data.weight2.Float = GetFloatOrDefault(data, "weight2", 0.0f);

		slot.data.filter.Int = GetIntOrDefault(data, "filter", 0);
		slot.data.blendInDuration.Float = GetFloatOrDefault(data, "blendInDuration", 0.125f);
		slot.data.blendOutDuration.Float = GetFloatOrDefault(data, "blendOutDuration", 0.125f);
		slot.data.timeToPlay.Int = GetIntOrDefault(data, "timeToPlay", -1);
		slot.data.flags.Int = GetIntOrDefault(data, "flags", 0);
		slot.data.ikFlags.Int = GetIntOrDefault(data, "ikFlags", 0);
	}
	catch (...)
	{
		// fallback to default empty slot
	}

	return slot;
}

static void RegisterScriptedAnimWrapper(uint64_t nativeHash)
{
	const auto handler = fx::ScriptEngine::GetNativeHandler(nativeHash);
	if (!handler)
	{
		return;
	}

	fx::ScriptEngine::RegisterNativeHandler(nativeHash, [handler](fx::ScriptContext& ctx)
	{
		const uint32_t entity = ctx.GetArgument<uint32_t>(0);
		const fx::scrObject low = ctx.GetArgument<fx::scrObject>(1);
		const fx::scrObject mid = ctx.GetArgument<fx::scrObject>(2);
		const fx::scrObject high = ctx.GetArgument<fx::scrObject>(3);
		const float blendIn = ctx.GetArgument<float>(4);
		const float blendOut = ctx.GetArgument<float>(5);

		ScriptInitSlotStorage lowSlot;
		ScriptInitSlotStorage midSlot;
		ScriptInitSlotStorage highSlot;

		try
		{
			lowSlot = MakeScriptInitSlotDataFromObject(low);
			midSlot = MakeScriptInitSlotDataFromObject(mid);
			highSlot = MakeScriptInitSlotDataFromObject(high);

			RefreshScriptInitSlotPointers(lowSlot);
			RefreshScriptInitSlotPointers(midSlot);
			RefreshScriptInitSlotPointers(highSlot);
		}
		catch (const std::exception& e)
		{
			fx::scripting::Warningf("natives", "Scripted animation wrapper: failed to deserialize msgpack object (%s)\n", e.what());
			return;
		}

		ctx.SetArgument<uint32_t>(0, entity);
		ctx.SetArgument<ScriptInitSlotData*>(1, &lowSlot.data);
		ctx.SetArgument<ScriptInitSlotData*>(2, &midSlot.data);
		ctx.SetArgument<ScriptInitSlotData*>(3, &highSlot.data);
		ctx.SetArgument<float>(4, blendIn);
		ctx.SetArgument<float>(5, blendOut);

		handler(ctx);
	});
}

static void ScriptedAnimNatives()
{
	RegisterScriptedAnimWrapper(0x126EF75F1E17ABE5ULL); // TASK_SCRIPTED_ANIMATION
	RegisterScriptedAnimWrapper(0x77A1EEC547E7FCF1ULL); // PLAY_ENTITY_SCRIPTED_ANIM
}

static InitFunction initFunction([]()
{
	rage::scrEngine::OnScriptInit.Connect([]()
	{
		ScriptedAnimNatives();
	});
});
