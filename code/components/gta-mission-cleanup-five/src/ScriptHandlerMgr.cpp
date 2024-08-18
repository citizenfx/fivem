/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ScriptHandlerMgr.h>

#include "Hooking.h"

#include "Pool.h"

#include <MinHook.h>

#include <CrossBuildRuntime.h>

#include <Error.h>

// pool functions, here temporarily we hope :)
static atPoolBase** g_scriptHandlerNetworkPool;

static CGameScriptHandlerMgr* g_scriptHandlerMgr;

static rage::scrThread*(*g_origGetThreadById)(uint32_t hash);

static std::map<uint32_t, rage::scrThread*> g_customThreads;
static std::map<uint32_t, std::string> g_customThreadsToNames;

static bool* CTheScripts__ms_bUpdatingScriptThreads;

UpdatingScriptThreadsScope::UpdatingScriptThreadsScope(bool newState)
{
	m_lastProcessTick = *CTheScripts__ms_bUpdatingScriptThreads;
	*CTheScripts__ms_bUpdatingScriptThreads = newState;
}

UpdatingScriptThreadsScope::~UpdatingScriptThreadsScope()
{
	if (m_lastProcessTick)
	{
		*CTheScripts__ms_bUpdatingScriptThreads = *m_lastProcessTick;
	}
}

static rage::scrThread* GetThreadById(uint32_t hash)
{
	auto it = g_customThreads.find(hash);

	if (it != g_customThreads.end())
	{
		return it->second;
	}

	return g_origGetThreadById(hash);
}

struct CGameScriptId
{
	void* vtbl;
	uint32_t hash;
	char scriptName[32];
};

static void (*g_origCGameScriptId__updateScriptName)(CGameScriptId* self);

static void CGameScriptId__updateScriptName(CGameScriptId* self)
{
	self->scriptName[0] = '\0';
	g_origCGameScriptId__updateScriptName(self);

	if (self->scriptName[0] == '\0')
	{
		auto thread = g_customThreadsToNames.find(self->hash);

		if (thread != g_customThreadsToNames.end())
		{
			strcpy_s(self->scriptName, thread->second.c_str());
		}
	}
}

DLL_EXPORT fwEvent<rage::scrThread*, const std::string&> OnCreateResourceThread;
DLL_EXPORT fwEvent<rage::scrThread*> OnDeleteResourceThread;

// find data fields and perform patches
static HookFunction hookFunction([] ()
{
	char* location = hook::pattern("BA E8 10 E8 4F 41").count(1).get(0).get<char>(52);

	g_scriptHandlerNetworkPool = (atPoolBase**)(location + *(int32_t*)location + 4);

	// rage::scriptHandler destructor does something incredibly stupid - the vtable gets set to the base as usual, but then the 'custom' code
	// in the case when we execute it decides to call GetScriptId, which is a __purecall in rage::scriptHandler.

	// therefore, we patch that check out to never execute.
	hook::put<uint8_t>(hook::pattern("80 78 32 00 75 34 B1 01 E8").count(1).get(0).get<void>(4), 0xEB);

	// find CGameScriptHandlerMgr pointer
	location = hook::pattern("48 8D 55 17 48 8D 0D ? ? ? ? FF").count(1).get(0).get<char>(7);

	g_scriptHandlerMgr = (CGameScriptHandlerMgr*)(location + *(int32_t*)location + 4);

	// script threads for dummies
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("33 D2 44 8B C1 85 C9 74 2B 0F"), GetThreadById, (void**)&g_origGetThreadById);
	MH_CreateHook(hook::get_pattern("48 8D 44 24 30 44 39 10 74 20 8B 10 48 8D 0D ? ? ? ? E8", -0xA6), CGameScriptId__updateScriptName, (void**)&g_origCGameScriptId__updateScriptName);
	MH_EnableHook(MH_ALL_HOOKS);

	OnCreateResourceThread.Connect([](rage::scrThread* thread, const std::string& name)
	{
		g_customThreads.insert({ thread->GetContext()->ThreadId, thread });
		g_customThreadsToNames.insert({ thread->GetContext()->ThreadId, name });
	});

	OnDeleteResourceThread.Connect([](rage::scrThread* thread)
	{
		g_customThreads.erase(thread->GetContext()->ThreadId);
		g_customThreadsToNames.erase(thread->GetContext()->ThreadId);
	});

	if (xbr::IsGameBuildOrGreater<2372>())
	{
		// FF 50 18 48 8B CE 48 8B D0 E8 ? ? ? ? 84
		// FF 50 18                call    qword ptr [rax+18h]
		// 48 8B CE                mov     rcx, rsi
		// 48 8B D0                mov     rdx, rax
		// E8 1F 97 FF FF          call    sub_7FF713538288
		// 84 C0                   test    al, al
		// 0F 84 AC 01 00 00       jz      loc_7FF71353ED1D   <---------------
		hook::nop(hook::get_pattern("FF 50 18 48 8B CE 48 8B D0 E8 ? ? ? ? 84", 16), 6);
	}
	else if (xbr::IsGameBuildOrGreater<2060>())
	{
		// [74] 19  jz short loc_140A3ABA3 -- 0xEB unconditional jmp
		hook::put<uint8_t>(hook::get_pattern("74 19 FF C1 48 83 C0 04"), 0xEB);
	}
});

