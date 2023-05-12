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

#include <CrossBuildRuntime.h>

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

struct scrVector
{
	alignas(8) float x;
	alignas(8) float y;
	alignas(8) float z;
};

struct alignas(16) scrVec3N
{
	float x;
	float y;
	float z;
};

struct scrVectorSpace
{
	scrVector* outVectors[4];
	scrVec3N inVectors[4];
};

class scrNativeCallContext
{
protected:
	void* m_pReturn; // +0
	uint32_t m_nArgCount; // +8
	void* m_pArgs; // +16

	uint32_t m_nDataCount; // +24

	// scratch space for vector things
	scrVectorSpace m_vectorSpace;
	uint8_t pad[96];

public:
	template<typename T>
	inline T& GetArgument(int idx)
	{
		intptr_t* arguments = (intptr_t*)m_pArgs;

		return *(T*)&arguments[idx];
	}

	template<typename T>
	inline void SetResult(int idx, T value)
	{
		intptr_t* returnValues = (intptr_t*)m_pReturn;

		if (returnValues)
		{
			*(T*)&returnValues[idx] = value;
		}
	}

	inline int GetArgumentCount()
	{
		return m_nArgCount;
	}

	template<typename T>
	inline T GetResult(int idx)
	{
		intptr_t* returnValues = (intptr_t*)m_pReturn;

		if (returnValues)
		{
			return *(T*)&returnValues[idx];
		}

		return T{};
	}

	inline void* GetArgumentBuffer()
	{
		return m_pArgs;
	}

	// copy vector3 pointer results to the initial argument
	inline void SetVectorResults()
	{
		for (size_t i = 0; i < m_nDataCount; i++)
		{
			auto outVector = m_vectorSpace.outVectors[i];
			const auto& inVector = m_vectorSpace.inVectors[i];

			outVector->x = inVector.x;
			outVector->y = inVector.y;
			outVector->z = inVector.z;
		}
	}

	inline const scrVec3N* GetVector()
	{
		return &m_vectorSpace.inVectors[0];
	}
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
//[2699] - GtaThread layout was changed, but not rage::scrThread
	char scriptName[64];                        // 208 0xD0 
	rage::scriptHandler* m_pScriptHandler;      // 272 0x110
	void* m_pNetcomponent;                      // 280 0x118
	char gta_pad2[24];                          // 288 0x120
	uint32_t m_networkId;                       // 312 0x138
	uint32_t gta_padInt;                        // 316 0x13C
	char flag1;                                 // 320 0x140
	char m_networkFlag;                         // 321 0x141
	uint16_t gta_pad3;                          // 322 0x142
	char gta_pad4[12];                          // 324 0x144
	uint8_t m_canRemoveBlipsFromOtherScripts;   // 336 0x150
	char gta_pad5[7];                           // 337 0x151
	char pad_b2699[8];

public:
	virtual void					DoRun() = 0;

	virtual rage::eThreadState		Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount);
	virtual rage::eThreadState		Run(uint32_t opsToExecute);
	virtual rage::eThreadState		Tick(uint32_t opsToExecute);
	virtual void					Kill();

	inline void SetScriptName(const char* name)
	{
		if (xbr::IsGameBuildOrGreater<2699>())
		{
			auto scriptHashPtr = reinterpret_cast<uint32_t*>((uint64_t)this + 0xD0);
			auto scriptNamePtr = reinterpret_cast<char*>((uint64_t)this + 0xD4);

			strncpy(scriptNamePtr, name, sizeof(scriptName) - 1);
			scriptNamePtr[sizeof(scriptName) - 1] = '\0';

			*scriptHashPtr = HashString(name);
		}
		else
		{
			strncpy(scriptName, name, sizeof(scriptName) - 1);
			scriptName[sizeof(scriptName) - 1] = '\0';
		}
	}

	inline void SetNetworkFlag(char state)
	{
		if (xbr::IsGameBuildOrGreater<2699>())
		{
			bool* thisNetFlag = (bool*)((uintptr_t)this + 0x149); // See GtaThreadInit function in GtaThread::GtaThread() (and extrapolate)
			*thisNetFlag = state;
		}
		else
		{
			m_networkFlag = state;
		}
	}

	inline void SetCanRemoveBlipsFromOtherScripts(uint8_t state)
	{
		if (xbr::IsGameBuildOrGreater<2699>())
		{
			bool* thisCanRemoveBlips = (bool*)((uintptr_t)this + 0x150); // See GtaThreadInit function in GtaThread::GtaThread()
			*thisCanRemoveBlips = state;
		}
		else
		{
			m_canRemoveBlipsFromOtherScripts = state;
		}
	}

	inline rage::scriptHandler* GetScriptHandler() 
	{
		if (xbr::IsGameBuildOrGreater<2699>())
		{
			return *(rage::scriptHandler**)((uintptr_t)this + 0x118);
		}

		return m_pScriptHandler;
	}

	inline void SetScriptHandler(rage::scriptHandler* scriptHandler) 
	{ 
		if (xbr::IsGameBuildOrGreater<2699>())
		{
			rage::scriptHandler** thisHandler = *(rage::scriptHandler***)((uintptr_t)this + 0x118); // See GtaThread constructor
			*thisHandler = scriptHandler;
		}
		else
		{
			m_pScriptHandler = scriptHandler; 
		}
	}

	inline void SetScriptHandler(void* scriptHandler)
	{
		SetScriptHandler(reinterpret_cast<rage::scriptHandler*>(scriptHandler));
	}

	inline void RemoveCleanupFlag() {  }
};

//static_assert(sizeof(GtaThread) == 344, "GtaThread has wrong size!");
static_assert(sizeof(GtaThread) == 352, "GtaThread has wrong size!"); // [2699]
