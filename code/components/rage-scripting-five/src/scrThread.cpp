/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

// This file contains code adapted from the original GTA IV script hook, the
// copyright notice for which follows below.

/*****************************************************************************\

Copyright (C) 2009, Aru <oneforaru at gmail dot com>

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

\*****************************************************************************/

#include "StdInc.h"
#include "scrThread.h"
#include "scrEngine.h"
#include "Hooking.h"

#include <ScriptHandlerMgr.h>

#include <CrossBuildStruct.h>

namespace rage
{
template<int Version, typename = void>
class scrThreadVersion
{
protected:
	scrThreadContext m_Context;
	void* m_pStack; // should be +176 including vtable
	void* pad;
	void* pad2;

	// should be +200
	const char* m_pszExitMessage;

public:
	virtual ~scrThreadVersion() = default;
	virtual eThreadState Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount) = 0;
	virtual eThreadState Run(uint32_t opsToExecute) = 0;
	virtual eThreadState Tick(uint32_t opsToExecute) = 0;
	virtual void Kill() = 0;
};

template<int Version>
class scrThreadVersion<Version, VersionBetween<Version, 3407, xbr::Build::Summer_2025>>
{
protected:
	scrThreadContext m_Context;
	void* m_pStack; // should be +176 including vtable
	void* pad;
	void* pad2;

	// should be +200
	const char* m_pszExitMessage;

public:
	virtual void CacheThreadData(void* cachedData) // Added in 3407
	{
	}

	virtual ~scrThreadVersion() = default;
	virtual eThreadState Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount) = 0;
	virtual eThreadState Run(uint32_t opsToExecute) = 0;
	virtual eThreadState Tick(uint32_t opsToExecute) = 0;
	virtual void Kill() = 0;
};

template<int Version>
class scrThreadVersion<Version, VersionAfter<Version, xbr::Build::Summer_2025>>
{
protected:
	scrThreadContext m_Context;
	void* m_pStack; // should be +176 including vtable
	void* pad;
	void* pad2;

	// should be +200
	const char* m_pszExitMessage;

public:
	virtual ~scrThreadVersion() = default;
	virtual eThreadState Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount) = 0;
	virtual eThreadState Run(uint32_t opsToExecute) = 0;
	virtual eThreadState Tick(uint32_t opsToExecute) = 0;
	virtual void Kill() = 0;

	virtual void CacheThreadData(void* cachedData) // Added in 3407, Moved to end in 3570
	{
	}
};

template<int Version>
class scrThreadMulti : public scrThreadVersion<Version>
{
public:
	scrThreadContext* GetContext()
	{
		return &m_Context;
	}
};

template<>
struct CrossBuildStruct<scrThread>
	: CrossBuildStructInfo<scrThread, scrThreadMulti, 3407, xbr::Build::Summer_2025>
{
};

rage::scrThreadContext* scrThread::GetContext()
{
	return XBV(GetContext());
}
}

template<int Version, typename = void>
class GtaThreadVersion : public rage::scrThreadMulti<Version>
{
public:
	char scriptName[64];
	rage::scriptHandler* m_pScriptHandler;
	void* m_pNetcomponent;
	char gta_pad2[24];
	uint32_t m_networkId;
	uint32_t gta_padInt;
	char flag1;
	char m_networkFlag;
	uint16_t gta_pad3;
	char gta_pad4[12];
	uint8_t m_canRemoveBlipsFromOtherScripts;
	char gta_pad5[7];

	inline void SetScriptName(const char* name)
	{
		strcpy_s(scriptName, name);
	}
};

template<int Version>
class GtaThreadVersion<Version, VersionAfter<Version, 2699>> : public rage::scrThreadMulti<Version>
{
public:
	uint32_t scriptHash; // Added in 2699
	char scriptName[64];
	rage::scriptHandler* m_pScriptHandler;
	void* m_pNetcomponent;
	char gta_pad2[24];
	uint32_t m_networkId;
	uint32_t gta_padInt;
	char flag1;
	char m_networkFlag;
	uint16_t gta_pad3;
	char gta_pad4[12];
	uint8_t m_canRemoveBlipsFromOtherScripts;
	char gta_pad5[7];

	inline void SetScriptName(const char* name)
	{
		strcpy_s(scriptName, name);
		scriptHash = HashString(name);
	}
};

template<int Version>
struct GtaThreadMulti : GtaThreadVersion<Version>
{
};

static_assert(sizeof(GtaThreadMulti<323>) == 344, "GtaThread has wrong size!");
static_assert(sizeof(GtaThreadMulti<2699>) == 352, "GtaThread has wrong size!");

