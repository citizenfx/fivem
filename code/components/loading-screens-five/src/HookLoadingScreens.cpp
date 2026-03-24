#include "StdInc.h"

#include "Hooking.h"
#include <gameSkeleton.h>

#include <stack>

#include <nutsnbolts.h>
#include <CefOverlay.h>
#include <DrawCommands.h>
#include "CoreConsole.h"

#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

#include <udis86.h>
#include <jitasm.h>

#include <optick.h>

static int g_instrumentedFuncs;

struct InstrumentedFuncMeta
{
	uint32_t tickCount;
	void* func;
	void* callFunc;
	int index;
};

static std::stack<InstrumentedFuncMeta> g_lastInstrumentedFuncs;

fwEvent<int, void*, void*> OnInstrumentedFunctionCall;

struct LoadScreenFuncs
{
	static void OnBegin(const InstrumentedFuncMeta& data)
	{
		OnInstrumentedFunctionCall(data.index, data.callFunc, data.func);
	}

	static void OnEnd(const InstrumentedFuncMeta& data)
	{
		if (data.index != 67 && data.index != 68)
		{
			auto msec = (timeGetTime() - data.tickCount);

			if (msec > 50)
			{
				trace(__FUNCTION__ ": Instrumented function %p (%i) took %dmsec\n", (void*)hook::get_unadjusted(data.func), data.index, msec);
			}
		}
	}
};

template<typename TFunctor>
struct InstrumentedCallStub : public jitasm::Frontend
{
	InstrumentedCallStub(int idx, uintptr_t originFunc, uintptr_t targetFunc)
		: m_index(idx), m_originFunc(originFunc), m_targetFunc(targetFunc)
	{

	}

	static void InstrumentedTarget(void* targetFn, void* callFn, int index)
	{
		if (!g_lastInstrumentedFuncs.empty())
		{
			auto data = g_lastInstrumentedFuncs.top();
			g_lastInstrumentedFuncs.pop();

			TFunctor::OnEnd(data);
		}

		InstrumentedFuncMeta meta;
		meta.tickCount = timeGetTime();
		meta.func = targetFn;
		meta.callFunc = callFn;
		meta.index = index;

		g_lastInstrumentedFuncs.push(meta);

		TFunctor::OnBegin(meta);
	}

	virtual void InternalMain() override
	{
		push(rcx);
		push(rdx);
		push(r8);
		push(r9);

		mov(rcx, m_targetFunc);
		mov(rdx, m_originFunc);
		mov(r8d, m_index);

		// scratch space (+ alignment for stack)
		sub(rsp, 0x28);

		mov(rax, (uintptr_t)InstrumentedTarget);
		call(rax);

		add(rsp, 0x28);

		pop(r9);
		pop(r8);
		pop(rdx);
		pop(rcx);

		mov(rax, m_targetFunc);
		jmp(rax);
	}

private:
	uintptr_t m_originFunc;
	uintptr_t m_targetFunc;

	int m_index;
};

