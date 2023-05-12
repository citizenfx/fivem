/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <jitasm.h>
#include "Hooking.h"
#include "Hooking.Stubs.h"

#include <atArray.h>
#include <Pool.h>
#include <sysAllocator.h>

#include <GameInit.h>
#include <nutsnbolts.h>

#include <CoreConsole.h>
#include <console/OptionTokenizer.h>

#include <se/Security.h>

#include <gameSkeleton.h>

#include <Error.h>

#include <LaunchMode.h>
#include <CrossBuildRuntime.h>

static hook::cdecl_stub<void()> lookAlive([] ()
{
	return hook::pattern("48 8D 6C 24 ? 48 81 EC ? 01 00 00 E8").count(1).get(0).get<void>(-0xC);
});

// map init states to cater for additional '7' in one particular digital distribution version
static bool g_isDigitalDistrib = false;
static bool g_triedLoading = false;

static inline int MapInitState(int initState)
{
	if (initState >= 7)
	{
		if (g_isDigitalDistrib)
		{
			initState += 1;
		}
		else if (xbr::IsGameBuild<2060>())
		{
			initState += 1;
		}
	}

	return initState;
}

static int* g_initState;

bool g_shouldSetState;
bool g_isInInitLoop; // to avoid some GFx crash?
static bool g_shouldReloadGame;

static hook::cdecl_stub<void()> runCriticalSystemServicing([]()
{
	return hook::get_call(hook::get_pattern("48 8D 0D ? ? ? ? BA 32 00 00 00 E8", 17));
});

static std::vector<ProgramArguments> g_argumentList;

static void WaitForInitLoop()
{
	// run our loop
	g_isInInitLoop = true;

	while (*g_initState <= MapInitState(6))
	{
		runCriticalSystemServicing();
		lookAlive();

		if (*g_initState <= MapInitState(6))
		{
			Sleep(15);
		}
	}

	//*g_initState = 7;
}

static bool g_launchedGame = false;

static void WaitForInitLoopWrap()
{
	// certain executables may recheck activation after connection, and want to perform this state change after 12 - ignore those cases
	*g_initState = MapInitState(6);

	WaitForInitLoop();
}

volatile bool g_isNetworkKilled;

enum LoadingScreenContext
{
	LOADINGSCREEN_CONTEXT_NONE = 0,
	LOADINGSCREEN_CONTEXT_INTRO_MOVIE = 1,
	LOADINGSCREEN_CONTEXT_LEGALSPLASH = 2,
	LOADINGSCREEN_CONTEXT_LEGALMAIN = 3,
	LOADINGSCREEN_CONTEXT_SWAP = 4,
	LOADINGSCREEN_CONTEXT_PC_LANDING = 5,
	LOADINGSCREEN_CONTEXT_LOADGAME = 6,
	LOADINGSCREEN_CONTEXT_INSTALL = 7,
	LOADINGSCREEN_CONTEXT_LOADLEVEL = 8,
	LOADINGSCREEN_CONTEXT_MAPCHANGE = 9,
	LOADINGSCREEN_CONTEXT_LAST_FRAME = 10,
};

static hook::cdecl_stub<void(LoadingScreenContext, int)> setupLoadingScreens([]()
{
	// trailing byte differs between 323 and 505
	if (Is372())
	{
		return hook::get_call(hook::get_pattern("8D 4F 08 33 D2 E8 ? ? ? ? 40", 5));
	}

	return hook::get_call(hook::get_pattern("8D 4F 08 33 D2 E8 ? ? ? ? C6", 5));
});

class CLoadingScreens
{
public:
	static void Init(LoadingScreenContext context, int a2)
	{
		return setupLoadingScreens(context, a2);
	}
};

static bool g_setLoadingScreens;
static bool g_shouldKillNetwork;

void SetRenderThreadOverride()
{
	g_setLoadingScreens = true;
}

static bool(*g_callBeforeLoad)();