template<>
struct CrossBuildStruct<GtaThread>
	: CrossBuildChildStructInfo<GtaThread, rage::scrThread, GtaThreadMulti, 2699>
{
};

extern rage::scriptHandlerMgr* g_scriptHandlerMgr;
extern uint32_t* scrThreadId;

void GtaThread::SetScriptName(const char* name)
{
	return XBV(SetScriptName(name));
}

rage::scriptHandler* GtaThread::GetScriptHandler()
{
	return XBV(m_pScriptHandler);
}

rage::eThreadState GtaThread::Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount)
{
	return XBV(Reset(scriptHash, pArgs, argCount));
}

rage::eThreadState GtaThread::Run(uint32_t opsToExecute)
{
	return XBV(Run(opsToExecute));
}

rage::eThreadState GtaThread::Tick(uint32_t opsToExecute)
{
	return XBV(Tick(opsToExecute));
}

void GtaThread::Kill()
{
	return XBV(Kill());
}

template<int Version>
struct CfxThreadMulti : GtaThreadMulti<Version>
{
	rage::eThreadState Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount) override;
	rage::eThreadState Run(uint32_t opsToExecute) override;
	rage::eThreadState Tick(uint32_t opsToExecute) override;
	void Kill() override;

	CfxThreadMulti(CfxThread& script)
		: GtaThreadMulti{}, Script(script)
	{
	}

	CfxThread& Script;
};

static hook::thiscall_stub<rage::eThreadState(void*, uint32_t)> gtaThreadTick([]()
{
	return hook::pattern("80 B9 ? 01 00 00 00 8B FA 48 8B D9 74 05").count(1).get(0).get<void>(-0xF);
});

template<int Version>
rage::eThreadState CfxThreadMulti<Version>::Tick(uint32_t opsToExecute)
{
	return gtaThreadTick(this, opsToExecute);
}

static hook::thiscall_stub<void(void*)> gtaThreadKill([]()
{
	return hook::pattern("48 83 EC 20 48 83 B9 ? 01 00 00 00 48 8B D9 74 14").count(1).get(0).get<void>(-6);
});

template<int Version>
void CfxThreadMulti<Version>::Kill()
{
	return gtaThreadKill(this);
}

template<int Version>
rage::eThreadState CfxThreadMulti<Version>::Run(uint32_t opsToExecute)
{
	// set the current thread
	rage::scrThread* activeThread = rage::scrEngine::GetActiveThread();
	rage::scrEngine::SetActiveThread((rage::scrThread*)this);

	// invoke the running thing if we're not dead
	if (GetContext()->State != rage::ThreadStateKilled)
	{
		Script.DoRun();
	}

	rage::scrEngine::SetActiveThread(activeThread);

	return GetContext()->State;
}

static hook::cdecl_stub<void(void*)> gtaThreadInit([]()
{
	return hook::pattern("83 89 ? 01 00 00 FF 83 A1 ? 01 00 00 F0").count(1).get(0).get<void>();
});

template<int Version>
rage::eThreadState CfxThreadMulti<Version>::Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount)
{
	Script.Reset();

	memset(&m_Context, 0, sizeof(m_Context));

	m_Context.State = rage::ThreadStateIdle;
	m_Context.ScriptHash = scriptHash;
	m_Context._mUnk1 = -1;
	m_Context._mUnk2 = -1;

	m_Context._set1 = 1;

	// zero out gtathread bits
	gtaThreadInit(this);

	m_networkFlag = true;
	m_canRemoveBlipsFromOtherScripts = false;

	m_pszExitMessage = "Normal exit";

	if (GetContext()->ThreadId == 0)
	{
		GetContext()->ThreadId = *scrThreadId;
		(*scrThreadId)++;
	}

	// attach script to the GTA script handler manager
	g_scriptHandlerMgr->AttachScript((rage::scrThread*)this);

	return m_Context.State;
}

struct CfxGtaThread : GtaThread
{
};

template<>
struct CrossBuildStruct<CfxGtaThread>
	: CrossBuildChildStructInfo<CfxGtaThread, GtaThread, CfxThreadMulti>
{
};

CfxThread::CfxThread()
	: Thread(xbr_new<CfxGtaThread>(*this))
{
}

CfxThread::~CfxThread()
{
	xbr_delete(Thread);
}

void CfxThread::AttachScriptHandler()
{
	auto thread = GetThread();

	CGameScriptHandlerMgr::GetInstance()->AttachScript(thread);
}

void CfxThread::DetachScriptHandler()
{
	auto thread = GetThread();

	if (auto handler = thread->GetScriptHandler())
	{
		handler->CleanupObjectList();

		CGameScriptHandlerMgr::GetInstance()->DetachScript(thread);
	}
}
