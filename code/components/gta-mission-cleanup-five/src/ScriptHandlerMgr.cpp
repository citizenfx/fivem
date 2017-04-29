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

#include <Error.h>

// pool functions, here temporarily we hope :)
static atPoolBase** g_scriptHandlerNetworkPool;

static CGameScriptHandlerMgr* g_scriptHandlerMgr;

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
});

// functions
static hook::thiscall_stub<void(CGameScriptHandlerNetwork*, rage::scrThread*)> scriptHandlerNetwork__ctor([] ()
{
	return hook::pattern("33 C0 48 89 83 A0 00 00 00 66 89 83 A8").count(1).get(0).get<void>(-0x18);
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
	__try
	{
		g_origDetachScript(a1, script);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		trace("CGameScriptHandlerMgr::DetachScript() excepted, caught and returned.\n");
	}
}

#include <mutex>

CGameScriptHandlerMgr* CGameScriptHandlerMgr::GetInstance()
{
	// if the vtable is set
	if (*(uintptr_t*)g_scriptHandlerMgr)
	{
		static std::once_flag of;

		std::call_once(of, []()
		{
			uintptr_t* vtable = *(uintptr_t**)g_scriptHandlerMgr;
			g_origDetachScript = ((decltype(g_origDetachScript))vtable[11]);
			vtable[11] = (uintptr_t)WrapDetachScript;
		});
	}

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