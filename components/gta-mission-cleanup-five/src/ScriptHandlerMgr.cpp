/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ScriptHandlerMgr.h>

#include "Hooking.h"

// pool functions, here temporarily we hope :)
struct CPool
{

};

static CPool** g_scriptHandlerNetworkPool;

static hook::thiscall_stub<void*(CPool*)> poolAllocate([] ()
{
	return hook::pattern("4C 8B D1 48 63 49 18 83 F9 FF 75 03").count(1).get(0).get<void>();
});

static CGameScriptHandlerMgr* g_scriptHandlerMgr;

// find data fields and perform patches
static HookFunction hookFunction([] ()
{
	char* location = hook::pattern("BA E8 10 E8 4F 41").count(1).get(0).get<char>(52);

	g_scriptHandlerNetworkPool = (CPool**)(location + *(int32_t*)location + 4);

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
	return poolAllocate(*g_scriptHandlerNetworkPool);
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