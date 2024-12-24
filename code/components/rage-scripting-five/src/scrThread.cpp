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

static hook::thiscall_stub<rage::eThreadState(GtaThread*, uint32_t)> gtaThreadTick([] ()
{
	return hook::pattern("80 B9 ? 01 00 00 00 8B FA 48 8B D9 74 05").count(1).get(0).get<void>(-0xF);
});

rage::eThreadState WRAPPER GtaThread::Tick(uint32_t opsToExecute)
{
	return gtaThreadTick(this, opsToExecute);
}

static hook::thiscall_stub<void(GtaThread*)> gtaThreadKill([] ()
{
	return hook::pattern("48 83 EC 20 48 83 B9 ? 01 00 00 00 48 8B D9 74 14").count(1).get(0).get<void>(-6);
});

void GtaThread::Kill()
{
	return gtaThreadKill(this);
}

rage::eThreadState GtaThread::Run(uint32_t opsToExecute)
{
	// set the current thread
	rage::scrThread* activeThread = rage::scrEngine::GetActiveThread();
	rage::scrEngine::SetActiveThread(this);

	// invoke the running thing if we're not dead
	if (m_Context.State != rage::ThreadStateKilled)
	{
		DoRun();
	}

	rage::scrEngine::SetActiveThread(activeThread);

	return m_Context.State;
}

static hook::cdecl_stub<void(GtaThread*)> gtaThreadInit([] ()
{
	return hook::pattern("83 89 ? 01 00 00 FF 83 A1 ? 01 00 00 F0").count(1).get(0).get<void>();
});

extern rage::scriptHandlerMgr* g_scriptHandlerMgr;
extern uint32_t* scrThreadId;

rage::eThreadState GtaThread::Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount)
{
	memset(&m_Context, 0, sizeof(m_Context));

	m_Context.State = rage::ThreadStateIdle;
	m_Context.ScriptHash = scriptHash;
	m_Context._mUnk1 = -1;
	m_Context._mUnk2 = -1;

	m_Context._set1 = 1;

	// zero out gtathread bits
	gtaThreadInit(this);

	SetNetworkFlag(true);
	SetCanRemoveBlipsFromOtherScripts(false);

	m_pszExitMessage = "Normal exit";

	if (GetContext()->ThreadId == 0)
	{
		GetContext()->ThreadId = *scrThreadId;
		(*scrThreadId)++;
	}

	// attach script to the GTA script handler manager
	g_scriptHandlerMgr->AttachScript(this);

	return m_Context.State;
}

static hook::thiscall_stub<void(rage::scrNativeCallContext*)> setVectorResults([] ()
{
	return hook::pattern("83 79 18 00 48 8B D1 74 4A FF 4A 18").count(1).get(0).get<void>();
});

uint64_t rage::scrThread::proxyMethod1(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4)
{
	return 0;
}

uint64_t rage::scrThread::proxyMethod2(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4)
{
	if (xbr::IsGameBuildOrGreater<3407>())
	{
		return 0;
	}
	else
	{
		return Reset((uint32_t)a1, (void*)a2, (uint32_t)a3);
	}
} // Reset or dtor

uint64_t rage::scrThread::proxyMethod3(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4)
{
	if (xbr::IsGameBuildOrGreater<3407>())
	{
		return Reset((uint32_t)a1, (void*)a2, (uint32_t)a3);
	}
	else
	{
		return Run((uint32_t)a1);
	}
} // Run or Reset

uint64_t rage::scrThread::proxyMethod4(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4)
{
	if (xbr::IsGameBuildOrGreater<3407>())
	{
		return Run((uint32_t)a1);
	}
	else
	{
		return Tick((uint32_t)a1);
	}
} // Run or Reset

uint64_t rage::scrThread::proxyMethod5(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4)
{
	if (xbr::IsGameBuildOrGreater<3407>())
	{
		return Tick((uint32_t)a1);
	}
	else
	{
		Kill();
		return 0;
	}
} // Tick or Run

uint64_t rage::scrThread::proxyMethod6(uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4)
{
	if (xbr::IsGameBuildOrGreater<3407>())
	{
		Kill();
		return 0;
	}
	else
	{
		return 0;
	}
} // Kill or Tick
