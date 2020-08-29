/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"

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
		else if (Is2060())
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

static volatile bool g_isNetworkKilled;

static hook::cdecl_stub<void(int, int)> setupLoadingScreens([]()
{
	// trailing byte differs between 323 and 505
	if (Is372())
	{
		return hook::get_call(hook::get_pattern("8D 4F 08 33 D2 E8 ? ? ? ? 40", 5));
	}

	return hook::get_call(hook::get_pattern("8D 4F 08 33 D2 E8 ? ? ? ? C6", 5));
});

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

	/*OnGameRequestLoad.Connect([=] ()
	{
		
	});*/

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
		trace("Running shutdown %s function: %p\n", typeMap[type], (void*)stub);
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
		
		mov(ecx, r14d);
		call(rax);

		add(rsp, 0x20);

		pop(r14);

		ret();
	}
};

struct InitFunctionStub2 : public jitasm::Frontend
{
	static uintptr_t LogStub(uintptr_t stub, int type)
	{
		trace("Running init %s function: %p\n", typeMap[type], stub);
		return stub;
	}

	virtual void InternalMain() override
	{
		push(r14);

		mov(rcx, qword_ptr[rax + rdx * 8]);

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

static hook::cdecl_stub<void(int)> initModelInfo([] ()
{
	return hook::pattern("48 83 EC 20 83 F9 01 0F 85 A5 06 00 00").count(1).get(0).get<void>(-0x15);
});

static hook::cdecl_stub<void(int)> shutdownModelInfo([] ()
{
	return hook::pattern("48 83 EC 20 83 F9 01 0F 85 77 01 00 00").count(1).get(0).get<void>(-0x10);
});

static hook::cdecl_stub<void()> initStreamingInterface([] ()
{
	return hook::get_call(hook::pattern("41 8B CE E8 ? ? ? ? 48 8B 0D ? ? ? ? 48 8D").count(1).get(0).get<void>(3));
});

static hook::cdecl_stub<void(void*)> clearSceneLinkedList([] ()
{
	return hook::pattern("48 8B F9 EB 38 48 8B 17 48 8B CB 48 8B").count(1).get(0).get<void>(-0xD);
});

static hook::cdecl_stub<void()> shutdownScene([] ()
{
	return hook::pattern("BB 01 00 00 00 8B CB E8 ? ? ? ? E8").count(1).get(0).get<void>(-0x4A);
});

static hook::cdecl_stub<void()> initScene([] ()
{
	return hook::pattern("BF 01 00 00 00 48 8D 0D ? ? ? ? 8B D7 E8").count(1).get(0).get<void>(-0x31);
});

static void(*g_deinitLevel)();

static void* g_sceneLinkedList;

template<typename T>
class allocWrap : public rage::sysUseAllocator
{
private:
	T m_value;

public:
	allocWrap()
		: m_value()
	{
		
	}

	allocWrap(const T& value)
		: m_value(value)
	{

	}

	inline T& Get() const
	{
		return m_value;
	}

	inline void Set(const T& value)
	{
		m_value = value;
	}

	operator T&() const
	{
		return m_value;
	}
};

static atArray<allocWrap<uint32_t>>* g_cacheArray;

static char* g_boundsStreamingModule;

class CInteriorProxy
{
public:
	virtual ~CInteriorProxy() = 0;
};

template<typename T>
struct LinkNode : public rage::sysUseAllocator
{
	T* value;
	LinkNode* next;
};

static void ClearInteriorProxyList(void* list)
{
	auto linkedList = *(LinkNode<CInteriorProxy>**)list;

	if (linkedList)
	{
		do 
		{
			delete linkedList->value;

			auto next = linkedList->next;
			delete linkedList;
			
			linkedList = next;
		} while (linkedList);
	}

	*(void**)list = nullptr;
}

static hook::cdecl_stub<void(int)> initPhysics([] ()
{
	return hook::pattern("83 F9 08 75 23 33 C9 E8 ? ? ? ? 48 8D 0D").count(1).get(0).get<void>(-0x12);
});

static hook::cdecl_stub<void(int)> shutdownPhysics([] ()
{
	return hook::pattern("83 F9 04 75 15 48 8D 0D").count(1).get(0).get<void>(-0x12);
});

static atPool<CInteriorProxy>** g_interiorProxyPool;

static bool g_inLevelFree;
static bool g_didLevelFree;

static int g_stackIdx;

static std::unordered_map<int, std::vector<uintptr_t>> g_stacks;

static atArray<CInteriorProxy*>* g_vehicleReflEntityArray;

static hook::cdecl_stub<void(void*)> extraContentMgr__doScanInt([] ()
{
	return hook::get_call(hook::pattern("48 83 EC 20 80 A1 7A 01 00 00 FE 41 8A D8").count(1).get(0).get<void>(17));
});

// a3: skip mount?
static hook::cdecl_stub<void(void*, bool, bool)> extraContentMgr__doScanPost([] ()
{
	//return hook::pattern("48 83 EC 20 80 A1 7A 01 00 00 FE 41 8A D8").count(1).get(0).get<void>(-0x6);
	return hook::get_call(hook::pattern("48 83 EC 20 80 A1 7A 01 00 00 FE 41 8A D8").count(1).get(0).get<void>(30));
});

static void** g_extraContentMgr;

static void ClearInteriorProxyPool()
{
	trace(__FUNCTION__ ": no-op\n");
}

struct CPortalScanner
{
	char pad[24];
	void* interiorProxy;
};

struct CRenderPhaseScanned
{
	char pad[1280];
	CPortalScanner* portalScanner;
};

static std::map<std::string, CRenderPhaseScanned*> g_renderPhases;

static void(*g_loadClipSets)();

static void DeinitLevel()
{
	static bool initedLateHook = false;

	if (!initedLateHook)
	{
		// late hook to prevent scenario manager from reinitializing on map load - it messes things up, a lot, and I doubt custom maps use scenarios anyway. :)
		hook::put<uint16_t>(hook::pattern("83 F9 04 0F 85 F9 00 00 00").count(1).get(0).get<void>(3), 0xE990); // shutdown
		hook::put<uint16_t>(hook::pattern("83 F9 02 0F 85 C6 01 00 00").count(1).get(0).get<void>(3), 0xE990); // init

		// don't load mounted ped (personality) metadata anymore (temp dbg-y?)
		hook::nop(hook::pattern("48 8B DA 83 E9 14 74 5B").count(1).get(0).get<void>(0x72 - 12), 5);

		// recreate interior proxy pool sanely
		//char* poolFn = hook::get_pattern<char>("41 0F B7 C6 4D 89 73 E8 41 8B F6 66 89 44 24 28", -0x23);
		//poolFn += 0xD4;
		//hook::nop(poolFn, 44);
		//hook::jump(poolFn, ClearInteriorProxyPool);

		initedLateHook = true;
	}

	// stuff
	g_inLevelFree = true;

	g_deinitLevel();

	shutdownModelInfo(1);
	initModelInfo(1);
	initStreamingInterface();

	//g_loadClipSets();

	// extra content manager shutdown session removes these, and we want these before init session, so we scan them right here, right now.
	// TEMP REMOVE
	extraContentMgr__doScanInt(*g_extraContentMgr);
	extraContentMgr__doScanPost(*g_extraContentMgr, false, false);

	//shutdownPhysics(1);
	//initPhysics(1);

	// unknown value in the bounds streaming module, doesn't get cleared on 'after map loaded' shutdown
	int colCrashOffset = *hook::get_pattern<int>("0F B7 83 ? ? 00 00 BA FF 1F 00 00 66 33 C1 66", 3);
	int colCrashCountMax = (colCrashOffset - 464) / 4;

	*(uint32_t*)(g_boundsStreamingModule + colCrashOffset) = 0;

	// bounds streaming module, 'has preloading bounds completed' value
	*(uint8_t*)(g_boundsStreamingModule + 255) = 0;

	// clear the 'loaded cache hashes' list
	//*g_cacheArray = atArray<allocWrap<uint32_t>>(16);
	g_cacheArray->ClearCount();

	// free one CScene list of all the bad influences from the last session
	ClearInteriorProxyList(g_sceneLinkedList);

	// also clear the interior proxy pool out, as it might contain garbage references to static bounds, still
	(*g_interiorProxyPool)->Clear();

	// and some global vehicle audio entity also houses... interior proxies.
	*g_vehicleReflEntityArray = atArray<CInteriorProxy*>();

	// clear interior proxies from render phases
	for (auto& pair : g_renderPhases)
	{
		CRenderPhaseScanned* renderPhase = pair.second;

		if (renderPhase->portalScanner)
		{
			trace("clearing %s interior proxy (was %p)\n", pair.first.c_str(), renderPhase->portalScanner->interiorProxy);

			renderPhase->portalScanner->interiorProxy = nullptr;
		}
	}

	g_inLevelFree = false;

	if (!g_didLevelFree)
	{
		for (auto& stack : g_stacks)
		{
			FILE* f = fopen(va("D:\\dev\\stacks\\%p.txt", ((stack.first / 256) * 256)), "a");

			if (f)
			{
				fprintf(f, "--- %p ---\n", stack.first);

				for (auto& entry : stack.second)
				{
					fprintf(f, "%p\n", entry);
				}

				fprintf(f, "--- --- ---\n");

				fclose(f);
			}
		}

		g_stackIdx = 0;
		g_stacks.clear();

		g_didLevelFree = true;
	}
}

namespace rage
{
	class fwArchetype
	{
	public:
		virtual ~fwArchetype() = 0;

		virtual void m1() = 0;

		virtual void m2() = 0;

		virtual void m3() = 0;

		virtual void m4() = 0;

		virtual void m5() = 0;

		virtual void m6() = 0;

		virtual void m7() = 0;

		virtual void m8() = 0;

		virtual void _CleanUp() = 0;
	};
}

static void DestructMI(rage::fwArchetype* archetype)
{
	archetype->_CleanUp();
	archetype->~fwArchetype(); // note: we can't delete this; that'll cause a double-free later on
}

static void(*g_origFreeMapTypes)(void* store, void* entry, void* a3, bool a4);

void DoFreeMapTypes(void* store, void* entry, void* a3, bool a4)
{
	if ((uintptr_t)entry != 16)
	{
		g_origFreeMapTypes(store, entry, a3, a4);
	}
}

static intptr_t(*g_origMemFree)(void*, void*);
static void** g_unsafePointerLoc;

#include <unordered_map>
#include <mutex>
#include <array>

static std::unordered_map<uintptr_t, size_t> g_allocData;
static std::mutex g_allocMutex;

static CRITICAL_SECTION g_allocCS;

static std::vector<uintptr_t> g_unsafeStack;
static std::map<uintptr_t, std::pair<size_t, std::array<uintptr_t, 16>>> g_freeThings;
static std::map<std::pair<uintptr_t, size_t>, std::array<uintptr_t, 16>> g_allocStuff;

intptr_t CustomMemFree(void* allocator, void* pointer)
{
	intptr_t retval = g_origMemFree(allocator, pointer);

	/*if (pointer != nullptr && *g_unsafePointerLoc)
	{
		size_t allocSize = 0;

		if (g_unsafeStack.size() == 0)
		{
			{
				std::unique_lock<std::mutex> lock(g_allocMutex);

				auto it = g_allocData.find(pointer);

				if (it != g_allocData.end())
				{
					allocSize = it->second;
					g_allocData.erase(it);
				}
			}

			if (**(void***)g_unsafePointerLoc >= pointer && **(void***)g_unsafePointerLoc < ((char*)pointer + allocSize))
			{
				std::vector<uintptr_t> stackList(96);

				uintptr_t* stack = (uintptr_t*)_AddressOfReturnAddress();

				for (int i = 0; i < stackList.size(); i++)
				{
					stackList[i] = stack[i];
				}

				g_unsafeStack = stackList;
			}
		}

	}*/

	if (/*!g_didLevelFree && */pointer != nullptr)
	{
		//std::unique_lock<std::mutex> lock(g_allocMutex);
		EnterCriticalSection(&g_allocCS);

		uintptr_t ptr = (uintptr_t)pointer;

		auto it = g_allocData.find(ptr);

		if (it != g_allocData.end())
		{
			size_t allocSize = it->second;

			g_allocStuff.erase({ ptr, allocSize });

			if (allocSize >= 8)
			{
				if (*(uintptr_t*)ptr == 0x141826A10)
				{
					uintptr_t* stack = (uintptr_t*)_AddressOfReturnAddress();
					stack += (32 / 8);

					std::array<uintptr_t, 16> stacky;
					memcpy(stacky.data(), stack, 16 * 8);

					{
						g_freeThings.insert({ ptr, { allocSize, stacky } });
					}
				}
			}

			/*static char* location = hook::pattern("4C 8D 0D ? ? ? ? 48 89 01 4C 89 81 80 00 00").count(1).get(0).get<char>(3);
			static char** g_collectionRoot = (char**)(location + *(int32_t*)location + 4);

			for (int i = 0; i < 0x950; i++)
			{
				if (g_collectionRoot[i])
				{
					void* baad = *(void**)(g_collectionRoot[i] + 32);

					if (baad >= pointer && baad < ((char*)pointer + allocSize))
					{
						atArray<char>* array = (atArray<char>*)(g_collectionRoot[i] + 128);

						trace("freed collection %s (%p-%p)\n", &array->Get(0), pointer, allocSize + (char*)pointer);

						uintptr_t* stack = (uintptr_t*)_AddressOfReturnAddress();
						stack += (32 / 8);

						for (int i = 0; i < 16; i++)
						{
							trace("stack: %p\n", stack[i]);
						}
					}
				}
			}*/



			/*if (g_inLevelFree)
			{
			if (allocSize != -1)
			{
			int stackIdx = g_stackIdx++;

			std::vector<uintptr_t> stackList(96);

			uintptr_t* stack = (uintptr_t*)_AddressOfReturnAddress();

			for (int i = 0; i < stackList.size(); i++)
			{
			stackList[i] = stack[i];
			}

			g_stacks[stackIdx] = stackList;

			trace("level free: %p-%p - stack idx: %d\n", pointer, (char*)pointer + allocSize, stackIdx);
			}
			}*/

			g_allocData.erase(it);
		}

		LeaveCriticalSection(&g_allocCS);
	}

	return retval;
}

static void*(*g_origMemAlloc)(void*, intptr_t size, intptr_t align, int subAlloc);

void* CustomMemAlloc(void* allocator, intptr_t size, intptr_t align, int subAlloc)
{
	void* ptr = g_origMemAlloc(allocator, size, align, subAlloc);

	/*if (*g_unsafePointerLoc >= ptr && *g_unsafePointerLoc < ((char*)ptr + size))
	{
#ifdef _DEBUG
		__debugbreak();
#endif

		assert(!"Tried to allocate over unsafe pointer!");
	}

	if (*g_unsafePointerLoc)
	{
		void*** unsafePtrLoc = (void***)g_unsafePointerLoc;

		if (**unsafePtrLoc >= ptr && **unsafePtrLoc < ((char*)ptr + size))
		{
#ifdef _DEBUG
			__debugbreak();
#endif

			assert(!"Tried to allocate over unsafe pointer!");
		}
	}*/
	//memset(ptr, 0, size);

	if (subAlloc == 0)
	{
		uintptr_t ptr_ = (uintptr_t)ptr;

		//std::unique_lock<std::mutex> lock(g_allocMutex);
		EnterCriticalSection(&g_allocCS);
		g_allocData[ptr_] = size;

		/*auto first = g_freeThings.lower_bound(ptr_);
		auto second = g_freeThings.upper_bound(ptr_ + size);

		for (auto it = first; first != second; first++)
		{
			if (ptr_ >= it->first && ptr_ < (it->first + it->second.first))
			{
				if (size == it->second.first)
				{
					trace("allocate over stacky!\n");

					auto stacky = it->second.second;

					for (auto& entry : stacky)
					{
						trace("%p\n", entry);
					}

					trace("noooooooooo!\n");
				}
			}
		}*/

		//g_allocData[ptr_] = size;
		uintptr_t* stack = (uintptr_t*)_AddressOfReturnAddress();
		stack += (32 / 8);

		std::array<uintptr_t, 16> stacky;
		memcpy(stacky.data(), stack, 16 * 8);

		g_allocStuff[{ ptr_, size }] = stacky;

		LeaveCriticalSection(&g_allocCS);
	}

	return ptr;
}

template<int Value>
static int ReturnInt()
{
	return Value;
}

static void ErrorDo(uint32_t error)
{
	if (error == 0xbe0e0aea) // ERR_GEN_INVALID
	{
		FatalError("Invalid rage::fiPackfile encryption type specified. If you have any modified game files, please remove or verify them. See http://rsg.ms/verify for more information.");
	}

	trace("error function called from %p for code 0x%08x\n", _ReturnAddress(), error);

	// provide pickup file for minidump handler to use
	FILE* dbgFile = _wfopen(MakeRelativeCitPath(L"cache\\error_out").c_str(), L"wb");

	if (dbgFile)
	{
		fwrite(&error, 1, 4, dbgFile);

		uint64_t retAddr = (uint64_t)_ReturnAddress();
		fwrite(&retAddr, 1, 8, dbgFile);

		fclose(dbgFile);
	}

	// overwrite the CALL address with a marker containing the error code, then move the return address ahead
	// this should lead to crash dumps showing the exact location of the CALL as the exception address
	{
		DWORD oldProtect;

		static struct : jitasm::Frontend
		{
			uint32_t error;

			virtual void InternalMain() override
			{
				mov(rax, 0x1000000000 | error);
				mov(dword_ptr[rax], 0xDEADBADE);
			}
		} code;

		code.error = error;

		// assemble *first* as GetCodeSize does not automatically call Assemble
		code.Assemble();

		// 5: CALL size
		// 6: size of mov dword_ptr[rax], ...
		char* retAddr = (char*)_ReturnAddress() - 5 - code.GetCodeSize() + 6;

		VirtualProtect(retAddr, code.GetCodeSize(), PAGE_EXECUTE_READWRITE, &oldProtect);
		memcpy(retAddr, code.GetCode(), code.GetCodeSize());

		// for good measure
		FlushInstructionCache(GetCurrentProcess(), retAddr, code.GetCodeSize());

		// jump to the new return address
		*(void**)_AddressOfReturnAddress() = retAddr;
	}

	/*
	// NOTE: crashes on this line are supposed to be read based on the exception-write address!
	*(uint32_t*)(error | 0x1000000000) = 0xDEADBADE;

	TerminateProcess(GetCurrentProcess(), -1);*/
}

static void(*g_runInitFunctions)(void*, int);
static void(*g_lookAlive)();

static void RunInitFunctionsWrap(void* skel, int type)
{
	if (g_callBeforeLoad)
	{
		while (!g_callBeforeLoad())
		{
			g_lookAlive();

			OnGameFrame();
			OnMainGameFrame();
		}
	}
	
	g_runInitFunctions(skel, type);
}

static void(*g_openPopCycle)(void*, void*);
void ClearAndOpenPopCycle(atArray<allocWrap<std::array<char, 608>>>* popArray, void* fname)
{
	popArray->Clear();
	//g_openPopCycle(popArray, fname);
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

static HANDLE hHeap = HeapCreate(0, 0, 0);

static bool(*g_origIsMine)(void*, void*);
static bool(*g_origRealloc)(void*, void*, size_t);

static bool isMine(void* allocator, void* mem)
{
	return (*(uint32_t*)((DWORD_PTR)mem - 4) & 0xFFFFFFF0) == 0xDEADC0C0;
}

static bool isMineHook(void* allocator, void* mem)
{
	return isMine(allocator, mem) || g_origIsMine(allocator, mem);
}

DWORD g_mainThreadId;

static void* AllocEntry(void* allocator, size_t size, int align, int subAlloc)
{
	if (!g_triedLoading && g_mainThreadId != GetCurrentThreadId())
	{
		return g_origMemAlloc(allocator, size, align, subAlloc);
	}

	DWORD_PTR ptr = (DWORD_PTR)HeapAlloc(hHeap, 0, size + 32);//malloc(size + 32);
	ptr += 4;

	void* mem = (void*)(((uintptr_t)ptr + 15) & ~(uintptr_t)0xF);

	*(uint32_t*)((uintptr_t)mem - 4) = 0xDEADC0C0 | (((uintptr_t)ptr + 15) & 0xF);

	return mem;
}

static void FreeEntry(void* allocator, void* ptr)
{
	if (!isMine(allocator, ptr))
	{
		g_origMemFree(allocator, ptr);
		return;
	}

	void* memReal = ((char*)ptr - (16 - (*(uint32_t*)((uintptr_t)ptr - 4) & 0xF)) - 3);

	HeapFree(hHeap, 0, memReal);
	//free(memReal);
}

static void ReallocEntry(void* allocator, void* ptr, size_t size)
{
	if (g_origIsMine(allocator, ptr))
	{
		g_origRealloc(allocator, ptr, size);
		return;
	}

	//FreeEntry(allocator, ptr);

/*	void* memReal = ((char*)ptr - (16 - (*(uint32_t*)((uintptr_t)ptr - 4) & 0xF)) - 3);
	ptrdiff_t delta = (char*)ptr - (char*)memReal;

	//HeapFree(hHeap, 0, memReal);
	memReal = realloc(memReal, size);

	void* mem = (char*)memReal + delta;

	assert(isMineHook(allocator, mem));

	return mem;*/

	return;
}

static void*(*CRenderPhase__ctor)(void* renderPhase, void* a2, void* a3, void* a4, void* a5, int a6);

void* CRenderPhaseScanned__ctorWrap(CRenderPhaseScanned* renderPhase, void* a2, char* name, void* a4, void* a5, int a6)
{
	g_renderPhases[name] = renderPhase;

	return CRenderPhase__ctor(renderPhase, a2, name, a4, a5, a6);
}

template<typename T>
struct atDictionary
{
	bool isValid;
	atArray<T> data;
};

struct fwClipSet
{
	char pad[20];
	uint32_t dicthash;
};

struct ClipSetEntry
{
	uint32_t key;
	fwClipSet* clipSet;
};

struct ClipMetaEntry
{
	uint32_t key;
	char streamingPolicy;
	void* clipMeta;
};

atDictionary<ClipSetEntry>* g_clipSets = (atDictionary<ClipSetEntry>*)0x141C986A0;
atDictionary<ClipMetaEntry>* g_clipSetMetas = (atDictionary<ClipMetaEntry>*)0x141C986B8;


void TrackClipSetShutdown()
{
	auto& clipSets = g_clipSets->data;
	auto& clipMetas = g_clipSetMetas->data;

	trace("clipset count: %d\n", clipSets.GetCount());
	trace("clipmeta count: %d\n", clipMetas.GetCount());

	for (auto& cs : clipSets)
	{
		trace("cs %08x: %08x\n", cs.key, cs.clipSet->dicthash);
	}

	for (auto& cm : clipMetas)
	{
		trace("cm %08x: %d%s\n", cm.key, cm.streamingPolicy, (cm.streamingPolicy & 2) ? " UNLOADING" : "");
	}
}

static hook::cdecl_stub<void()> g_runWarning([]()
{
	return hook::get_pattern("83 F9 FF 74 0E E8 ? ? ? ? 84 C0 0F 94", -0x22);
});

static void(*g_origRunInitState)();
static void WrapRunInitState()
{
	if (g_setLoadingScreens)
	{
		setupLoadingScreens(10, 0);

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

	/*while (g_isNetworkKilled)
	{
		Sleep(50);

		// warning screens apparently need to run on main thread
		OnGameFrame();
		OnMainGameFrame();

		g_runWarning();
	}*/

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
		OnGameFrame();
		OnMainGameFrame();

		// 1604 (same as nethook)
		// 1868
		// 2060
		if (!Is2060())
		{
			((void(*)())hook::get_adjusted(0x1400067E8))();
			((void(*)())hook::get_adjusted(0x1407D1960))();
			((void(*)())hook::get_adjusted(0x140025F7C))();
			((void(*)(void*))hook::get_adjusted(0x141595FD4))((void*)hook::get_adjusted(0x142DC9BA0));
		}
		else
		{
			((void (*)())hook::get_adjusted(0x140006A80))();
			((void (*)())hook::get_adjusted(0x1407EB39C))();
			((void (*)())hook::get_adjusted(0x1400263A4))();
			((void (*)(void*))hook::get_adjusted(0x1415CF268))((void*)hook::get_adjusted(0x142D3DCC0));
		}

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

static void OverrideArguments()
{
	// unless specified in INI, force windowed-borderless
#ifdef EXPERIMENT_FLAG_DISABLE_FULLSCREEN
	{
		std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");

		bool forceWindowed = true;

		if (GetFileAttributes(fpath.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			forceWindowed = (GetPrivateProfileInt(L"Game", L"ForceWindowingMode", 1, fpath.c_str()) == 1);
		}

		if (forceWindowed)
		{
			// -windowed
			hook::put<int>(hook::get_address<int*>(hook::get_pattern("41 3B CE 0F 85 FC 00 00 00 48 39 1D", 12)), 1);

			// -borderless
			hook::put<int>(hook::get_address<int*>(hook::get_pattern("48 1B DB 4C 39 3D ? ? ? ? 41", 6)), 1);
		}
	}
#endif
}

static InitFunction initFunction([]()
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
		catch (std::runtime_error& e)
		{
			trace("couldn't parse command line: %s\n", e.what());
		}
	}

	se::ScopedPrincipal principalScope(se::Principal{ "system.console" });

	for (const auto& set : setList)
	{
		console::GetDefaultContext()->ExecuteSingleCommandDirect(ProgramArguments{ "set", set.first, set.second });
	}
});

static HookFunction hookFunction([] ()
{
	// continue on
	_wunlink(MakeRelativeCitPath(L"cache\\error_out").c_str());

	InitializeCriticalSectionAndSpinCount(&g_allocCS, 1000);

	g_mainThreadId = GetCurrentThreadId();

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

	// don't run north audio stuff in loading screens
	{
		// maybe
	}

	// NOP out any code that sets the 'entering state 2' (2, 0) FSM internal state to '7' (which is 'load game'), UNLESS it's digital distribution with standalone auth...
	char* p = (Is2060()) ? hook::pattern("BA 08 00 00 00 8D 41 FC 83 F8 01").count(1).get(0).get<char>(14) : hook::pattern("BA 07 00 00 00 8D 41 FC 83 F8 01").count(1).get(0).get<char>(14);

	char* varPtr = p + 2;
	g_initState = (int*)(varPtr + *(int32_t*)varPtr + 4);

	// check the pointer to see if it's digital distribution
	g_isDigitalDistrib = (p[-26] == 3);

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

		// also add a silly loop to state 6 ('wait for landing page'?)
		p = hook::pattern("C7 05 ? ? ? ? 06 00 00 00 EB 3F").count(1).get(0).get<char>(0);

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

	// for now, always reload the level in 'reload game' state, even if the current level did not change
	/*auto matches = hook::pattern("75 0F E8 ? ? ? ? 8B 0D ? ? ? ? 3B C8 74").count(2);

	for (int i = 0; i < matches.size(); i++)
	{
		if (i == 0)
		{
			void* call = matches.get(i).get<void>(17);

			hook::set_call(&g_deinitLevel, call);
			hook::call(call, DeinitLevel);
		}

		hook::nop(matches.get(i).get<void>(15), 2);
	}*/

	// fwmaptypesstore shutdown in CScene type 1 shutdown
	//hook::nop(hook::pattern("BB 01 00 00 00 8B CB E8 ? ? ? ? E8").count(1).get(0).get<void>(-0x5), 5);

	// redundant(?) path server init
	//hook::nop(hook::pattern("33 C9 E8 ? ? ? ? EB 02 32 DB 8A C3 48").count(1).get(0).get<void>(2), 5);

	// skip CModelInfo type 4 shutdown as the structure only gets initialized by core init
	//hook::put<uint16_t>(hook::pattern("83 F9 04 0F 85 ? 01 00 00 48 8D 1D").count(1).get(0).get<void>(3), 0xE990);

	// similar, fwmaptypesstore
	//hook::return_function(hook::pattern("FF 90 00 01 00 00 85 C0 7E 72 8B D7").count(1).get(0).get<void>(-0x12));

	// CModelInfo shutdown: also call destructor on said archetypes (as that's what those little bitches are in the end)
	//char* miPtr = hook::pattern("83 F9 04 0F 85 ? 01 00 00 48 8D 1D").count(1).get(0).get<char>(12);
	//*(int32_t*)miPtr = (int32_t)((char*)hook::AllocateFunctionStub(DestructMI) - miPtr - 4);

	// CVehicleRecording streaming module 'freeing' in shutdown 4
	//hook::nop(hook::pattern("48 83 EC 20 83 F9 04 75 11 48 8D 0D").count(1).get(0).get<void>(0x10), 5);

	// scene linked list
	char* loc = hook::pattern("41 D2 E0 41 F6 D9 48 8D 0D").count(1).get(0).get<char>(9);

	g_sceneLinkedList = (void*)(loc + *(int32_t*)loc + 4);

	// temp dbg: don't scan for platform dlc packs... it's mean.
	//hook::nop(hook::pattern("7C B4 48 8B CF E8").count(1).get(0).get<void>(5), 5);
	//hook::return_function(hook::pattern("7C B4 48 8B CF E8").count(1).get(0).get<void>(-0xA8));

	//hook::call(hook::pattern("B9 CD 36 41 A8 E8").count(1).get(0).get<void>(5), DebugBreakDo);
	//hook::jump(hook::get_call(hook::pattern("B9 CD 36 41 A8 E8").count(1).get(0).get<void>(5)), DebugBreakDo);

	char* errorFunc = reinterpret_cast<char*>(hook::get_call(hook::pattern("B9 84 EC F4 C6 E8").count(1).get(0).get<void>(5)));
	hook::jump(hook::get_call(errorFunc + 6), ErrorDo);
	hook::jump(errorFunc, ErrorDo);

	//hook::nop(hook::pattern("B9 CD 36 41 A8 E8").count(1).get(0).get<void>(0x14), 5);
	//hook::nop(hook::pattern("B9 CD 36 41 A8 E8").count(1).get(0).get<void>(5), 5);

	if (!CfxIsSinglePlayer())
	{
		// init function bit #1
		static InitFunctionStub initFunctionStub;
		initFunctionStub.Assemble();

		p = hook::pattern("41 8B CE FF 54 D0 08").count(1).get(0).get<char>(0);

		hook::nop(p, 7);
		hook::call_rcx(p, initFunctionStub.GetCode());

		// init function bit #2
		static InitFunctionStub2 initFunctionStub2;
		initFunctionStub2.Assemble();

		p = hook::pattern("41 8B CE FF 14 D0").count(1).get(0).get<char>(0);

		hook::nop(p, 6);
		hook::call_rcx(p, initFunctionStub2.GetCode());
	}

	char* location = hook::pattern("40 32 FF 45 84 C9 40 88 3D").count(1).get(0).get<char>(0x20);
	g_cacheArray = (atArray<allocWrap<uint32_t>>*)(location + *(int32_t*)location + 4);

	// bounds streaming module
	location = hook::pattern("83 F9 04 75 15 48 8D 0D").count(1).get(0).get<char>(8);

	g_boundsStreamingModule = (char*)(location + *(int32_t*)location + 4);

	// interior proxy pool
	location = hook::pattern("BA A1 85 94 52 41 B8 01").count(1).get(0).get<char>(0x34);

	g_interiorProxyPool = (decltype(g_interiorProxyPool))(location + *(int32_t*)location + 4);

	// unverified 'fix' for data pointer in fwMapTypesStore entries pointing to whatever structure being null upon free, however pool flags not having been nulled themselves
	//void* freeMapTypes = hook::pattern("45 8A CE 4C 8B 01 48 83 C2 10").count(1).get(0).get<void>(10);

	//hook::set_call(&g_origFreeMapTypes, freeMapTypes);
	//hook::call(freeMapTypes, DoFreeMapTypes);

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

	// attempted 'fix' for item #2 (also: logging when the destructor gets called)
	// 505 changed the jump address; so it's a pattern now
	/*location = hook::pattern("1C F6 83 A2 00 00 00 40 74 ? 48 8D 0D").count(1).get(0).get<char>(13);

	g_vehicleReflEntityArray = (decltype(g_vehicleReflEntityArray))(location + *(int32_t*)location + 4 + 800);
	g_unsafePointerLoc = (void**)((location + *(int32_t*)location + 4) + 800);*/

	// dlc get
	void* extraDataGetty = hook::pattern("45 33 F6 48 8B D8 48 85 C0 74 48").count(1).get(0).get<void>(0);

	location = hook::pattern("75 34 48 85 DB 75 34 B9 B0 09 00 00").count(1).get(0).get<char>(-9);

	g_extraContentMgr = (void**)(location + *(int32_t*)location + 4);

	// loading screen frame limit
	location = hook::pattern("0F 2F 05 ? ? ? ? 0F 82 E6 02 00 00").count(1).get(0).get<char>(3);

	hook::put<float>(location + *(int32_t*)location + 4, 0.0f);

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

	// kill GROUP_EARLY_ON DLC - this seems to make it unmount in a really weird way, and this was (as of 350) only ever used to mount platform:/patch_1/, a change
	// R* reverted through dlc_patches; yet the template setup2.xml was copied pretty much everywhere by then...
	//hook::put<uint32_t>(hook::pattern("41 B8 08 D4 8B 6F").count(1).get(0).get<void>(2), HashString("GROUP_REALLY_EARLY_ON"));

	// similar to above, except it's actually isLevelPack which matters - get rid of those, as these packs don't even add any 'levels'...
	//const uint8_t killInst[] = { 0x44, 0x88, 0xB6, 0xB0, 0x00, 0x00, 0x00 }; // mov byte ptr [rsi+0xb0], r15b

	//memcpy(hook::pattern("44 38 B6 B0 00 00 00 74 07").count(1).get(0).get<void>(9), killInst, sizeof(killInst));

	// mount dlc even if allegedly already mounted - bad bad idea
	//hook::nop(hook::pattern("84 C0 75 7A 48 8D 4C 24 20").count(1).get(0).get<void>(2), 2);

	// debug info for item #2 (generic free hook; might be useful elsewhere)
	//location = hook::pattern("48 89 01 83 61 48 00 48 8B C1 C3").count(1).get(0).get<char>(-4); // multiallocator
	location = hook::pattern("48 8B D9 41 8A E9 48 89 01 41 8B F8 48 63 F2 B9").count(1).get(0).get<char>(-4); // simpleallocator
	
	void** vt = (void**)(location + *(int32_t*)location + 4);

	g_origMemAlloc = (decltype(g_origMemAlloc))vt[2];
	//vt[2] = AllocEntry;
	
	g_origMemFree = (decltype(g_origMemFree))vt[4];
	//vt[4] = FreeEntry;

	g_origRealloc = (decltype(g_origRealloc))vt[6];
	//vt[6] = ReallocEntry;

	g_origIsMine = (decltype(g_origIsMine))vt[26];
	//vt[26] = isMineHook;

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

	// RELOADING ATTEMPT NOP
#if 0
	// temp dbg: do not load interior proxy ordering files
	hook::nop(hook::get_pattern("81 BA 90 00 00 00 AD 00 00 00 74 04", 10), 2);

	// temp dbg: do not even load interior proxy bounds (for DLC)
	hook::nop(hook::get_pattern("E8 ? ? ? ? 84 DB 74 12 E8", 0), 5);

	// do not even load DLC at all
	hook::nop(hook::get_pattern("B4 48 8B CF E8 ? ? ? ? 48 8D 4C 24 40 E8", 4), 5);
#endif

	// don't shut down DLC either
	//hook::return_function(hook::get_pattern("48 85 C9 74 3B 41 B8 01 00 00 00 8B D3 E8", -15));

	// nor rline really
	//hook::nop((void*)0x14001B429, 5);

	// or the other rline
	//hook::nop((void*)0x1400172ED, 5);

	// don't conditionally check player blip handle
	hook::call(hook::get_pattern("C8 89 05 ? ? ? ? E8 ? ? ? ? 89 05", 7), BlipAsIndex);

	// RELOADING ATTEMPT NOP
#if 0
	// clear popcycle file upon non-dlc load too
	{
		void* loc = hook::get_pattern("BA 0E 00 00 00 E8 ? ? ? ? 83 B8 94", 29);
		hook::set_call(&g_openPopCycle, loc);
		hook::call(loc, ClearAndOpenPopCycle);
	}

	// don't even try to set any popcycle zonebinds
	{
		hook::put<uint8_t>(hook::get_pattern("44 89 7C 24 60 81 FA DB 3E 14 7D 74 16", 11), 0xEB);
		hook::put<uint8_t>(hook::get_pattern("44 89 44 24 60 81 FA DB 3E 14 7D 74 16", 11), 0xEB);
	}

	// also don't reload clipsets because whoever implemented V DLC loading is a cuntflap
	hook::set_call(&g_loadClipSets, hook::get_pattern("45 33 E4 44 39 25 ? ? ? 00 75 0A E8", 12));
	//hook::return_function(hook::get_pattern("45 33 E4 44 39 25 ? ? ? 00 75 0A E8", -0x19));
	//hook::jump(hook::get_pattern("45 33 E4 44 39 25 ? ? ? 00 75 0A E8", -0x19), TrackClipSetShutdown);
#endif

	// disable fwclipsetmanager session shutdown by making it test for 9 shutdown
	//hook::put<uint8_t>(hook::get_pattern("83 F9 08 0F 85 26 01 00 00 44 0F", 2), 9);

	// track scanned render phases which may have interior proxies in portal/vis trackers
	location = hook::get_pattern<char>("66 89 83 08 05 00 00 48 8B C3 48 83 C4 30 5B C3", -36);

	hook::set_call(&CRenderPhase__ctor, location);
	hook::call(location, CRenderPhaseScanned__ctorWrap);

	// RELOADING ATTEMPT NOP
#if 0
	// don't switch clipset manager to network mode ever
	//hook::return_function(hook::get_pattern("A8 04 75 30 8B 04 13 4C", -0x3B));

	// dlc conditional anims? bah.
	hook::nop(hook::get_pattern("48 85 C0 75 38 8D 48 28", -14), 7);
#endif

	// argh.
	//hook::jump(0x1400012F4, AllocEntry);
	//hook::jump(0x14000132C, FreeEntry);

	/*
	void* setCache = hook::pattern("40 32 FF 45 84 C9 40 88 3D").count(1).get(0).get<void>(3);
	

	printf("");
	*/

	// second entry, in a single obfuscated stub
	/*auto obfEntries = hook::pattern("C7 05 ? ? ? ? 07 00 00 00 E9");

	for (int i = 0; i < obfEntries.size(); i++)
	{
		char* ptr = obfEntries.get(i).get<char>();
		int32_t relValue = *(int32_t*)(ptr + 2);

		// obfuscated functions are AFTER .data/.rdata, so this will usually be the case for this single stub
		if (relValue < 0)
		{
			hook::nop(ptr, 9);
		}
	}*/

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

	// commandline overriding stuff (replace a nullsub near sysParam_init)
	hook::call(hook::get_pattern("48 8B 54 24 48 8B 4C 24 40 E8", 25), OverrideArguments);

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
	hook::put<uint8_t>(hook::get_pattern("41 3B D9 72 09", 3), 0xEB);
});
// C7 05 ? ? ? ? 07 00  00 00 E9