void FiveGameInit::LoadGameFirstLaunch(bool(*callBeforeLoad)())
{
	AddCrashometry("load_game_first_launch", "true");

	g_callBeforeLoad = callBeforeLoad;

	g_launchedGame = true;

	OnGameFrame.Connect([=] ()
	{
		if (g_shouldSetState)
		{
			if (*g_initState == MapInitState(6))
			{
				*g_initState = MapInitState(7);
				g_triedLoading = true;

				g_shouldSetState = false;
			}
		}

		static bool isLoading = false;

		if (isLoading)
		{
			if (*g_initState == 0)
			{
				trace("^2Game finished loading!\n");

				ClearVariable("shutdownGame");
				ClearVariable("killedGameEarly");

				OnGameFinalizeLoad();
				isLoading = false;
			}
		}
		else
		{
			if (*g_initState != 0)
			{
				isLoading = true;
			}
		}
	});

	OnKillNetwork.Connect([=] (const char* message)
	{
		AddCrashometry("kill_network", "true");
		AddCrashometry("kill_network_msg", message);

		trace("Killing network: %s\n", message);

		g_shouldKillNetwork = true;

		Instance<ICoreGameInit>::Get()->ClearVariable("networkInited");

		SetRenderThreadOverride();

		if (!Instance<ICoreGameInit>::Get()->GetGameLoaded())
		{
			Instance<ICoreGameInit>::Get()->SetVariable("killedGameEarly");
			Instance<ICoreGameInit>::Get()->SetVariable("gameKilled");
			Instance<ICoreGameInit>::Get()->SetVariable("shutdownGame");

			AddCrashometry("kill_network_game_early", "true");

			OnKillNetworkDone();
		}
	}, 500);

	OnGameRequestLoad();

	// stuff
	if (*g_initState == MapInitState(6))
	{
		*g_initState = MapInitState(7);
		g_triedLoading = true;
	}
	else
	{
		if (*g_initState == MapInitState(20))
		{
			*g_initState = MapInitState(11);
		}

		g_shouldSetState = true;
	}
}

void FiveGameInit::ReloadGame()
{
	AddCrashometry("reload_game", "true");

	//g_shouldReloadGame = true;
	m_gameLoaded = false;

	g_isNetworkKilled = false;

	ClearVariable("gameKilled");
}

static void DebugBreakDo()
{
	__debugbreak();
	ExitProcess(0);
}

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

struct InitFunctionStub : public jitasm::Frontend
{
	static uintptr_t LogStub(uintptr_t stub, int type)
	{
		trace("Running shutdown %s function: %p\n", typeMap[type], (void*)hook::get_unadjusted(stub));
		return stub;
	}

	virtual void InternalMain() override
	{
		push(r14);

		mov(rcx, qword_ptr[rax + rdx * 8 + 8]);
		mov(edx, r14d);

		// scratch space!
		sub(rsp, 0x20);

		mov(rax, (uintptr_t)LogStub);
		call(rax);

		// unwind
		add(rsp, 0x20);

		mov(ecx, r14d);
		pop(r14);

		jmp(rax);
	}
};

template<int Value>
static int ReturnInt()
{
	return Value;
}

static void(*g_runInitFunctions)(void*, int);
static void(*g_lookAlive)();

static void RunInitFunctionsWrap(void* skel, int type)
{
	if (g_callBeforeLoad)
	{
		while (!g_callBeforeLoad())
		{
			RunRlInitServicing();
		}
	}
	
	g_runInitFunctions(skel, type);
}

int ReturnFalse()
{
	return 0;
}

int BlipAsIndex(int blip)
{
	assert(blip != -1);

	return (blip & 0xFFFF);
}

static hook::cdecl_stub<void()> g_runWarning([]()
{
	return hook::get_pattern("83 F9 FF 74 0E E8 ? ? ? ? 84 C0 0F 94", -0x22);
});

DLL_EXPORT fwEvent<> PreSetupLoadingScreens;

