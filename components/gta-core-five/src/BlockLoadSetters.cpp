/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"

#include <atArray.h>
#include <sysAllocator.h>

#include <GameInit.h>
#include <nutsnbolts.h>

static hook::cdecl_stub<void()> lookAlive([] ()
{
	return hook::pattern("48 8D 6C 24 A0 48 81 EC 60 01 00 00 E8").count(1).get(0).get<void>(-0xC);
});

static int* g_initState;

bool g_shouldSetState;
bool g_isInInitLoop; // to avoid some GFx crash?

static void WaitForInitLoop()
{
	g_isInInitLoop = true;

	while (*g_initState <= 6)
	{
		lookAlive();

		if (*g_initState <= 6)
		{
			Sleep(15);
		}
	}

	//*g_initState = 7;
}

static void WaitForInitLoopWrap()
{
	*g_initState = 6;

	WaitForInitLoop();
}

void FiveGameInit::LoadGameFirstLaunch(bool(*callBeforeLoad)())
{
	OnGameFrame.Connect([=] ()
	{
		if (g_shouldSetState)
		{
			if (*g_initState == 6)
			{
				*g_initState = 7;

				g_shouldSetState = false;
			}
		}

		static bool isLoading = false;

		if (isLoading)
		{
			if (*g_initState == 0)
			{
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

	// stuff
	if (*g_initState == 6)
	{
		*g_initState = 7;
	}
	else
	{
		if (*g_initState == 20)
		{
			*g_initState = 11;
		}

		g_shouldSetState = true;
	}
}

void FiveGameInit::ReloadGame()
{
	*g_initState = 14;
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
		trace("Running shutdown %s function: %p\n", typeMap[type], stub);
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

template<typename T>
class fwPool
{
private:
	char* m_data;
	int8_t* m_flags;
	uint32_t m_count;
	uint32_t m_entrySize;

public:
	T* GetAt(int index) const
	{
		return reinterpret_cast<T*>(m_data + (index * m_entrySize));
	}

	void Clear()
	{
		for (int i = 0; i < m_count; i++)
		{
			if (m_flags[i] >= 0)
			{
				delete GetAt(i);
			}
		}
	}
};

static fwPool<CInteriorProxy>** g_interiorProxyPool;

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

		initedLateHook = true;
	}

	// stuff
	g_inLevelFree = true;

	g_deinitLevel();

	shutdownModelInfo(1);
	initModelInfo(1);
	initStreamingInterface();

	// extra content manager shutdown session removes these, and we want these before init session, so we scan them right here, right now.
	extraContentMgr__doScanInt(*g_extraContentMgr);
	extraContentMgr__doScanPost(*g_extraContentMgr, false, false);

	//shutdownPhysics(1);
	//initPhysics(1);

	// unknown value in the bounds streaming module, doesn't get cleared on 'after map loaded' shutdown
	*(uint32_t*)(g_boundsStreamingModule + 5664) = 0;

	// bounds streaming module, 'has preloading bounds completed' value
	*(uint8_t*)(g_boundsStreamingModule + 255) = 0;

	// clear the 'loaded cache hashes' list
	*g_cacheArray = atArray<allocWrap<uint32_t>>(16);

	// free one CScene list of all the bad influences from the last session
	ClearInteriorProxyList(g_sceneLinkedList);

	// also clear the interior proxy pool out, as it might contain garbage references to static bounds, still
	(*g_interiorProxyPool)->Clear();

	// and some global vehicle audio entity also houses... interior proxies.
	*g_vehicleReflEntityArray = atArray<CInteriorProxy*>();

	g_inLevelFree = false;

	if (!g_didLevelFree)
	{
		/*for (auto& stack : g_stacks)
		{
			FILE* f = fopen(va("D:\\dev\\stacks\\%p.txt", stack.first), "w");

			if (f)
			{
				for (auto& entry : stack.second)
				{
					fprintf(f, "%p\n", entry);
				}

				fclose(f);
			}
		}

		g_stackIdx = 0;
		g_stacks.clear();*/

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

static void(*g_origMemFree)(void*, void*);
static void** g_unsafePointerLoc;

#include <unordered_map>
#include <mutex>

static std::map<void*, size_t> g_allocData;
static std::mutex g_allocMutex;

static std::vector<uintptr_t> g_unsafeStack;

void CustomMemFree(void* allocator, void* pointer)
{
	if (pointer != nullptr && *g_unsafePointerLoc)
	{
		/*__debugbreak();

		assert(!"Tried to free unsafe pointer!");*/

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

	}

	/*if (!g_didLevelFree && pointer != nullptr)
	{
		std::unique_lock<std::mutex> lock(g_allocMutex);

		if (g_inLevelFree)
		{
			size_t allocSize = g_allocData[pointer];

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
		}

		g_allocData.erase(pointer);
	}*/

	return g_origMemFree(allocator, pointer);
}

static void*(*g_origMemAlloc)(void*, int size, int align, int subAlloc);

void* CustomMemAlloc(void* allocator, int size, int align, int subAlloc)
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

	if (!g_didLevelFree)
	{
		std::unique_lock<std::mutex> lock(g_allocMutex);
		g_allocData[ptr] = size;
	}

	return ptr;
}

template<int Value>
static int ReturnInt()
{
	return Value;
}

static HookFunction hookFunction([] ()
{
	// NOP out any code that sets the 'entering state 2' (2, 0) FSM internal state to '7' (which is 'load game'?)
	char* p = hook::pattern("BA 07 00 00 00 8D 41 FC 83 F8 01").count(1).get(0).get<char>(14);

	char* varPtr = p + 2;
	g_initState = (int*)(varPtr + *(int32_t*)varPtr + 4);

	hook::nop(p, 6);

	// and call our little internal loop function from there
	hook::call(p, WaitForInitLoop);

	// also add a silly loop to state 6 ('wait for landing page'?)
	p = hook::pattern("C7 05 ? ? ? ? 06 00 00 00 EB 3F").count(1).get(0).get<char>(0);

	hook::nop(p, 10);
	hook::call(p, WaitForInitLoopWrap);

	// for now, always reload the level in 'reload game' state, even if the current level did not change
	auto matches = hook::pattern("75 0F E8 ? ? ? ? 8B 0D ? ? ? ? 3B C8 74").count(2);

	for (int i = 0; i < matches.size(); i++)
	{
		if (i == 0)
		{
			void* call = matches.get(i).get<void>(17);

			hook::set_call(&g_deinitLevel, call);
			hook::call(call, DeinitLevel);
		}

		hook::nop(matches.get(i).get<void>(15), 2);
	}

	// fwmaptypesstore shutdown in CScene type 1 shutdown
	//hook::nop(hook::pattern("BB 01 00 00 00 8B CB E8 ? ? ? ? E8").count(1).get(0).get<void>(-0x5), 5);

	// redundant(?) path server init
	//hook::nop(hook::pattern("33 C9 E8 ? ? ? ? EB 02 32 DB 8A C3 48").count(1).get(0).get<void>(2), 5);

	// skip CModelInfo type 4 shutdown as the structure only gets initialized by core init
	//hook::put<uint16_t>(hook::pattern("83 F9 04 0F 85 ? 01 00 00 48 8D 1D").count(1).get(0).get<void>(3), 0xE990);

	// similar, fwmaptypesstore
	//hook::return_function(hook::pattern("FF 90 00 01 00 00 85 C0 7E 72 8B D7").count(1).get(0).get<void>(-0x12));

	// CModelInfo shutdown: also call destructor on said archetypes (as that's what those little bitches are in the end)
	char* miPtr = hook::pattern("83 F9 04 0F 85 ? 01 00 00 48 8D 1D").count(1).get(0).get<char>(12);
	*(int32_t*)miPtr = (int32_t)((char*)hook::AllocateFunctionStub(DestructMI) - miPtr - 4);

	// CVehicleRecording streaming module 'freeing' in shutdown 4
	hook::nop(hook::pattern("48 83 EC 20 83 F9 04 75 11 48 8D 0D").count(1).get(0).get<void>(0x10), 5);

	// scene linked list
	char* loc = hook::pattern("41 D2 E0 41 F6 D9 48 8D 0D").count(1).get(0).get<char>(9);

	g_sceneLinkedList = (void*)(loc + *(int32_t*)loc + 4);

	// temp dbg: don't scan for platform dlc packs... it's mean.
	//hook::nop(hook::pattern("7C B4 48 8B CF E8").count(1).get(0).get<void>(5), 5);
	//hook::return_function(hook::pattern("7C B4 48 8B CF E8").count(1).get(0).get<void>(-0xA8));

	//hook::call(hook::pattern("B9 CD 36 41 A8 E8").count(1).get(0).get<void>(5), DebugBreakDo);
	hook::jump(hook::get_call(hook::pattern("B9 CD 36 41 A8 E8").count(1).get(0).get<void>(5)), DebugBreakDo);
	hook::nop(hook::pattern("B9 CD 36 41 A8 E8").count(1).get(0).get<void>(0x14), 5);
	hook::nop(hook::pattern("B9 CD 36 41 A8 E8").count(1).get(0).get<void>(5), 5);

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

	char* location = hook::pattern("40 32 FF 45 84 C9 40 88 3D").count(1).get(0).get<char>(0x20);
	g_cacheArray = (atArray<allocWrap<uint32_t>>*)(location + *(int32_t*)location + 4);

	// bounds streaming module
	location = hook::pattern("83 F9 04 75 15 48 8D 0D").count(1).get(0).get<char>(8);

	g_boundsStreamingModule = (char*)(location + *(int32_t*)location + 4);

	// interior proxy pool
	location = hook::pattern("BA A1 85 94 52 41 B8 01").count(1).get(0).get<char>(0x34);

	g_interiorProxyPool = (decltype(g_interiorProxyPool))(location + *(int32_t*)location + 4);

	// unverified 'fix' for data pointer in fwMapTypesStore entries pointing to whatever structure being null upon free, however pool flags not having been nulled themselves
	void* freeMapTypes = hook::pattern("45 8A CE 4C 8B 01 48 83 C2 10").count(1).get(0).get<void>(10);

	hook::set_call(&g_origFreeMapTypes, freeMapTypes);
	hook::call(freeMapTypes, DoFreeMapTypes);

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
	location = hook::pattern("F6 83 A2 00 00 00 40 74 17 48 8D 0D").count(1).get(0).get<char>(12);

	g_vehicleReflEntityArray = (decltype(g_vehicleReflEntityArray))(location + *(int32_t*)location + 4 + 800);
	g_unsafePointerLoc = (void**)((location + *(int32_t*)location + 4) + 800);

	// debug info for item #2 (generic free hook; might be useful elsewhere)
	location = hook::pattern("48 89 01 83 61 48 00 48 8B C1 C3").count(1).get(0).get<char>(-4);

	// dlc get
	void* extraDataGetty = hook::pattern("45 33 F6 48 8B D8 48 85 C0 74 48").count(1).get(0).get<void>(0);

	printf("");

	location = hook::pattern("75 34 48 85 DB 75 34 B9 B0 09 00 00").count(1).get(0).get<char>(-9);

	g_extraContentMgr = (void**)(location + *(int32_t*)location + 4);

	// loading screen frame limit
	location = hook::pattern("0F 2F 05 ? ? ? ? 0F 82 E6 02 00 00").count(1).get(0).get<char>(3);

	hook::put<float>(location + *(int32_t*)location + 4, 1000.0f / 60.0f);

	// bypass the state 20 calibration screen loop (which might be wrong; it doesn't seem to exist in my IDA dumps of 323/331 Steam)
	matches = hook::pattern("E8 ? ? ? ? 8A D8 84 C0 74 0E C6 05");

	assert(matches.size() <= 1);

	for (int i = 0; i < matches.size(); i++)
	{
		hook::call(matches.get(i).get<void>(), ReturnInt<1>);
	}

	// mount dlc even if allegedly already mounted - bad bad idea
	//hook::nop(hook::pattern("84 C0 75 7A 48 8D 4C 24 20").count(1).get(0).get<void>(2), 2);

	/*
	void** vt = (void**)(location + *(int32_t*)location + 4);

	g_origMemAlloc = (decltype(g_origMemAlloc))vt[2];
	vt[2] = CustomMemAlloc;

	g_origMemFree = (decltype(g_origMemFree))vt[4];
	vt[4] = CustomMemFree;
	*/

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
});
// C7 05 ? ? ? ? 07 00  00 00 E9