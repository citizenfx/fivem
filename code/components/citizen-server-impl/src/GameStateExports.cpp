#include <StdInc.h>
#include <GameServer.h>

#include <GameBuilds.h>
#include <charconv>

static bool g_bigMode;
static bool g_lengthHack;
static bool(*g_onesync)();

extern fx::GameBuild g_enforcedGameBuild;

namespace fx
{
bool IsBigMode()
{
	return g_bigMode;
}

bool IsLengthHack()
{
	return g_lengthHack;
}

bool IsOneSync()
{
	if (!g_onesync)
	{
		return false;
	}

	return g_onesync();
}

void SetOneSyncGetCallback(bool (*cb)())
{
	g_onesync = cb;
}

void SetBigModeHack(bool bigMode, bool lengthHack)
{
	g_bigMode = bigMode;
	g_lengthHack = lengthHack;
}

std::string_view GetEnforcedGameBuild()
{
	return g_enforcedGameBuild;
}

int GetEnforcedGameBuildNumber()
{
	int build = 0;
	auto buildNum = GetEnforcedGameBuild();
	std::from_chars(buildNum.data(), buildNum.data() + buildNum.size(), build);

	return build;
}
}