static void(*g_origRunInitState)();
static void WrapRunInitState()
{
	if (g_setLoadingScreens)
	{
		PreSetupLoadingScreens();
		
		// code here used to use LOADINGSCREEN_CONTEXT_LAST_FRAME, but this also
		// triggered the loading spinner and/or some other render thread hang.
		//
		// LOADINGSCREEN_CONTEXT_INSTALL seems to be relatively neutral as it
		// would normally be used when copying disc content.
		CLoadingScreens::Init(LOADINGSCREEN_CONTEXT_INSTALL, 0);

		g_setLoadingScreens = false;
	}

	if (g_shouldReloadGame)
	{
		Instance<ICoreGameInit>::Get()->OnGameRequestLoad();

		*g_initState = MapInitState(14);

		g_shouldReloadGame = false;
	}

	if (g_shouldKillNetwork)
	{
		trace("Killing network, stage 2...\n");

		*g_initState = MapInitState(14);

		g_shouldKillNetwork = false;
	}

	if (!g_isNetworkKilled)
	{
		g_origRunInitState();
	}
}

static bool(*g_origSkipInit)(int);

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

static void(*g_shutdownSession)();

void ShutdownSessionWrap()
{
	Instance<ICoreGameInit>::Get()->SetVariable("gameKilled");
	Instance<ICoreGameInit>::Get()->SetVariable("shutdownGame");

	g_isNetworkKilled = true;
	*g_initState = MapInitState(14);

	AddCrashometry("kill_network_game", "true");

	OnKillNetworkDone();

	g_shutdownSession();

	Instance<ICoreGameInit>::Get()->OnShutdownSession();

	g_shouldKillNetwork = false;

	while (g_isNetworkKilled)
	{
		// warning screens apparently need to run on main thread
		RunRlInitServicing();

		g_runWarning();
	}

	Instance<ICoreGameInit>::Get()->OnGameRequestLoad();
}

template<typename T>
static void SafeRun(const T&& func)
{
	__try
	{
		func();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{

	}
}

void Component_RunPreInit()
{
	// initialize console arguments
	std::vector<std::pair<std::string, std::string>> setList;

	auto commandLine = GetCommandLineW();

	{
		wchar_t* s = commandLine;

		if (*s == L'"')
		{
			++s;
			while (*s)
			{
				if (*s++ == L'"')
				{
					break;
				}
			}
		}
		else
		{
			while (*s && *s != L' ' && *s != L'\t')
			{
				++s;
			}
		}
		
		while (*s == L' ' || *s == L'\t')
		{
			s++;
		}

		try
		{
			std::tie(g_argumentList, setList) = TokenizeCommandLine(ToNarrow(s));
		}
		catch (std::runtime_error)
		{
		}
	}

	se::ScopedPrincipal principalScope(se::Principal{ "system.console" });

	for (const auto& set : setList)
	{
		console::GetDefaultContext()->ExecuteSingleCommandDirect(ProgramArguments{ "set", set.first, set.second });
	}
}

static bool WaitWithWait(HANDLE handle)
{
	return WaitForSingleObject(handle, 500) == WAIT_OBJECT_0;
}

static hook::cdecl_stub<void()> _setRenderDeleg([]
{
	return hook::get_pattern("48 89 5D 0F 48 8D 55 37 0F 10 4D 07 0F", -0xA7);
});

static hook::cdecl_stub<void(void*, bool)> _kickRender([]
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 45 33 FF 44 38 7B 0D 74 0E"));
});

static void (*g_origCtrlInit)();

