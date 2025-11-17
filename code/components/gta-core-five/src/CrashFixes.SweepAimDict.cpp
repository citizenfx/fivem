#include "StdInc.h"

#include "Hooking.FlexStruct.h"
#include "Hooking.Patterns.h"
#include "Hooking.Stubs.h"

static hook::cdecl_stub<bool(int)> fwAnimManager_IsValidSlot([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 84 C0 74 ? 8B D3 48 8B CF E8 ? ? ? ? FF C3"));
});

static int32_t clipDictIdOffset;

static int (*g_origTaskGeneralSweepOnUpdate)(hook::FlexStruct*);
static int TaskGeneralSweepOnUpdate(hook::FlexStruct* task)
{
	int clipDictId = task->Get<int>(clipDictIdOffset);

	if (!fwAnimManager_IsValidSlot(clipDictId))
	{
		return 0x2; // FSM_QUIT
	}

	return g_origTaskGeneralSweepOnUpdate(task);
}

static HookFunction hookFunction([]()
{
	// This hook prevents a client crash caused by invalid clip dictionary IDs in SweepAim tasks.
	// When a player triggers TaskSweepAimEntity using a clip dictionary that exists only on their
	// local client (e.g., a modded or unloaded anim dict), the task is replicated to remote clients.
	// During replication, the clipDictId may resolve to an invalid local index on other clients.
	// Later, the CTaskGeneralSweep task uses this invalid clipDictId inside fwAnimManager, which
	// eventually reaches fwClipDictionaryStore::GetNumRefs and tries to access:
	//     m_aStorage[index * size]
	// If 'index' is invalid, this causes an out-of-range pointer dereference, leading to a client
	// crash.
	g_origTaskGeneralSweepOnUpdate = hook::trampoline(hook::get_pattern("EB ? 8B 93 ? ? ? ? 48 81 C1", -0x2F), TaskGeneralSweepOnUpdate);
	clipDictIdOffset = *hook::get_pattern<int32_t>("8B 93 ? ? ? ? 48 81 C1 ? ? ? ? E8 ? ? ? ? 84 C0", 0x2);
});
