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
	ThreadState4, // Sets opsToExecute to 1100000, and state to Idle in CallNative
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

	inline void* GetResultBuffer()
	{
		return m_pReturn;
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

class RAGE_SCRIPTING_EXPORT scrThread
{
public:
	scrThreadContext* GetContext();
};
}

class RAGE_SCRIPTING_EXPORT GtaThread : public rage::scrThread
{
public:
	void SetScriptName(const char* name);
	rage::scriptHandler* GetScriptHandler();

	rage::eThreadState Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount);
	rage::eThreadState Run(uint32_t opsToExecute);
	rage::eThreadState Tick(uint32_t opsToExecute);
	void Kill();
};

class RAGE_SCRIPTING_EXPORT CfxThread
{
public:
	CfxThread();

	virtual ~CfxThread();

	virtual void Reset()
	{
	}

	virtual void DoRun() = 0;

	virtual void Kill()
	{
	}

	void AttachScriptHandler();
	void DetachScriptHandler();

	GtaThread* GetThread()
	{
		return Thread;
	}

private:
	GtaThread* Thread;
};
