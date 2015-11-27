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

#ifdef COMPILING_RAGE_SCRIPTING_FIVE
#define RAGE_SCRIPTING_EXPORT DLL_EXPORT
#else
#define RAGE_SCRIPTING_EXPORT DLL_IMPORT
#endif

namespace rage
{
class scriptHandler;

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

	// scratch space for vector things
	alignas(uintptr_t) uint8_t m_vectorSpace[192];

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

	// copy vector3 pointer results to the initial argument
	void RAGE_SCRIPTING_EXPORT SetVectorResults();
};

// size should be 168
struct scrThreadContext
{
	uint32_t ThreadId;
	uint32_t ScriptHash; // + 4 (program id)
	eThreadState State; // + 8
	uint32_t IP; // + 12
	uint32_t FrameSP; // 
	uint32_t SP; // + 20, aka + 28
	uint32_t TimerA; // + 24
	uint32_t TimerB; // + 28
	uint32_t TimerC; // + 32, aka + 40
	uint32_t _mUnk1;
	uint32_t _mUnk2;
	uint32_t _f2C;
	uint32_t _f30;
	uint32_t _f34;
	uint32_t _f38;
	uint32_t _f3C;
	uint32_t _f40;
	uint32_t _f44;
	uint32_t _f48;
	uint32_t _f4C;
	uint32_t _f50; // should be +88 aka +80; stack size?

	uint32_t pad1;
	uint32_t pad2;
	uint32_t pad3;

	uint32_t _set1;

	uint32_t pad[68 / 4];
};

static_assert(sizeof(scrThreadContext) == 168, "scrThreadContext has wrong size!");

class 
#ifdef COMPILING_RAGE_SCRIPTING_FIVE
	__declspec(dllexport)
#endif
	scrThread
{
protected:
	scrThreadContext		m_Context;
	void*					m_pStack; // should be +176 including vtable
	void*					pad;
	void*					pad2;

	// should be +200
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
#ifdef COMPILING_RAGE_SCRIPTING_FIVE
	__declspec(dllexport)
#endif
	GtaThread :
		public rage::scrThread
{
protected:
	char scriptName[16];
	char gta_pad[48];
	rage::scriptHandler* m_pScriptHandler;
	char gta_pad2[40];
	char flag1;
	char m_networkFlag;
	uint16_t gta_pad3;
	char gta_pad4[12];
	uint8_t m_canRemoveBlipsFromOtherScripts;
	char gta_pad5[7];

public:
	virtual void					DoRun() = 0;

	virtual rage::eThreadState		Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount);
	virtual rage::eThreadState		Run(uint32_t opsToExecute);
	virtual rage::eThreadState		Tick(uint32_t opsToExecute);
	virtual void					Kill();

	inline void SetScriptName(const char* name)
	{
		strcpy(scriptName, va("%08x", HashString(name)));
	}

	inline rage::scriptHandler* GetScriptHandler() { return m_pScriptHandler; }

	inline void SetScriptHandler(void* scriptHandler) { m_pScriptHandler = reinterpret_cast<rage::scriptHandler*>(scriptHandler); }

	inline void SetScriptHandler(rage::scriptHandler* scriptHandler) { m_pScriptHandler = scriptHandler; }

	inline void RemoveCleanupFlag() {  }
};

static_assert(sizeof(GtaThread) == 344, "GtaThread has wrong size!");