template<typename TFunctor>
int InstrumentFunction(void* fn, std::vector<void*>& outFunctions)
{
	ud_t ud;
	ud_init(&ud);
	ud_set_mode(&ud, 64);

	// set the program counter
	ud_set_pc(&ud, reinterpret_cast<uint64_t>(fn));

	// set the input buffer
	ud_set_input_buffer(&ud, reinterpret_cast<uint8_t*>(fn), INT32_MAX);

	// return counter
	int& numFuncs = g_instrumentedFuncs;

	// loop the instructions
	while (true)
	{
		// disassemble the next instruction
		ud_disassemble(&ud);

		// if this is a retn, break from the loop
		if (ud_insn_mnemonic(&ud) == UD_Iret)
		{
			break;
		}
#ifdef GTA_FIVE // because rdr have WrapLevelLoad hook ):
		// if this is a 32-bit JMP, break as well
		if (ud_insn_mnemonic(&ud) == UD_Ijmp)
		{
			// get the first operand
			auto operand = ud_insn_opr(&ud, 0);

			// if the operand is immediate
			if (operand->type == UD_OP_JIMM && operand->size == 32)
			{
				break;
			}
		}
#endif
		if (ud_insn_mnemonic(&ud) == UD_Icall)
		{
			// get the first operand
			auto operand = ud_insn_opr(&ud, 0);

			// if the operand is immediate
			if (operand->type == UD_OP_JIMM)
			{
				// cast the relative offset as a char
				uintptr_t targetPtr = static_cast<uintptr_t>(ud_insn_len(&ud) + ud_insn_off(&ud) + operand->lval.sdword);
				uintptr_t callPtr   = static_cast<uintptr_t>(ud_insn_off(&ud));

				auto stub = new InstrumentedCallStub<TFunctor>(numFuncs, callPtr, targetPtr);
				hook::call(callPtr, stub->GetCode());

				outFunctions.push_back((void*)targetPtr);

				++numFuncs;
			}
		}
	}

	return numFuncs;
}

fwEvent<int, const char*> OnReturnDataFileEntry;

struct DataFileEntry
{
	char name[128];
	char pad[16]; // 128
	int32_t type; // 140
	int32_t index; // 148
	bool flag1; // 152
	bool flag2; // 153
	bool flag3; // 154
	bool disabled; // 155
#ifdef GTA_FIVE
	char pad2[12];
#elif IS_RDR3
	char pad2[20];
#endif
};

class CDataFileMgr;

CDataFileMgr* g_dataFileMgr;

class CDataFileMgr
{
private:
	atArray<DataFileEntry> m_entries;
#ifdef GTA_FIVE
	intptr_t pad[48 / 8];
#elif IS_RDR3
	intptr_t pad[56 / 8];
#endif

	DataFileEntry m_emptyEntry;

public:
	DataFileEntry* FindNextEntry(DataFileEntry* entry)
	{
		g_dataFileMgr = this;

		int nextIndex = entry->index + 1;

		int type = entry->type;

		for (int i = nextIndex; i < m_entries.GetCount(); i++)
		{
			entry = &m_entries[i];

			if (entry->type == type)
			{
				if (OnReturnDataFileEntry(type, entry->name))
				{
					return entry;
				}
			}
		}

		return &m_emptyEntry;
	}

	DataFileEntry* FindPreviousEntry(DataFileEntry* entry)
	{
		g_dataFileMgr = this;

		int nextIndex = entry->index;

		// TODO: replace with clamp function
		if (nextIndex >= 0)
		{
			if (nextIndex > m_entries.GetCount())
			{
				nextIndex = m_entries.GetCount();
			}
		}
		else
		{
			nextIndex = 0;
		}

		--nextIndex;

		int type = entry->type;

		for (int i = nextIndex; i >= 0; i--)
		{
			entry = &m_entries[i];

			if (entry->type == type)
			{
				if (OnReturnDataFileEntry(type, entry->name))
				{
					return entry;
				}
			}
		}

		return &m_emptyEntry;
	}

	template<typename T>
	auto CountEntries(const T& predicate)
	{
		return std::count_if(m_entries.begin(), m_entries.end(), predicate);
	}
};

int CountRelevantDataFileEntries()
{
	return g_instrumentedFuncs;
}



void (*g_updateContentArray)(void*);

int dlcIdx = -1;
int extraContentWrapperIdx = std::numeric_limits<int>::max();
void** g_extraContentManagerLocation;
std::map<uint32_t, std::string> g_dlcNameMap;

