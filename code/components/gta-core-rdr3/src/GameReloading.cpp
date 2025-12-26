#include <StdInc.h>

#include <CoreConsole.h>

#include <GameInit.h>
#include <Error.h>

#include <MinHook.h>
#include <Hooking.h>

//
// Game reload code flow:
// 1. GameInit::KillNetwork
// 2. Triggers OnKillNetwork, where we'll set loading screen override and set the init state to 'reload save'.
// 3. Game will execute session shutdown, which we intercept with a blocking (busy-ish) loop at the end,
//    and triggering events before and after.
// 4. When GameInit::ReloadGame is called, we'll return and let the game finish reloading.
//

bool g_setLoadingScreens;
bool g_shouldKillNetwork;
bool g_isNetworkKilled;

// 1207.80
// instead of setting init state
static hook::cdecl_stub<void(bool)> _newGame([]()
{
	return hook::get_call(hook::get_pattern("33 C9 E8 ? ? ? ? B1 01 E8 ? ? ? ? 83 63 08 00", 9));
});

static hook::cdecl_stub<void(int)> setupLoadingScreens([]()
{
	return hook::get_pattern("83 FA 01 75 ? 8B 05 ? ? ? ? 8A CA 89 05", -0x3A);
});

static hook::cdecl_stub<void()> loadingScreenUpdate([]()
{
	return hook::get_pattern("74 53 48 83 64 24 20 00 4C 8D", -0x31);
});

static hook::cdecl_stub<void()> lookAlive([]()
{
	return hook::get_pattern("40 8A FB 38 1D", -0x29);
});


enum GameInitState : int
{
	STATE_START = 0,
	STATE_BEFORE_INIT = 10,
	STATE_LOAD_GAME = 15,
	STATE_POST_INIT = 21
};

int* g_initState;
bool g_isInInitLoop;
static bool g_shouldReloadGame;

static const char* typeMap[] = {
	nullptr,
	"system",
	"beforeMapLoaded",
	nullptr,
	"afterMapLoaded",
	nullptr,
	nullptr,
	nullptr,
	"session"
};

struct InitFunctionStub: public jitasm::Frontend
{
	static uintptr_t LogStub(uintptr_t stub, int type)
	{
		trace("Running shutdown %s function: %p\n", typeMap[type], (void*)hook::get_unadjusted(stub));
		return stub;
	}

	virtual void InternalMain() override
	{
		imul(rdx, 0x38);

		push(r14);

		mov(rcx, qword_ptr[rax + rdx + 8]);
		mov(edx, r14d);

		// scratch space!
		sub(rsp, 0x20);

		mov(rax, (uintptr_t)LogStub);
		call(rax);

		mov(ecx, r14d);
		call(rax);

		add(rsp, 0x20);
		pop(r14);
		ret();
	}
};

static void (*g_origRunInitState)();
static void WrapRunInitState()
{
	if (g_setLoadingScreens)
	{
		setupLoadingScreens(3);
		loadingScreenUpdate();

		g_setLoadingScreens = false;
	}

	if (g_shouldReloadGame)
	{
		Instance<ICoreGameInit>::Get()->OnGameRequestLoad();
		*g_initState = STATE_LOAD_GAME;
		g_shouldReloadGame = false;
	}

	if (g_shouldKillNetwork)
	{
		trace("Killing network, stage 2...\n");

		_newGame(false);

		g_shouldKillNetwork = false;
	}

	if (!g_isNetworkKilled)
	{
		g_origRunInitState();
	}
}

static bool (*g_origSkipInit)(int);
static bool WrapSkipInit(int a1)
{
	if (g_isNetworkKilled)
	{
		if (g_origSkipInit(a1))
		{
			printf("");
		}

		return false;
	}

	return g_origSkipInit(a1);
}

static void* g_renderThreadInterface;
static hook::cdecl_stub<void()> runCriticalSystemServicing([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? BA 32 00 00 00 48 8D 0D ? ? ? ? E8", 22));
});

static hook::cdecl_stub<void(void*, char)> flushRenderThread([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 84 DB 74 71"));
});

