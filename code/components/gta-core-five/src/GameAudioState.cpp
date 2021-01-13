#include <StdInc.h>
#include <Hooking.h>

static bool* audioNotFocused;
static int* muteOnFocusLoss; 

bool DLL_EXPORT ShouldMuteGameAudio()
{
	return *audioNotFocused && *muteOnFocusLoss;
}

static HookFunction hookFunction([]()
{
	auto location = hook::get_pattern<char>("75 17 40 38 2D ? ? ? ? 74 0E 39 2D", 5);
	audioNotFocused = hook::get_address<bool*>(location);
	muteOnFocusLoss = hook::get_address<int*>(location + 8);
});