char g_extraContentManagerContentOffset;
uint32_t g_mountableContentSize;
uint32_t g_mountableContentFlagsOffset;
#ifdef GTA_FIVE
static void (*dataFileMgr__loadDefDat)(void*, const char*, bool);
static void LoadDefDats(void* dataFileMgr, const char* name, bool enabled)
{
	rage::InitFunctionData ifd;
	ifd.funcHash = HashRageString(name);
	ifd.initOrder = 3;
	ifd.shutdownOrder = 42;
	ifd.initFunction = [](int)
	{
	};

	std::string nameStripped = name;
	nameStripped = nameStripped.substr(0, nameStripped.find("CRC"));
	g_dlcNameMap.emplace(ifd.funcHash, nameStripped);

	if (dlcIdx >= 0)
	{
		rage::OnInitFunctionInvoking(rage::INIT_SESSION, extraContentWrapperIdx + dlcIdx, ifd);

		dlcIdx++;
	}

	dataFileMgr__loadDefDat(dataFileMgr, name, enabled);

	if (dlcIdx >= 0)
	{
		rage::OnInitFunctionInvoked(rage::INIT_SESSION, ifd);
	}
}
#elif IS_RDR3
static void (*dataFileMgr__loadDefDat)(void*, const char*, bool, void*);
static void LoadDefDats(void* dataFileMgr, const char* name, bool enabled, void* unk)
{
	rage::InitFunctionData ifd;
	ifd.funcHash = HashRageString(name);
	ifd.initOrder = 3;
	ifd.shutdownOrder = 42;
	ifd.initFunction = [](int) {
	};

	std::string nameStripped = name;
	nameStripped = nameStripped.substr(0, nameStripped.find("CRC"));
	g_dlcNameMap.emplace(ifd.funcHash, nameStripped);

	if (dlcIdx >= 0)
	{
		rage::OnInitFunctionInvoking(rage::INIT_SESSION, extraContentWrapperIdx + dlcIdx, ifd);

		dlcIdx++;
	}

	dataFileMgr__loadDefDat(dataFileMgr, name, enabled, unk);

	if (dlcIdx >= 0)
	{
		rage::OnInitFunctionInvoked(rage::INIT_SESSION, ifd);
	}
}
#endif
#ifdef GTA_FIVE
static int* loadingScreenState;
#endif
extern bool g_doDrawBelowLoadingScreens;

