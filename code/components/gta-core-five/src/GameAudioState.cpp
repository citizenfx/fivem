#include <StdInc.h>
#include <Hooking.h>

#include <CoreConsole.h>

static bool* audioNotFocused;
static int* muteOnFocusLoss; 

bool DLL_EXPORT ShouldMuteGameAudio()
{
	return *audioNotFocused && *muteOnFocusLoss;
}

namespace rage
{
bool* g_audUseFrameLimiter;
}

static HookFunction hookFunction([]()
{
	{
		auto location = hook::get_pattern<char>("75 17 40 38 2D ? ? ? ? 74 0E 39 2D", 5);
		audioNotFocused = hook::get_address<bool*>(location);
		muteOnFocusLoss = hook::get_address<int*>(location + 8);
	}
	
	{
		auto location = hook::get_pattern("80 3D ? ? ? ? 00 0F 84 ? 00 00 00 80 BB", 2);
		rage::g_audUseFrameLimiter = hook::get_address<bool*>(location) + 1;
	}

	static ConVar<bool> audUseFrameLimiter("game_useAudioFrameLimiter", ConVar_Archive, true, rage::g_audUseFrameLimiter);
});
