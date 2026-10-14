#include <StdInc.h>

#include <CoreConsole.h>
#include <CustomText.h>
#include <DrawCommands.h>
#include <FrameRateLimiter.h>
#include <Hooking.h>
#include <nutsnbolts.h>
#include <scrEngine.h>

// GameProfileSettings.cpp
int* GetProfilePref(uint32_t prefHash);

// the unused PREF_VID_STEREO_SEPARATION pref backs the graphics settings entry, pausemenu.xml needs one
// MENU_OPTION_GFX_WATER display value per entry
static constexpr int g_fpsLimitValues[] = { 0, 30, 40, 50, 60, 75, 90, 120, 144, 165, 240 };
static constexpr int g_fpsLimitCount = int(std::size(g_fpsLimitValues));

static auto& GetFpsLimitVar()
{
	static ConVar<int> var("game_fpsLimit", ConVar_Archive | ConVar_UserPref, 0);
	return var;
}

// limits below the lowest preset make the game barely usable
static int NormalizeFpsLimit(int fpsLimit)
{
	if (fpsLimit <= 0)
	{
		return 0;
	}

	return std::max(fpsLimit, g_fpsLimitValues[1]);
}

static int GetFpsLimitPrefValue(int fpsLimit)
{
	if (fpsLimit <= 0)
	{
		return 0;
	}

	// values set from the console might not be in the list, pick the closest one to display
	int closest = 1;

	for (int i = 2; i < g_fpsLimitCount; i++)
	{
		if (std::abs(g_fpsLimitValues[i] - fpsLimit) < std::abs(g_fpsLimitValues[closest] - fpsLimit))
		{
			closest = i;
		}
	}

	return closest;
}

static int UpdateFpsLimit()
{
	static int* pref = nullptr;
	static int lastFpsLimit = -1;
	static int lastPrefValue = -1;
	static bool prefOutdated = false;

	int fpsLimit = NormalizeFpsLimit(GetFpsLimitVar().GetValue());

	if (!pref)
	{
		pref = GetProfilePref(HashRageString("PREF_VID_STEREO_SEPARATION"));
	}

	if (pref)
	{
		int prefValue = *pref;
		bool prefValid = (prefValue >= 0 && prefValue < g_fpsLimitCount);

		// changed from the pause menu
		if (lastPrefValue != -1 && prefValue != lastPrefValue && fpsLimit == lastFpsLimit && prefValid)
		{
			fpsLimit = g_fpsLimitValues[prefValue];
			GetFpsLimitVar().GetHelper()->SetRawValue(fpsLimit);
			prefOutdated = false;
		}
		else if (lastPrefValue == -1 || fpsLimit != lastFpsLimit || !prefValid || prefOutdated)
		{
			// an open pause menu doesn't redraw the entry when the pref changes, wait for it to be closed
			prefOutdated = (lastPrefValue != -1 && prefValid && NativeInvoke::Invoke<0xB0034A223497FFCB, bool>()); // IS_PAUSE_MENU_ACTIVE

			if (!prefOutdated)
			{
				*pref = GetFpsLimitPrefValue(fpsLimit);
			}
		}

		lastPrefValue = *pref;
	}

	lastFpsLimit = fpsLimit;
	return fpsLimit;
}

static HookFunction hookFunction([]()
{
	game::AddCustomText("CFX_FPS_LIMIT", "Frame Rate Limit");

	for (int i = 1; i < g_fpsLimitCount; i++)
	{
		game::AddCustomText(fmt::sprintf("CFX_FPS_LIMIT_%d", i), fmt::sprintf("%d FPS", g_fpsLimitValues[i]));
	}

	// wait before input gets polled (like AMD Anti-Lag 2 in rage-input-five) rather than after presenting, to keep input latency low
	OnGameFrame.Connect([]()
	{
		static FrameRateLimiter limiter;

		int fpsLimit = UpdateFpsLimit();

		// vsync already caps the frame rate, pacing on top of it would only cause stutter
		if (int vsyncFrameRate = GfxGetVsyncFrameRate(); vsyncFrameRate > 0 && fpsLimit >= vsyncFrameRate)
		{
			fpsLimit = 0;
		}

		limiter.Wait(fpsLimit);
	},
	INT32_MIN);
});
