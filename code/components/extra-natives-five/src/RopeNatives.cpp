#include "StdInc.h"

#include "ropeManager.h"

#include <Hooking.h>
#include <ScriptEngine.h>
#include <ScriptSerialization.h>

namespace rage
{
static ropeDataManager* g_ropeDataManager;

ropeDataManager* ropeDataManager::Get()
{
	return g_ropeDataManager;
}

static hook::thiscall_stub<rope*(ropeManager* manager, uint32_t handle)> ropeManager_getRopeFromHandle([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 4C 8B E8 48 89 45 67"));
});

rope* ropeManager::GetRopeFromHandle(uint32_t handle)
{
	return ropeManager_getRopeFromHandle(this, handle);
}

static ropeManager** g_ropeManager;

ropeManager* ropeManager::Get()
{
	return (g_ropeManager) ? *g_ropeManager : nullptr;
}
}

static HookFunction hookFunction([]()
{
	{
		auto location = hook::get_pattern("48 8B 05 ? ? ? ? 48 8B 0C F8 48 89 71 18", 0x3);
		auto address = hook::get_address<uintptr_t>(location) - 0x8;
		rage::g_ropeDataManager = reinterpret_cast<rage::ropeDataManager*>(address);
	}

	{
		auto location = hook::get_pattern("8B 91 ? ? ? ? 48 8B 0D ? ? ? ? E8 ? ? ? ? 33 DB", 0x9);
		rage::g_ropeManager = hook::get_address<rage::ropeManager**>(location);
	}

	fx::ScriptEngine::RegisterNativeHandler("GET_ALL_ROPES", [](fx::ScriptContext& context)
	{
		std::vector<uint32_t> handles;

		if (auto manager = rage::ropeManager::Get())
		{
			if (manager->numAllocated > 0)
			{
				auto rope = manager->allocated.head;
				while (rope)
				{
					handles.push_back(rope->handle);
					rope = rope->nextRope;
				}
			}
		}

		context.SetResult(fx::SerializeObject(handles));
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_ROPE_LENGTH_CHANGE_RATE", [](fx::ScriptContext& context)
	{
		int handle = context.GetArgument<int>(0);
		float lengthChangeRate = 0.0f;

		if (auto manager = rage::ropeManager::Get())
		{
			if (auto rope = manager->GetRopeFromHandle(handle))
			{
				lengthChangeRate = rope->lengthChangeRate;
			}
		}

		context.SetResult(lengthChangeRate);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_ROPE_LENGTH_CHANGE_RATE", [](fx::ScriptContext& context)
	{
		int handle = context.GetArgument<int>(0);
		float lengthChangeRate = context.GetArgument<float>(1);

		if (auto manager = rage::ropeManager::Get())
		{
			if (auto rope = manager->GetRopeFromHandle(handle))
			{
				rope->lengthChangeRate = lengthChangeRate;
			}
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_ROPE_TIME_MULTIPLIER", [](fx::ScriptContext& context)
	{
		int handle = context.GetArgument<int>(0);
		float multiplier = 0.0f;

		if (auto manager = rage::ropeManager::Get())
		{
			if (auto rope = manager->GetRopeFromHandle(handle))
			{
				multiplier = rope->timeMultiplier;
			}
		}

		context.SetResult(multiplier);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_ROPE_FLAGS", [](fx::ScriptContext& context)
	{
		int handle = context.GetArgument<int>(0);
		rage::eRopeFlags flags{};

		if (auto manager = rage::ropeManager::Get())
		{
			if (auto rope = manager->GetRopeFromHandle(handle))
			{
				flags = rope->flags;
			}
		}

		context.SetResult(flags);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_ROPE_UPDATE_ORDER", [](fx::ScriptContext& context)
	{
		int handle = context.GetArgument<int>(0);
		uint32_t updateOrder = 0;

		if (auto manager = rage::ropeManager::Get())
		{
			if (auto rope = manager->GetRopeFromHandle(handle))
			{
				updateOrder = rope->updateOrder;
			}
		}

		context.SetResult(updateOrder);
	});
});
