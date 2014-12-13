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

rage::eThreadState WRAPPER GtaThread::Tick(uint32_t opsToExecute) { EAXJMP(0xBBCDF0); }
void WRAPPER GtaThread::Kill() { EAXJMP(0xBBCE70); }

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

rage::eThreadState GtaThread::Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount)
{
	m_Context.IP = 0;
	m_Context.SP = 0;
	m_Context.FrameSP = m_Context.SP;
	m_Context.TimerA = 0;
	m_Context.TimerB = 0;
	m_Context.TimerC = 0;
	m_Context.ExIP = 0;
	m_Context.ExFrameSP = 0;
	m_Context.ExSP = 0;
	m_Context._f50 = 0;
	m_Context.State = rage::ThreadStateIdle;
	m_Context.ScriptHash = scriptHash;

	// zero out gtathread bits
	memset(&_f88, 0, ((uintptr_t)&m_nFlags - (uintptr_t)&_f88) + 4);

	m_pszExitMessage = "Normal exit";
	m_bCanBePaused = true;
	m_paused = false;

	return m_Context.State;
}