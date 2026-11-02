#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>

struct TaskSlotData
{
	uint32_t taskType;
	uint32_t taskPriority;
	uint32_t taskTreeDepth;
	uint32_t taskSequenceId;
	bool taskActive;
};

static constexpr int kNumTaskSlots = 8;
static constexpr uint32_t kMaxTaskPriority = 5;
static constexpr uint32_t kMaxTreeDepth = 8;

static int32_t g_taskSlotsOffset;
static uint32_t g_noTaskType;
static std::once_flag g_traceOnce;

static void (*g_CNetObjPed_SetTaskTreeData)(void*, void*);

static void CNetObjPed_SetTaskTreeData(void* pedNodeData, void* taskTreeNode)
{
	auto slots = reinterpret_cast<TaskSlotData*>(reinterpret_cast<char*>(taskTreeNode) + g_taskSlotsOffset);
	bool seen[kMaxTaskPriority][kMaxTreeDepth] = {};

	for (int i = 0; i < kNumTaskSlots; i++)
	{
		auto& slot = slots[i];

		if (slot.taskType == g_noTaskType || slot.taskPriority >= kMaxTaskPriority || slot.taskTreeDepth >= kMaxTreeDepth)
		{
			continue;
		}

		if (seen[slot.taskPriority][slot.taskTreeDepth])
		{
			std::call_once(g_traceOnce, []()
			{
				trace("Dropped duplicate task tree slot in CPedTaskTreeDataNode\n");
			});

			slot.taskType = g_noTaskType;
			continue;
		}

		seen[slot.taskPriority][slot.taskTreeDepth] = true;
	}

	g_CNetObjPed_SetTaskTreeData(pedNodeData, taskTreeNode);
}

static HookFunction hookFunction([]()
{
	auto location = hook::get_pattern<char>("48 8D 9A ? ? ? ? 89 46 ? 44 88 46 ? 48 2B FA BD 08 00 00 00 8B 94 1F ? ? ? ? 81 FA");

	g_taskSlotsOffset = *reinterpret_cast<int32_t*>(location + 3) - offsetof(TaskSlotData, taskActive);
	g_noTaskType = *reinterpret_cast<uint32_t*>(location + 31);

	g_CNetObjPed_SetTaskTreeData = hook::trampoline(hook::get_pattern("48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 48 89 78 ? 41 56 48 83 EC ? 48 8B 81 ? ? ? ? 48 8B F9"), CNetObjPed_SetTaskTreeData);
});
