#include <StdInc.h>
#include <Hooking.h>

#include <MinHook.h>

#include <optick.h>

namespace rage
{
	static bool(*CommonMain_Prologue)(int argc, const char** argv);
	static bool(**g_pProjectMainOrDoOneLoop)();
	static void(*CommonMain_Epilogue)();

	int CommonMain(int argc, const char** argv)
	{
		if (CommonMain_Prologue(argc, argv))
		{
			while (true)
			{
				OPTICK_FRAME("Main");

				if (!(*g_pProjectMainOrDoOneLoop)())
				{
					break;
				}
			}

			CommonMain_Epilogue();
		}

		return 0;
	}
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

		hook::set_call(&rage::CommonMain_Prologue, location + 4);
		rage::g_pProjectMainOrDoOneLoop = hook::get_address<decltype(rage::g_pProjectMainOrDoOneLoop)>(location + 15);
		hook::set_call(&rage::CommonMain_Epilogue, location + 0x17);

		hook::jump(location, rage::CommonMain);
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
