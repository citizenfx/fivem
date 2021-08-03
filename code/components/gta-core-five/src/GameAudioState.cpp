#include <StdInc.h>
#include <Hooking.h>

#include <CoreConsole.h>
#include <nutsnbolts.h>

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

	{
		static bool useSynchronousAudio = false;
		static bool lastUseSynchronousAudio = false;

		static auto asynchronousAudio = hook::get_address<bool*>(hook::get_pattern("E8 ? ? ? ? 40 38 35 ? ? ? ? 75 05", 8));
		static auto audioTimeout = hook::get_address<int*>(hook::get_pattern("8B 15 ? ? ? ? 41 03 D6 3B", 2));

		OnGameFrame.Connect([]()
		{
			if (useSynchronousAudio != lastUseSynchronousAudio)
			{
				if (useSynchronousAudio)
				{
					*asynchronousAudio = false;
					*audioTimeout = 0;
				}
				else
				{
					*asynchronousAudio = true;
					*audioTimeout = 1000;
				}

				lastUseSynchronousAudio = useSynchronousAudio;
			}
		});

		static ConVar<bool> audUseFrameLimiter("game_useSynchronousAudio", ConVar_Archive, false, &useSynchronousAudio);
	}

	static ConVar<bool> audUseFrameLimiter("game_useAudioFrameLimiter", ConVar_Archive, true, rage::g_audUseFrameLimiter);
});
