#include <StdInc.h>

#include <jitasm.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>

static int32_t forceReevalOffset = 0;
static int32_t condAnimGroupOffset = 0;
static uint8_t targetFlagValue = 0;

int (*g_NoClipPlayingFSMUpdateOrig)(hook::FlexStruct* task);
int NoClipPlayingFSMUpdate(hook::FlexStruct* task)
{
	uint8_t flags = task->Get<uint8_t>(forceReevalOffset);
	if (flags & targetFlagValue)
	{
		void* condAnimGroupPtr = task->Get<void*>(condAnimGroupOffset);
		if (!condAnimGroupPtr)
		{
			// TODO: Remove log after approve from players that crash fixed
			trace("Crash was possible here earlier. Seems to be fixed\n");

			flags ^= targetFlagValue;
			task->Set<uint8_t>(forceReevalOffset, flags);
		}
	}

	return g_NoClipPlayingFSMUpdateOrig(task);
}

static HookFunction hookFunction([]
{
	auto checkReevalFlagLocation = hook::get_pattern<uint8_t>("F6 83 ? ? ? ? ? 74 ? 0F B7 8B");
	forceReevalOffset = *(int32_t*)(checkReevalFlagLocation + 2);
	targetFlagValue = *(checkReevalFlagLocation + 6);
	condAnimGroupOffset = *hook::get_pattern<int32_t>("48 8B 83 ? ? ? ? 0F BF C9", 3);

	auto noClipPlayingFSMUpdateAddr = hook::get_pattern<void>("48 8B C4 48 89 58 ? 55 56 57 41 54 41 56 48 8D 68 ? 48 81 EC ? ? ? ? 48 8B 71");

	g_NoClipPlayingFSMUpdateOrig = hook::trampoline(noClipPlayingFSMUpdateAddr, &NoClipPlayingFSMUpdate);
});
