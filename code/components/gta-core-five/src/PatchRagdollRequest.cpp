#include <StdInc.h>

#include <Hooking.h>

#include <GameInit.h>
#include <CoreConsole.h>

//
// CRagdollRequestEvent should only be triggered on a player ped when
// CClonedNMControlInfo exists in CTaskTreeClone. This patch filters out the
// illegitimate case where CTaskNMControl does not exist when handling
// CRagdollRequestEvent.
//
// The filtering is placed behind the replicated 'game_sanitizeRagdollEvents'
// convar to allow users to test for regressions which may only be noticeable
// around NMTask desynchronization.
//
static bool g_sanitizeRagdollEvents = false;

// #TODO: Eventually unify with EntitySystem

namespace rage
{
	class fwEntity;
	class aiTaskTree
	{
	public:
		bool FindActiveTaskByType(int taskIndex);
	};
}

struct eTaskType
{
	inline static int32_t TASK_NM_CONTROL;
};

class CPedIntelligence
{
public:
	inline static ptrdiff_t kActiveTreeOffset;
	inline static ptrdiff_t kMotionTreeOffset;

private:
	inline rage::aiTaskTree* GetTaskTree(ptrdiff_t offset)
	{
		auto location = reinterpret_cast<char*>(this) + offset;
		return *reinterpret_cast<rage::aiTaskTree**>(location);
	}

	inline bool HasTaskByType(ptrdiff_t offset, int32_t taskIndex)
	{
		if (auto tree = GetTaskTree(offset))
		{
			return tree->FindActiveTaskByType(taskIndex);
		}
		return false;
	}

public:
	inline bool HasActiveTaskByType(int32_t taskIndex)
	{
		return HasTaskByType(kActiveTreeOffset, taskIndex);
	}

	inline bool HasMotionTaskByType(int32_t taskIndex)
	{
		return HasTaskByType(kMotionTreeOffset, taskIndex);
	}
};

class CPed
{
public:
	inline static ptrdiff_t kPlayerFlagsOffset;
	inline static ptrdiff_t kPedIntelligenceOffset;

public:
	inline CPedIntelligence* GetIntelligence()
	{
		auto location = reinterpret_cast<char*>(this) + kPedIntelligenceOffset;
		return *reinterpret_cast<CPedIntelligence**>(location);
	}

	inline bool IsLocalPlayer()
	{
		auto location = reinterpret_cast<char*>(this) + kPlayerFlagsOffset;
		uint32_t bits = *reinterpret_cast<uint32_t*>(location);
		return (bits & 1) == 0 && (bits & 2) != 0;
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

static bool (*g_origCanRagdoll)(CPed*, int32_t, rage::fwEntity*, float);
static bool CTaskNM_CanRagdoll(CPed* ped, int32_t flags, rage::fwEntity* source, float p4)
{
	if (g_sanitizeRagdollEvents && ped->IsLocalPlayer())
	{
		if (auto intelligence = ped->GetIntelligence())
		{
			// IS_PED_RUNNING_RAGDOLL_TASK
			if (!intelligence->HasActiveTaskByType(eTaskType::TASK_NM_CONTROL)
				&& !intelligence->HasMotionTaskByType(eTaskType::TASK_NM_CONTROL))
			{
				return false;
			}
		}
	}

	return g_origCanRagdoll(ped, flags, source, p4);
}

static HookFunction hookFunction([]()
{
	static ConVar<bool> sanitizeRagdollEvents("game_sanitizeRagdollEvents", ConVar_Replicated, false, &g_sanitizeRagdollEvents);

	eTaskType::TASK_NM_CONTROL = *hook::get_pattern<int32_t>("40 84 ED 74 0C 81 FB ? ? ? ? 0F 85", 0x7);
	CPed::kPlayerFlagsOffset = *hook::get_pattern<int32_t>("8A 81 ? ? ? ? A8 01 75 04 A8 02 75 53", 0x2);
	CPedIntelligence::kActiveTreeOffset = *hook::get_pattern<int32_t>("48 85 C0 0F 95 C0 C0 E0 07 08 83", 0x16 + 0x3);

	{
		auto location = hook::get_pattern<char>("84 C0 0F 84 ? ? ? ? 48 8B 82 ? ? ? ? BA");
		CPed::kPedIntelligenceOffset = *reinterpret_cast<int32_t*>(location + 0xB);
		CPedIntelligence::kMotionTreeOffset = *reinterpret_cast<int32_t*>(location + 0x17);
	}

	{
		auto location = hook::get_pattern("45 33 C0 48 8B ? 41 8D 50 1C E8", 0xA);
		hook::set_call(&g_origCanRagdoll, location);
		hook::call(location, CTaskNM_CanRagdoll);
	}
});
