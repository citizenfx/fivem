#include <StdInc.h>

#include <CoreConsole.h>
#include <CustomText.h>
#include <DrawCommands.h>
#include <FrameRateLimiter.h>
#include <Hooking.h>
#include <nutsnbolts.h>
#include <scrEngine.h>

int* GetProfilePref(uint32_t prefHash);

static constexpr int g_fpsLimitValues[] = { 0, 30, 40, 50, 60, 75, 90, 120, 144, 165, 240 };
static constexpr int g_fpsLimitCount = int(std::size(g_fpsLimitValues));

static auto& GetFpsLimitVar()
{
	static ConVar<int> var("game_fpsLimit", ConVar_Archive | ConVar_UserPref, 0);
	return var;
}

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

		if (lastPrefValue != -1 && prefValue != lastPrefValue && fpsLimit == lastFpsLimit && prefValid)
		{
			fpsLimit = g_fpsLimitValues[prefValue];
			GetFpsLimitVar().GetHelper()->SetRawValue(fpsLimit);
			prefOutdated = false;
		}
		else if (lastPrefValue == -1 || fpsLimit != lastFpsLimit || !prefValid || prefOutdated)
		{
			prefOutdated = (lastPrefValue != -1 && prefValid && NativeInvoke::Invoke<0xB0034A223497FFCB, bool>());

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

	OnGameFrame.Connect([]()
	{
		static FrameRateLimiter limiter;

		int fpsLimit = UpdateFpsLimit();

		if (int vsyncFrameRate = GfxGetVsyncFrameRate(); vsyncFrameRate > 0 && fpsLimit >= vsyncFrameRate)
		{
			fpsLimit = 0;
		}

		limiter.Wait(fpsLimit);
	},
	INT32_MIN);
});