static void WaitForInitLoop()
{
	// run our loop
	g_isInInitLoop = true;

	while (*g_initState <= STATE_BEFORE_INIT)
	{
		runCriticalSystemServicing();
		lookAlive();
		flushRenderThread(g_renderThreadInterface, 0);

		if (*g_initState <= STATE_BEFORE_INIT)
		{
			Sleep(15);
		}
	}
}

static hook::cdecl_stub<void()> g_criticalTest([]()
{
	return hook::get_pattern("48 83 EC 28 E8 ? ? ? ? E8 ? ? ? ? 84 C0");
});

static void (*g_shutdownSession)();
void ShutdownSessionWrap()
{
	Instance<ICoreGameInit>::Get()->SetVariable("gameKilled");
	Instance<ICoreGameInit>::Get()->SetVariable("shutdownGame");

	g_isNetworkKilled = true;
	*g_initState = STATE_LOAD_GAME;

	AddCrashometry("kill_network_game", "true");

	OnKillNetworkDone();

	g_shutdownSession();

	Instance<ICoreGameInit>::Get()->OnShutdownSession();

	g_shouldKillNetwork = false;

	while (g_isNetworkKilled)
	{
		runCriticalSystemServicing();
		lookAlive();
		flushRenderThread(g_renderThreadInterface, 0);
	}

	Instance<ICoreGameInit>::Get()->OnGameRequestLoad();
}

static void UpdateInitState(int state)
{
#if _DEBUG
	trace("UpdateInitState %d -> %d\n", *g_initState, state);
#endif
	WaitForInitLoop();
	*g_initState = state;
}

static int Return2()
{
	return 2;
}

static HookFunction hookFunction([]()
{
	// never try to reload SP save
	//hook::jump(hook::get_pattern("48 8B 03 48 8B CB FF 50 38 48 8B C8 E8 ? ? ? ? F6 D8", -0x12), Return0);
	hook::jump(hook::get_pattern("84 C0 8D 4B 02 0F 44 D9", -0x10), Return2);

	// don't try the SP transition if we're init type 15
	hook::put<uint8_t>(hook::get_pattern("83 F8 0F 75 58 8B 4B 0C", 3), 0xEB);

	g_renderThreadInterface = hook::get_address<void*>(hook::get_pattern("E8 ? ? ? ? 84 DB 74 71", -4));

	// shutdown session wrapper
	{
		MH_Initialize();
		MH_CreateHook(hook::get_pattern("41 B9 13 00 00 00 45 33 C0 33 D2", -0x51), ShutdownSessionWrap, (void**)&g_shutdownSession);
		MH_EnableHook(MH_ALL_HOOKS);
	}

	static struct : jitasm::Frontend
	{
		virtual void InternalMain() override
		{
			sub(rsp, 0x20);

			push(rsi);
			mov(rsi, (uintptr_t)UpdateInitState);
			call(rsi);
			pop(rsi);

			add(rsp, 0x20);
			ret();
		}
	} updateInitStateStub;

	{
		// NOP any code that sets 
		auto loc = hook::get_call(hook::get_call(hook::get_pattern("3B C8 41 0F 45 C0 E9", 6)));
		// nop the right pointer
		hook::nop(loc, 6);
		// and call our internal loop function from there
		hook::jump(loc, updateInitStateStub.GetCode());
	}

	// fwApp 2:1 state handler (loaded game), before running init machine
	{
		auto loc = hook::get_pattern<char>("40 8A DE 84 C0 74 02 B3 01 E8");

		hook::set_call(&g_origRunInitState, loc + 9);
		hook::call(loc + 9, WrapRunInitState);

		hook::set_call(&g_origSkipInit, loc - 5);
		hook::call(loc - 5, WrapSkipInit);

		g_initState = hook::get_address<int*>(loc - 25);
	}

	// Init function #1
	{
		static InitFunctionStub initFunctionStub;
		initFunctionStub.Assemble();

		auto location = hook::get_pattern<char>("41 8B CE 48 6B D2 38 FF 54 02 08");
		hook::nop(location, 11);
		hook::call_rcx(location, initFunctionStub.GetCode());
	}
});