static void OnCtrlInit()
{
	uint8_t orig1, orig2, orig3;

	static auto location1 = hook::get_pattern<uint8_t>("E8 ? ? ? ? 84 C0 74 1F 48 8B 05 ? ? ? ? 48 8B 40", 7);

	{
		orig1 = location1[0];
		orig2 = location1[-0x5F];
		hook::put<uint8_t>(location1, 0xEB);
		hook::put<uint8_t>(location1 - 0x5F, 0xEB);
	}

	static auto location3 = hook::get_pattern<uint8_t>("33 D2 89 7C 24 44 66 C7", -0x37);

	{
		orig3 = *location3;
		hook::return_function(location3);
	}

	static auto rti = hook::get_address<void*>(hook::get_pattern("E8 ? ? ? ? 45 33 FF 44 38 7B 0D 74 0E", -6));

	_setRenderDeleg();
	_kickRender(rti, true);

	OnMainGameFrame.Connect([orig1, orig2, orig3]
	{
		hook::put<uint8_t>(location1, orig1);
		hook::put<uint8_t>(location1 - 0x5F, orig2);
		hook::put<uint8_t>(location3, orig3);
	});

	// orig
	g_origCtrlInit();
}

static bool (*g_origParamToInt)(void* param, int* value);

static bool ParamToInt_Threads(void* param, int* value)
{
	bool rv = g_origParamToInt(param, value);

	if (!rv)
	{
		*value = std::min(*value, 4);
	}

	return rv;
}

static void (*g_origPhotoSize)(int* w, int* h, int down);

static void PhotoSizeStub(int* w, int* h, int down)
{
	// story mode may lead to more advanced photo requests, which will crash in JPEG serialization
	if (!Instance<ICoreGameInit>::Get()->HasVariable("storyMode"))
	{
		down = 1;
	}

	return g_origPhotoSize(w, h, down);
}

