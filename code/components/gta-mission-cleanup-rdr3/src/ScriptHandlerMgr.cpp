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

#include <Error.h>

static CGameScriptHandlerMgr* g_scriptHandlerMgr;

static std::map<uint32_t, rage::scrThread*> g_customThreads;
static std::map<uint32_t, std::string> g_customThreadsToNames;

static rage::scrThread* (*g_origGetThreadById)(uint32_t hash);

static rage::scrThread* GetThreadById(uint32_t hash)
{
	auto it = g_customThreads.find(hash);

	if (it != g_customThreads.end())
	{
		return it->second;
	}

	return g_origGetThreadById(hash);
}

DLL_EXPORT fwEvent<rage::scrThread*, const std::string&> OnCreateResourceThread;
DLL_EXPORT fwEvent<rage::scrThread*> OnDeleteResourceThread;

// find data fields and perform patches
static HookFunction hookFunction([]()
{
	// find CGameScriptHandlerMgr pointer
	g_scriptHandlerMgr = hook::get_address<CGameScriptHandlerMgr*>(hook::pattern("48 8D 96 ? ? 00 00 48 C1 E0 07 48 8D 0D").count(1).get(0).get<char>(14));

	// script threads for dummies
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("44 8B C2 4B 8B 04 C2 39 48 08", -26), GetThreadById, (void**)&g_origGetThreadById);
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
});

// functions
static hook::thiscall_stub<void(CGameScriptHandlerNetwork*, rage::scrThread*)> scriptHandlerNetwork__ctor([]()
{
	return hook::pattern("24 ? C6 87 DC 00 00 00 00 0C 01").count(1).get(0).get<void>(-0x39);
});

void* CGameScriptHandlerNetwork::operator new(size_t size)
{
	auto pool = rage::GetPoolBase("CGameScriptHandlerNetwork");
	return rage::PoolAllocate(pool);
}

CGameScriptHandlerNetwork::CGameScriptHandlerNetwork(rage::scrThread* thread)
{
	scriptHandlerNetwork__ctor(this, thread);
}

static hook::thiscall_stub<void(void*, uint32_t*, void*)> setHashMap([]()
{
	return hook::get_call(hook::pattern("F3 48 0F 2C C2 89 45 B8 E8").count(1).get(0).get<void>(8));
});

void CGameScriptHandlerMgr::scriptHandlerHashMap::Set(uint32_t *hash, rage::scriptHandler **handler)
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
		auto vtable = hook::get_address<uintptr_t*>(hook::get_pattern("BD ? ? ? ? 48 89 07 48 8D 9F ? ? ? ? 83 67", -4));

		g_origDetachScript = ((decltype(g_origDetachScript))vtable[11]);
		vtable[11] = (uintptr_t)WrapDetachScript;
	}
});
