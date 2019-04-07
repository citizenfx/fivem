#include <StdInc.h>
#include <Hooking.h>

#include <MinHook.h>

#include <optick.h>

static bool(*rageInitGame)(int argc, const char** argv);
static bool(**rageGameLoop)();
static void(*rageShutdownGame)();

int GameMain(int argc, const char** argv)
{
	if (rageInitGame(argc, argv))
	{
		while (true)
		{
			OPTICK_FRAME("Main");

			if (!(*rageGameLoop)())
			{
				break;
			}
		}

		rageShutdownGame();
	}

	return 0;
}

static void(*g_origRenderFrame)(void*);

static void RenderFrame(void* a1)
{
#if USE_OPTICK
	static ::Optick::ThreadScope mainThreadScope("Render");
	::Optick::Event OPTICK_CONCAT(autogen_event_, __LINE__)(*::Optick::GetFrameDescription(::Optick::FrameType::Render));
#endif

	g_origRenderFrame(a1);
}

static void(*g_waitSema)(void* sema, int count);

static void WaitRenderSema(void* sema, int count)
{
	OPTICK_CATEGORY(OPTICK_FUNC, Optick::Category::Wait);

	g_waitSema(sema, count);
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

	// render thread
	{
		MH_Initialize();
		MH_CreateHook(hook::get_pattern("41 57 48 83 EC 20 48 8D 99 50 04", -0x12), RenderFrame, (void**)&g_origRenderFrame);
		MH_EnableHook(MH_ALL_HOOKS);
	}

	// render thread wait sema
	{
		auto location = hook::get_pattern("48 8B 4F 40 BA 01 00 00 00 48 8B D8", 12);

		hook::set_call(&g_waitSema, location);
		hook::call(location, WaitRenderSema);
	}
});