static HookFunction hookFunction([] ()
{
	// continue on
	_wunlink(MakeRelativeCitPath(L"data\\cache\\error_out").c_str());

	// fwApp 2:1 state handler (loaded game), before running init state machine
	if (!CfxIsSinglePlayer())
	{
		auto loc = hook::get_pattern<char>("32 DB EB 02 B3 01 E8 ? ? ? ? 48 8B", 6);

		hook::set_call(&g_origRunInitState, loc);
		hook::call(loc, WrapRunInitState);

		loc -= 9;
		loc -= 6;

		hook::set_call(&g_origSkipInit, loc);
		hook::call(loc, WrapSkipInit);
	}

	// NOP out any code that sets the 'entering state 2' (2, 0) FSM internal state to '7' (which is 'load game'), UNLESS it's digital distribution with standalone auth...
	// Since game build 2699.16 executables now shared.
	char* p = (Is2060() || Is2802()) ? hook::pattern("BA 08 00 00 00 8D 41 FC 83 F8 01").count(1).get(0).get<char>(14) : hook::pattern("BA 07 00 00 00 8D 41 FC 83 F8 01").count(1).get(0).get<char>(14);

	char* varPtr = p + 2;
	g_initState = (int*)(varPtr + *(int32_t*)varPtr + 4);

	// check the pointer to see if it's digital distribution
	g_isDigitalDistrib = Is2802() || (p[-26] == 3);

	// this is also a comparison point to find digital distribution type... this function will also set '3' if it's digital distrib with standalone auth
	// and if this *is* digital distribution, we want to find a completely different place that sets the value to 8 (i.e. BA 08 ...)
	if (g_isDigitalDistrib)
	{
		p = hook::pattern("BA 08 00 00 00 8D 41 FC 83 F8 01").count(1).get(0).get<char>(14);
	}

	if (!CfxIsSinglePlayer())
	{
		// nop the right pointer
		hook::nop(p, 6);

		// and call our little internal loop function from there
		hook::call(p, WaitForInitLoop);

		// also add a silly loop to state 6 ('wait for landing page'?) - or 20 in 2189+
		if (!xbr::IsGameBuildOrGreater<2189>())
		{
			p = hook::pattern("C7 05 ? ? ? ? 06 00 00 00 EB 3F").count(1).get(0).get<char>(0);
		}
		else
		{
			p = hook::get_pattern<char>("85 C0 74 E8 83 F8 01 75 37", 0x20);

		}

		hook::nop(p, 10);
		hook::call(p, WaitForInitLoopWrap);

		// force the above hook to go to stage 6, not stage 7
		hook::nop(p - 10, 2);

		// grr, reloading stage
		{
			auto loc = hook::get_pattern("75 0F E8 ? ? ? ? 8B 0D ? ? ? ? 3B C8", -12);
			hook::set_call(&g_shutdownSession, loc);
			hook::call(loc, ShutdownSessionWrap);
		}
	}

	if (!CfxIsSinglePlayer())
	{
		// init function bit #1
		static InitFunctionStub initFunctionStub;
		initFunctionStub.Assemble();

		p = hook::pattern("41 8B CE FF 54 D0 08").count(1).get(0).get<char>(0);

		hook::nop(p, 7);
		hook::call_rcx(p, initFunctionStub.GetCode());
	}

	// OF NOTE:
	// -> CBaseModelInfo + 0x70 (instance?) being a weird FF-style pointer on deletion
	// -> ??_7audVehicleReflectionsEntity@@6B@ + 800 (atArray of unk) being set to a freed pointer although capacity > 0 (destructor called somehow?)

	// attempted fix for item #1
	static struct : jitasm::Frontend
	{
		void InternalMain() override
		{
			mov(qword_ptr[rcx + 0x70], r9);

			// original code
			mov(qword_ptr[rcx + 0xA8], r9);

			ret();
		}
	} miStub;

	hook::jump(hook::pattern("4C 89 89 A8 00 00 00 C3").count(1).get(0).get<void>(), miStub.GetCode());

	// loading screen frame limit (~250 FPS max, to fix people who force-disable vsync)
	// use 0.0f to uncap entirely
	hook::put<float>(hook::get_address<float*>(hook::get_pattern("0F 2F 05 ? ? ? ? 0F 82 ? ? ? ? E8 ? ? ? ? 48 89", 3)), 4.0f);

	if (!CfxIsSinglePlayer())
	{
		// bypass the state 20 calibration screen loop (which might be wrong; it doesn't seem to exist in my IDA dumps of 323/331 Steam)
		auto matches = hook::pattern("E8 ? ? ? ? 8A D8 84 C0 74 0E C6 05");

		assert(matches.size() <= 1);

		for (int i = 0; i < matches.size(); i++)
		{
			hook::call(matches.get(i).get<void>(), ReturnInt<1>);
		}
	}

	// stop the ros sdk input blocker
	hook::jump(hook::get_pattern("48 8B 01 FF 50 10 84 C0 74 05 BB 01 00 00 00 8A", -0x14), ReturnFalse);
	
	// block loading until conditions succeed
	char* loadStarter = hook::pattern("BA 02 00 00 00 E8 ? ? ? ? E8 ? ? ? ? 8B").count(1).get(0).get<char>(5);
	hook::set_call(&g_runInitFunctions, loadStarter);
	hook::set_call(&g_lookAlive, loadStarter + 5);

	if (!CfxIsSinglePlayer())
	{
		hook::call(loadStarter, RunInitFunctionsWrap);
	}

	// don't conditionally check player blip handle
	hook::call(hook::get_pattern("C8 89 05 ? ? ? ? E8 ? ? ? ? 89 05", 7), BlipAsIndex);

	if (!CfxIsSinglePlayer())
	{
		// don't load commandline.txt
		hook::return_function(hook::get_pattern("45 33 E4 83 39 02 4C 8B FA 45 8D 6C", -0x1C));
	}

	// sometimes this crashes
	SafeRun([]()
	{
		// ignore collision-related archetype flag in /CREATE_OBJECT(_NO_OFFSET)?/
		hook::nop(hook::get_pattern("48 50 C1 E9 04 F6 C1 01 0F 84 ? ? 00 00 45", 8), 6);
	});

	// disable eventschedule.json refetching on failure
	//hook::nop(hook::get_pattern("80 7F 2C 00 75 09 48 8D 4F F8 E8", 10), 5);
	// 1493+:
	if (!Is372())
	{
		hook::nop(hook::get_pattern("38 4B 2C 75 60 48 8D 4B F8 E8", 9), 5);
	}

	// don't set pause on focus loss, force it to 0
	if (!Is372())
	{
		auto location = hook::get_pattern<char>("0F 95 05 ? ? ? ? E8 ? ? ? ? 48 85 C0");
		auto addy = hook::get_address<char*>(location + 3);
		hook::put<char>(addy, 0);
		hook::nop(location, 7);
	}

	// don't complain about not meeting minimum system requirements
	hook::return_function(hook::get_pattern("B9 11 90 02 8A 8B FA E8", -10));

	// increase reserved physical memory amount threefold (to ~900 MB)
	hook::put<uint32_t>(hook::get_pattern("48 81 C1 00 00 00 12 48 89 0D", 3), 0x36000000);

	// early init command stuff
	rage::OnInitFunctionStart.Connect([](rage::InitFunctionType type)
	{
		if (type == rage::InitFunctionType::INIT_CORE)
		{
			// run command-line initialization
			se::ScopedPrincipal principalScope(se::Principal{ "system.console" });

			for (const auto& bit : g_argumentList)
			{
				console::GetDefaultContext()->ExecuteSingleCommandDirect(bit);
			}
		}
	});

	// fix 32:9 being interpreted as 3 spanned monitors
	// (change 3.5 aspect cap to 3.6, which is enough to contain 3x 5:4, but does not contain 2x16:9 anymore)
	{
		auto location = hook::get_pattern<char>("EB ? 0F 2F 35 ? ? ? ? 76 ? 48 8B CF E8F", 5);
		auto stubLoc = hook::AllocateStubMemory(4);

		*(float*)stubLoc = 3.6f;

		hook::put<int32_t>(location, int32_t(intptr_t(stubLoc) - intptr_t(location) - 4));
	}

	// SC eula accepted
	hook::put<uint32_t>(hook::get_pattern("84 C0 74 36 48 8B 0D ? ? ? ? 48 85 C9", -13), 0x90C301B0);

	// don't downscale photos a lot
	g_origPhotoSize = hook::trampoline(hook::get_pattern("41 3B D9 72 09", -0x3A), PhotoSizeStub);

	// don't do 500ms waits for renderer
	{
		auto location = hook::get_pattern<char>("EB 0A B9 F4 01 00 00 E8", 7);
		hook::nop(location, 5); // sleep call
		hook::call(location + 21, WaitWithWait); // wait call
	}

	// kick renderer before slow dinput code
	{
		auto location = hook::get_pattern("74 0B E8 ? ? ? ? 8A 0D ? ? ? ? 80", 2);
		hook::set_call(&g_origCtrlInit, location);
		hook::call(location, OnCtrlInit);
	}

	// no showwindow early
	if (!CfxIsSinglePlayer())
	{
		auto location = hook::get_pattern<char>("41 8B D4 48 8B C8 48 8B D8 FF 15", 9);
		hook::nop(location, 6);
		hook::nop(location + 9, 6);
		hook::nop(location + 18, 6);
	}

	// b2699 fix: force `-nodpiadjust` as it's broken
	if (xbr::IsGameBuild<2699>())
	{
		hook::put<uint16_t>(hook::get_pattern("48 83 3D ? ? ? ? 00 0F 85 A3 00 00 00 48 8B 4B", 8), 0xE990);
	}

	// limit max worker threads to 4 (since on high-core-count systems this leads
	// to a lot of overhead when there's a blocking wait)
	{
		auto location = hook::get_pattern("89 05 ? ? ? ? E8 ? ? ? ? 48 8D 3D ? ? ? ? 48 63", 6);
		hook::set_call(&g_origParamToInt, location);
		hook::call(location, ParamToInt_Threads);
	}
});