// functions
static hook::thiscall_stub<void(CGameScriptHandlerNetwork*, rage::scrThread*)> scriptHandlerNetwork__ctor([] ()
{
	return hook::pattern("33 C0 48 89 83 A0 00 00 00 89 83 A8 00 00 00 66 89 83 AC 00").count(1).get(0).get<void>(-0x18);
});

void* CGameScriptHandlerNetwork::operator new(size_t size)
{
	return rage::PoolAllocate(*g_scriptHandlerNetworkPool);
}

CGameScriptHandlerNetwork::CGameScriptHandlerNetwork(rage::scrThread* thread)
{
	scriptHandlerNetwork__ctor(this, thread);
}

static hook::thiscall_stub<void(void*, uint32_t*, void*)> setHashMap([] ()
{
	return hook::get_call(hook::pattern("48 8D 54 24 50 48 8B CE 89 44 24 50 E8").count(1).get(0).get<void>(12));
});

void CGameScriptHandlerMgr::scriptHandlerHashMap::Set(uint32_t* hash, rage::scriptHandler** handler)
{
	return setHashMap(this, hash, handler);
}

static void(*g_origDetachScript)(void*, void*);
void WrapDetachScript(void* a1, void* script)
{
	// sometimes scripts here are on the C++ side, which use a copied scripthandler from another script
	// these will except as they're _already_ freed, so we catch that exception here
	//__try
	{
		g_origDetachScript(a1, script);
	}
	//__except (EXCEPTION_EXECUTE_HANDLER)
	//{
//		trace("CGameScriptHandlerMgr::DetachScript() excepted, caught and returned.\n");
	//}
}

#include <mutex>

CGameScriptHandlerMgr* CGameScriptHandlerMgr::GetInstance()
{
	return g_scriptHandlerMgr;
}

// implemented parent functions for shutting up the compiler
namespace rage
{
	scriptHandler::~scriptHandler()
	{

	}

	scriptHandlerImplemented::~scriptHandlerImplemented()
	{
		FatalError(__FUNCTION__);
	}

	void scriptHandlerImplemented::CleanupObjectList()
	{
		FatalError(__FUNCTION__);
	}

	void scriptHandlerImplemented::m_8()
	{
		FatalError(__FUNCTION__);
	}

	void scriptHandlerImplemented::m_10()
	{
		FatalError(__FUNCTION__);
	}

	scriptId* scriptHandlerImplemented::GetScriptId()
	{
		FatalError(__FUNCTION__);

		return nullptr;
	}

	scriptId* scriptHandlerImplemented::GetScriptId_2()
	{
		FatalError(__FUNCTION__);

		return nullptr;
	}

	bool scriptHandlerImplemented::IsNetworkScript()
	{
		FatalError(__FUNCTION__);

		return false;
	}
}

static HookFunction hookFunctionVtbl([]()
{
	{
		auto vtable = hook::get_address<uintptr_t*>(hook::get_pattern("41 83 C8 FF 48 89 03 89 53 70 88 53 74 4C 89 4B", -11));

		g_origDetachScript = ((decltype(g_origDetachScript))vtable[11]);
		hook::put<uintptr_t>(&vtable[11], (uintptr_t)WrapDetachScript);
	}

	CTheScripts__ms_bUpdatingScriptThreads = hook::get_address<bool*>(hook::get_pattern("45 33 F6 41 8A F0 8B EA 44 38 35", 11));
});
