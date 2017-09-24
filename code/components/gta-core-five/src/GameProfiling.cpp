#include <StdInc.h>
#include <Hooking.h>

#include <Brofiler.h>

static bool(*rageInitGame)(int argc, const char** argv);
static bool(**rageGameLoop)();
static void(*rageShutdownGame)();

int GameMain(int argc, const char** argv)
{
	if (rageInitGame(argc, argv))
	{
		while (true)
		{
			BROFILER_FRAME("Main");

			if (!(*rageGameLoop)())
			{
				break;
			}
		}

		rageShutdownGame();
	}

	return 0;
}

static HookFunction hookFunction([]()
{
	// RAGE main loop
	{
		auto location = hook::get_pattern<char>("84 C0 74 0F FF 15 ? ? ? ? 84 C0 75 F6 E8", -9);

		hook::set_call(&rageInitGame, location + 4);
		rageGameLoop = hook::get_address<decltype(rageGameLoop)>(location + 15);
		hook::set_call(&rageShutdownGame, location + 0x17);

		hook::jump(location, GameMain);
	}
});
