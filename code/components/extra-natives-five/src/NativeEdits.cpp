#include <StdInc.h>
#include <GameInit.h>

#include <Hooking.h>

#include <ScriptEngine.h>
#include <scrEngine.h>

namespace rage
{
class aiTaskTree
{
public:
	bool FindActiveTaskByType(int taskIndex);
};
};

class CPedIntelligence
{
public:
	inline static ptrdiff_t kMotionTreeOffset;
	inline static ptrdiff_t kPedIntelligenceOffset;

public:
	inline rage::aiTaskTree* GetMotionTaskTree()
	{
		auto location = reinterpret_cast<char*>(this) + kMotionTreeOffset;
		return *reinterpret_cast<rage::aiTaskTree**>(location);
	}

	inline bool HasMotionTaskByType(int32_t taskIndex)
	{
		if (auto tree = GetMotionTaskTree())
		{
			return tree->FindActiveTaskByType(taskIndex);
		}
		return false;
	}

	// Utility function that should eventually be moved to CPed
	static inline CPedIntelligence* GetIntelligence(CPed* ped)
	{
		auto location = reinterpret_cast<char*>(ped) + kPedIntelligenceOffset;
		return *reinterpret_cast<CPedIntelligence**>(location);
	}
};

static hook::cdecl_stub<bool(rage::aiTaskTree*, int)> _findTaskByType([]()
{
	return hook::get_pattern("83 79 10 FF 74 20 48 63 41 10 48 8B 4C C1");
});

bool rage::aiTaskTree::FindActiveTaskByType(int taskIndex)
{
	return _findTaskByType(this, taskIndex);
}

// GH-2401: Have GET_IS_TASK_ACTIVE search the motion-tree for TASK_COMBAT_ROLL.
// CTaskCombatRoll is used only as a subtask of CTaskMotionAiming which will not
// be exposed via GET_IS_TASK_ACTIVE.
static void EditIsTaskActive()
{
	constexpr uint64_t GET_IS_TASK_ACTIVE = 0xB0760331C7AA4155;

	const auto handler = fx::ScriptEngine::GetNativeHandler(GET_IS_TASK_ACTIVE);
	if (!handler)
	{
		return;
	}

	fx::ScriptEngine::RegisterNativeHandler(GET_IS_TASK_ACTIVE, [handler](fx::ScriptContext& ctx)
	{
		constexpr int32_t TASK_COMBAT_ROLL = 0x3;
		uint32_t pedHandle = ctx.GetArgument<uint32_t>(0);
		uint32_t taskIndex = ctx.GetArgument<uint32_t>(1);

		handler(ctx);
		if (taskIndex != TASK_COMBAT_ROLL || ctx.GetResult<bool>())
		{
			return;
		}

		// Could not find TASK_COMBAT_ROLL check the peds motion task tree.
		auto ped = rage::fwScriptGuid::GetBaseFromGuid(pedHandle);
		if (ped && ped->IsOfType<CPed>())
		{
			if (auto intelligence = CPedIntelligence::GetIntelligence(reinterpret_cast<CPed*>(ped)))
			{
				if (intelligence->HasMotionTaskByType(TASK_COMBAT_ROLL))
				{
					ctx.SetResult<bool>(true);
				}
			}
		}
	});
}

static bool* g_ropesCreateNetworkWorldState;

static HookFunction hookFunction([]()
{
	{
		auto location = hook::get_pattern<char>("84 C0 0F 84 ? ? ? ? 48 8B 82 ? ? ? ? BA");
		CPedIntelligence::kPedIntelligenceOffset = *reinterpret_cast<int32_t*>(location + 0xB);
		CPedIntelligence::kMotionTreeOffset = *reinterpret_cast<int32_t*>(location + 0x17);
	}

	g_ropesCreateNetworkWorldState = (bool*)hook::AllocateStubMemory(1);
	*g_ropesCreateNetworkWorldState = false;

	// replace ADD_ROPE native net game check whether to create CNetworkRopeWorldStateData
	auto location = hook::get_pattern<uint32_t>("80 3D ? ? ? ? ? 74 71 E8 ? ? ? ? 48", 2);
	hook::put<int32_t>(location, (intptr_t)g_ropesCreateNetworkWorldState - (intptr_t)location - 5);

	fx::ScriptEngine::RegisterNativeHandler("SET_ROPES_CREATE_NETWORK_WORLD_STATE", [](fx::ScriptContext& context)
	{
		bool shouldCreate = context.GetArgument<bool>(0);
		*g_ropesCreateNetworkWorldState = shouldCreate;
	});

	OnKillNetworkDone.Connect([]()
	{
		*g_ropesCreateNetworkWorldState = false;
	});

	rage::scrEngine::OnScriptInit.Connect([]()
	{
		EditIsTaskActive();
	});
});
