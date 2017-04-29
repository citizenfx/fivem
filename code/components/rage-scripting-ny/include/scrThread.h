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

#pragma once

namespace rage
{
enum eThreadState
{
	ThreadStateIdle,
	ThreadStateRunning,
	ThreadStateKilled,
	ThreadState3,
	ThreadState4,			// Sets opsToExecute to 1100000, and state to Idle in CallNative
};

class scrNativeCallContext
{
protected:
	void* m_pReturn;
	uint32_t m_nArgCount;
	void* m_pArgs;

	uint32_t m_nDataCount;

public:
	template<typename T>
	inline T GetArgument(int idx)
	{
		intptr_t* arguments = (intptr_t*)m_pArgs;

		return *(T*)&arguments[idx];
	}

	template<typename T>
	inline void SetResult(int idx, T value)
	{
		intptr_t* returnValues = (intptr_t*)m_pReturn;

		*(T*)&returnValues[idx] = value;
	}

	inline int GetArgumentCount()
	{
		return m_nArgCount;
	}

	template<typename T>
	inline T GetResult(int idx)
	{
		intptr_t* returnValues = (intptr_t*)m_pReturn;

		return *(T*)&returnValues[idx];
	}
};

struct scrThreadContext
{
	uint32_t ThreadId;
	uint32_t ScriptHash;
	eThreadState State;
	uint32_t IP;
	uint32_t FrameSP;
	uint32_t SP;
	uint32_t TimerA;
	uint32_t TimerB;
	uint32_t TimerC;
	float WaitTime;
	uint32_t _f28;
	uint32_t _f2C;
	uint32_t _f30;
	uint32_t _f34;
	uint32_t _f38;
	uint32_t _f3C;
	uint32_t _f40;
	uint32_t ExIP;
	uint32_t ExFrameSP;
	uint32_t ExSP;
	uint32_t _f50;
};

class 
#ifdef COMPILING_RAGE_SCRIPTING_NY
	__declspec(dllexport)
#endif
	scrThread
{
protected:
	scrThreadContext		m_Context;
	void*					m_pStack;
	uint32_t				_f5C;
	void*					m_pXLiveBuffer;
	uint32_t				_f64;
	uint32_t				_f68;
	const char*				m_pszExitMessage;

public:
	virtual ~scrThread()																	{}
	virtual eThreadState	Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount)		= 0;
	virtual eThreadState	Run(uint32_t opsToExecute)										= 0;
	virtual eThreadState	Tick(uint32_t opsToExecute)										= 0;
	virtual void			Kill()															= 0;

	scrThreadContext*		GetContext()													{ return &m_Context; }
};
}

class
#ifdef COMPILING_RAGE_SCRIPTING_NY
	__declspec(dllexport)
#endif
	GtaThread :
		public rage::scrThread
{
protected:
	char m_szProgramName[24];
	uint32_t _f88;
	uint32_t _f8C;
	uint32_t _f90;
	uint8_t _f94;
	uint8_t _f95;
	uint8_t m_bThisScriptShouldBeSaved;
	uint8_t m_bPlayerControlOnInMissionCleanup;
	uint8_t m_bClearHelpInMissionCleanup;
	uint8_t _f99;
	uint8_t m_bAllowNonMinigameTextMessages;
	uint8_t _f9B;
	uint8_t m_paused;
	uint8_t m_bCanBePaused;
	uint8_t _f9E;
	uint8_t _f9F;
	uint8_t _fA0;
	uint8_t m_bCanRemoveBlipsCreatedByAnyScript;
	uint8_t _fA5;
	uint8_t _fA6;
	uint8_t _fA7;
	uint32_t _fA8;
	uint32_t m_nFlags;

public:
	virtual void					DoRun() = 0;

	virtual rage::eThreadState		Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount);
	virtual rage::eThreadState		Run(uint32_t opsToExecute);
	virtual rage::eThreadState		Tick(uint32_t opsToExecute);
	virtual void					Kill();

	inline void RemoveCleanupFlag() { _f94 = 0; }
};