static HookFunction hookFunction([] ()
{
#ifdef GTA_FIVE
	{
		g_extraContentManagerLocation = hook::get_address<void**>(hook::get_pattern("79 91 C8 BC E8 ? ? ? ? 48 8D", -0x16));

		g_extraContentManagerContentOffset = *(char*)hook::get_pattern<char>("48 83 C1 ? 89 82", 3);
		g_mountableContentSize = *(uint32_t*)hook::get_pattern<char>("48 69 C0 ? ? ? ? 48 03 43 ? F6 80", 3);
		g_mountableContentFlagsOffset = *(uint32_t*)hook::get_pattern<char>("8A 81 ? ? ? ? 41 BC ? ? ? ? 48 8B F9", 2);

		g_updateContentArray = (void(*)(void*))hook::get_call(hook::get_pattern("E8 ? ? ? ? 44 8A C3 B2"));
	}

	hook::jump(hook::get_pattern("44 8B D8 4D 63 C8 4C 3B C8 7D 33 8B", -0x16), &CDataFileMgr::FindNextEntry);
	hook::jump(hook::get_pattern("4C 8B C9 45 85 C0 79 05 45 33 C0 EB 07 44", -0x0B), &CDataFileMgr::FindPreviousEntry);

	std::vector<void*> functions;
	InstrumentFunction<LoadScreenFuncs>(hook::get_call(hook::get_pattern("41 B0 01 48 8B D3 E8 ? ? ? ? E8", 24)), functions);

	InstrumentFunction<LoadScreenFuncs>(functions[4], functions);

	// don't play game loading music
	hook::return_function(hook::get_pattern("41 B8 97 96 11 96", -0x9A));

	auto hookPoint = hook::pattern("E8 ? ? ? ? 48 8B 1D ? ? ? ? 41 8B F7").count(1).get(0).get<void>(0);
	hook::set_call(&dataFileMgr__loadDefDat, hookPoint);
	hook::call(hookPoint, LoadDefDats); // Call the new function to load the handling files

#elif IS_RDR3
	{
		g_extraContentManagerLocation = hook::get_address<void**>(hook::get_pattern("48 8B 0D ? ? ? ? 44 8A C6 89 45 ? E8 ? ? ? ? 48 8B 0D", 0x3));
		g_extraContentManagerContentOffset = *(char*)hook::get_pattern<char>("48 83 C1 ? 03 D5", 3);
		g_mountableContentSize = *(uint32_t*)hook::get_pattern<char>("48 69 C8 ? ? ? ? 48 03 4F ? F6 81 ? ? ? ? ? 74 ? 8B 81", 3);
		g_mountableContentFlagsOffset = *(uint32_t*)hook::get_pattern<char>("8A 81 ? ? ? ? 41 BC ? ? ? ? 48 8B F9", 2);

		g_updateContentArray = (void (*)(void*))hook::get_call(hook::get_pattern("E8 ? ? ? ? 44 8A C3 B2 ? 48 8B CF E8 ? ? ? ? 48 8B 5C 24"));

	}
	// caled one time lol type 23 which its CTimecycle
	hook::jump(hook::get_pattern("44 8B 82 ? ? ? ? 44 0F B7 51"), &CDataFileMgr::FindNextEntry);

	std::vector<void*> functions;
	InstrumentFunction<LoadScreenFuncs>(hook::get_call(hook::get_pattern("E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 8B 1D")), functions);

	InstrumentFunction<LoadScreenFuncs>(functions[1], functions);
	InstrumentFunction<LoadScreenFuncs>(functions[2], functions);

	// don't play game loading music
	hook::return_function(hook::get_pattern("41 BC ? ? ? ? 45 8B F0", -0x21));

	auto hookPoint = hook::pattern("E8 ? ? ? ? 48 8B 1D ? ? ? ? 41 8B F7").count(1).get(0).get<void>(0);
	hook::set_call(&dataFileMgr__loadDefDat, hookPoint);
	hook::call(hookPoint, LoadDefDats); // Call the new function to load the handling files
#endif
#ifdef GTA_FIVE
	// loading screen state 10 draws postFX every frame, which will make for a lot of unneeded GPU load below NUI
	loadingScreenState = hook::get_address<int*>(hook::get_pattern("83 3D ? ? ? ? 05 75 ? 8B"), 2, 7);

	OnGameFrame.Connect([]()
	{
		if (*loadingScreenState == 10)
		{
			if (nui::HasMainUI() || g_doDrawBelowLoadingScreens)
			{
				*loadingScreenState = 6;
			}
		}
	});

#if USE_OPTICK
	struct ProfilerMetaData
	{
		Optick::EventDescription* desc;
		std::unique_ptr<Optick::Event> ev;
	};

	static std::unordered_map<void*, ProfilerMetaData> g_profilerMap;

	struct ProfilerFuncs
	{
		static void OnBegin(const InstrumentedFuncMeta& func)
		{
			if (func.func == (void*)0x1407A948C)
			{
				return;
			}

			auto it = g_profilerMap.find(func.func);

			if (it == g_profilerMap.end())
			{
				it = g_profilerMap.emplace(func.func, ProfilerMetaData{ Optick::EventDescription::Create(strdup(va("ProfileFunc %llx", (uintptr_t)func.func)), __FILE__, __LINE__, Optick::Color::Red), nullptr }).first;
			}

			it->second.ev = std::make_unique<Optick::Event>(*it->second.desc);
		}

		static void OnEnd(const InstrumentedFuncMeta& func)
		{
			if (func.func == (void*)0x1407A948C)
			{
				return;
			}

			auto it = g_profilerMap.find(func.func);

			if (it != g_profilerMap.end())
			{
				it->second.ev = nullptr;
			}
		}
	};

	std::vector<void*> proFunctions;
	InstrumentFunction<ProfilerFuncs>(hook::get_pattern("B9 05 00 00 00 E8 ? ? ? ? 48 8D 0D", -0x13), proFunctions);
	InstrumentFunction<ProfilerFuncs>(proFunctions[2], proFunctions);
	InstrumentFunction<ProfilerFuncs>(proFunctions[7], proFunctions);

	// CStreaming::Update
	// don't use it tends to break the profiler output
	//InstrumentFunction<ProfilerFuncs>(hook::get_pattern("BF 01 00 00 00 84 C0 75 23 38 1D ? ? ? ? 75", -0x51), proFunctions);
#endif
#endif
});
#ifdef GTA_FIVE
static hook::cdecl_stub<void(float)> _drawLoadingSpinner([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 83 3D ? ? ? ? 00 74 0C E8 ? ? ? ? EB 05"));
});

