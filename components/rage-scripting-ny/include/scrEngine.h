#pragma once
#include "pgCollection.h"
#include "scrThread.h"

namespace rage
{
class 
#ifdef COMPILING_RAGE_SCRIPTING_NY
	__declspec(dllexport)
#else
	__declspec(dllimport)
#endif
	scrEngine
{
public:
	static pgPtrCollection<GtaThread>* GetThreadCollection();

	static uint32_t IncrementThreadId();

	// gets the active thread
	static scrThread* GetActiveThread();

	// sets the currently running thread
	static void SetActiveThread(scrThread* thread);

	// adds a precreated custom thread to the runtime and starts it
	static void CreateThread(GtaThread* thread);

	// native function handler type
	typedef void(__cdecl * NativeHandler)(scrNativeCallContext* context);

	// gets a native function handler
	static NativeHandler GetNativeHandler(uint32_t hash);

	// queues the registration of a custom native function handler
	static void RegisterNativeHandler(const char* nativeName, NativeHandler handler);
	
public:
	static fwEvent<> OnScriptInit;
};
}

class NativeContext :
	public rage::scrNativeCallContext
{
private:
	// Configuration
	enum
	{
		MaxNativeParams = 16,
		ArgSize = 4,
	};

	// Anything temporary that we need
	uint8_t m_TempStack[MaxNativeParams * ArgSize];

public:
	inline NativeContext()
	{
		m_pArgs = &m_TempStack;
		m_pReturn = &m_TempStack;		// It's okay to point both args and return at 
										// the same pointer. The game should handle this.
		m_nArgCount = 0;
		m_nDataCount = 0;
	}

	template <typename T>
	inline void Push(T value)
	{
		if (sizeof(T) > ArgSize)
		{
			// We only accept sized 4 or less arguments... that means no double/f64 or large structs are allowed.
			throw "Argument has an invalid size";
		}
		else if (sizeof(T) < ArgSize)
		{
			// Ensure we don't have any stray data
			*reinterpret_cast<uint32_t*>(m_TempStack + ArgSize * m_nArgCount) = 0;
		}

		*reinterpret_cast<T*>(m_TempStack + ArgSize * m_nArgCount) = value;
		m_nArgCount++;
	}

	template <typename T>
	inline T GetResult()
	{
		return *reinterpret_cast<T*>(m_TempStack);
	}
};

struct pass
{
	template<typename ...T> pass(T...) {}
};

class NativeInvoke
{
private:
	static inline void Invoke(NativeContext *cxt, uint32_t hash)
	{
		auto fn = rage::scrEngine::GetNativeHandler(hash);

		// Commented out to reduce debug spam.
		//LogDebug("Invoking native: %s", name);

		if (fn != 0)
		{
			fn(cxt);
		}
	}

public:

	template<uint32_t Hash, typename R, typename... Args>
	static inline R Invoke(Args... args)
	{
		NativeContext cxt;

		pass{([&] ()
		{
			cxt.Push(args);
		}(), 1)...};

		Invoke(&cxt, Hash);

		return cxt.GetResult<R>();
	}
};