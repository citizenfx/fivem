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
	return hook::get_pattern("8B FA 48 8B D9 74 05 8B 41 10", -0x11);
});

rage::eThreadState WRAPPER GtaThread::Tick(uint32_t opsToExecute)
{
	return gtaThreadTick(this, opsToExecute);
}

static hook::thiscall_stub<void(GtaThread*)> gtaThreadKill([] ()
{
	return hook::get_pattern("48 8B D7 FF 50 58 0F BE", -0x3A);
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
	return hook::get_pattern("84 C0 74 07 40 88 BB 86", -0x7C);
});

extern rage::scriptHandlerMgr* g_scriptHandlerMgr;
extern uint32_t* scrThreadId;

rage::eThreadState GtaThread::Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount)
{
	//nameHash = HashString("startup"); // cheat for some init-time checks

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
	g_scriptHandlerMgr->AttachScript(this);

	return m_Context.State;
}

static hook::thiscall_stub<void(rage::scrNativeCallContext*)> setVectorResults([] ()
{
	return hook::get_pattern("8B 41 18 4C 8B C1 85 C0");
});
