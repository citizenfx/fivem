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
	return hook::pattern("80 B9 46 01 00 00 00 8B  FA 48 8B D9 74 05").count(1).get(0).get<void>(-0xF);
});

rage::eThreadState WRAPPER GtaThread::Tick(uint32_t opsToExecute)
{
	return gtaThreadTick(this, opsToExecute);
}

static hook::thiscall_stub<void(GtaThread*)> gtaThreadKill([] ()
{
	return hook::pattern("48 83 EC 20 48 83 B9 10 01 00 00 00 48 8B D9 74 14").count(1).get(0).get<void>(-6);
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
	return hook::pattern("83 89 38 01 00 00 FF 83 A1 50 01 00 00 F0").count(1).get(0).get<void>();
});

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

	m_networkFlag = true;
	m_canRemoveBlipsFromOtherScripts = false;

	m_pszExitMessage = "Normal exit";

	return m_Context.State;
}

static hook::thiscall_stub<void(rage::scrNativeCallContext*)> setVectorResults([] ()
{
	return hook::pattern("83 79 18 00 48 8B D1 74 4A FF 4A 18").count(1).get(0).get<void>();
});

void rage::scrNativeCallContext::SetVectorResults()
{
	return setVectorResults(this);
}