static bool* g_isPendingGFx;
static void** g_scaleformMgr;
static void (*g_render)(void*);

static void (*setupBusySpinner)(bool);
static void (*updateBusySpinner)();
static bool* spinnerDep;

// CScaleformMgr::UpdateAtEndOfFrame
static hook::cdecl_stub<void(bool)> _sf_updateEndOfFrame([]()
{
	return hook::get_pattern("45 33 FF 44 8A F1 44 38 ? ? ? ? 01 0F 85", -0x18);
});

static HookFunction hookFunctionSpinner([]()
{
	// set up GFx bits
	{
		auto location = hook::get_pattern<char>("74 1A 80 3D ? ? ? ? 00 74 11 E8 ? ? ? ? 48 8B");
		g_isPendingGFx = hook::get_address<bool*>(location, 4, 9);
		g_scaleformMgr = hook::get_address<void**>(location + 19);
		hook::set_call(&g_render, location + 0x17);
	}

	// spinner setup deps
	{
		auto location = hook::get_pattern("B1 01 E8 ? ? ? ? 48 8D 0D ? ? ? ? 48 8B ? E8", 2);
		hook::set_call(&setupBusySpinner, location);
	}

	{
		auto location = hook::get_pattern<char>("77 15 C6 05 ? ? ? ? 01 E8", 2);
		spinnerDep = hook::get_address<bool*>(location, 2, 7);
		hook::set_call(&updateBusySpinner, location + 7);
	}

	// don't ignore spinner
	auto p = hook::get_call(hook::get_pattern<char>("E8 ? ? ? ? 83 3D ? ? ? ? 00 74 0C E8 ? ? ? ? EB 05"));
	hook::nop(p + 0x1D, 6);
});

static InitFunction initFunction([] ()
{
	static bool showBusySpinner = true;
	static ConVar showBusySpinnerConVar("sv_showBusySpinnerOnLoadingScreen", ConVar_Replicated, true, &showBusySpinner);

	OnLookAliveFrame.Connect([]()
	{
		if (nui::HasFrame("loadingScreen") && showBusySpinner)
		{
			if (*g_scaleformMgr)
			{
				if (GetCurrentThreadId() != g_mainThreadId)
				{
					trace("Error: BusySpinner update called from non-main thread!\n");
					return;
				}

				setupBusySpinner(1);
				*spinnerDep = true;
				updateBusySpinner();

				_sf_updateEndOfFrame(false);
			}
		}
	});
		
	OnPostFrontendRender.Connect([]()
	{
		if (nui::HasFrame("loadingScreen") && showBusySpinner)
		{
			*g_isPendingGFx = true;

			if (*g_scaleformMgr)
			{
				static auto timeStart = std::chrono::high_resolution_clock::now().time_since_epoch();
				auto timeSpan = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch() - timeStart);
				_drawLoadingSpinner(timeSpan.count() / 1000.0 + 4);

				g_render(*g_scaleformMgr);
			}
		}
	}, 5000);
});
#endif
