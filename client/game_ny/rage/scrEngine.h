#pragma once
#include "pgCollection.h"
#include "scrThread.h"

namespace rage
{
class GAMESPEC_EXPORT scrEngine
{
public:
	static pgPtrCollection<GtaThread>* GetThreadCollection();

	static void SetInitHook(void(*hook)(void*));

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

	// Here be repetitive ugly code... x16 (thanks copy and paste)
	// Only because I didn't want to use variadic functions
	// CitizenMP TODO: replace with variadic templates

	template <uint32_t Hash, typename R>
	static inline R Invoke()
	{
		NativeContext cxt;

		Invoke(&cxt, Hash);

		return cxt.GetResult<R>();
	}

	template <uint32_t Hash, typename R, typename T1>
	static inline R Invoke(T1 p1)
	{
		NativeContext cxt;

		cxt.Push(p1);

		Invoke(&cxt, Hash);

		return cxt.GetResult<R>();
	}

	template <uint32_t Hash, typename R, typename T1, typename T2>
	static inline R Invoke(T1 p1, T2 p2)
	{
		NativeContext cxt;

		cxt.Push(p1);
		cxt.Push(p2);

		Invoke(&cxt, Hash);

		return cxt.GetResult<R>();
	}

	template <uint32_t Hash, typename R, typename T1, typename T2, typename T3>
	static inline R Invoke(T1 p1, T2 p2, T3 p3)
	{
		NativeContext cxt;

		cxt.Push(p1);
		cxt.Push(p2);
		cxt.Push(p3);

		Invoke(&cxt, Hash);

		return cxt.GetResult<R>();
	}

	template <uint32_t Hash, typename R, typename T1, typename T2, typename T3, typename T4>
	static inline R Invoke(T1 p1, T2 p2, T3 p3, T4 p4)
	{
		NativeContext cxt;

		cxt.Push(p1);
		cxt.Push(p2);
		cxt.Push(p3);
		cxt.Push(p4);

		Invoke(&cxt, Hash);

		return cxt.GetResult<R>();
	}

	template <uint32_t Hash, typename R, typename T1, typename T2, typename T3, typename T4,
		typename T5>
		static inline R Invoke(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5)
	{
		NativeContext cxt;

		cxt.Push(p1);
		cxt.Push(p2);
		cxt.Push(p3);
		cxt.Push(p4);
		cxt.Push(p5);

		Invoke(&cxt, Hash);

		return cxt.GetResult<R>();
	}

	template <uint32_t Hash, typename R, typename T1, typename T2, typename T3, typename T4,
		typename T5, typename T6>
		static inline R Invoke(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6)
	{
		NativeContext cxt;

		cxt.Push(p1);
		cxt.Push(p2);
		cxt.Push(p3);
		cxt.Push(p4);
		cxt.Push(p5);
		cxt.Push(p6);

		Invoke(&cxt, Hash);

		return cxt.GetResult<R>();
	}

	template <uint32_t Hash, typename R, typename T1, typename T2, typename T3, typename T4,
		typename T5, typename T6, typename T7>
		static inline R Invoke(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7)
	{
		NativeContext cxt;

		cxt.Push(p1);
		cxt.Push(p2);
		cxt.Push(p3);
		cxt.Push(p4);
		cxt.Push(p5);
		cxt.Push(p6);
		cxt.Push(p7);

		Invoke(&cxt, Hash);

		return cxt.GetResult<R>();
	}

	template <uint32_t Hash, typename R, typename T1, typename T2, typename T3, typename T4,
		typename T5, typename T6, typename T7, typename T8>
		static inline R Invoke(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8)
	{
		NativeContext cxt;

		cxt.Push(p1);
		cxt.Push(p2);
		cxt.Push(p3);
		cxt.Push(p4);
		cxt.Push(p5);
		cxt.Push(p6);
		cxt.Push(p7);
		cxt.Push(p8);

		Invoke(&cxt, Hash);

		return cxt.GetResult<R>();
	}

	template <uint32_t Hash, typename R, typename T1, typename T2, typename T3, typename T4,
		typename T5, typename T6, typename T7, typename T8, typename T9>
		static inline R Invoke(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8,
		T9 p9)
	{
		NativeContext cxt;

		cxt.Push(p1);
		cxt.Push(p2);
		cxt.Push(p3);
		cxt.Push(p4);
		cxt.Push(p5);
		cxt.Push(p6);
		cxt.Push(p7);
		cxt.Push(p8);
		cxt.Push(p9);

		Invoke(&cxt, Hash);

		return cxt.GetResult<R>();
	}

	template <uint32_t Hash, typename R, typename T1, typename T2, typename T3, typename T4,
		typename T5, typename T6, typename T7, typename T8, typename T9,
		typename T10>
		static inline R Invoke(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8,
		T9 p9, T10 p10)
	{
		NativeContext cxt;

		cxt.Push(p1);
		cxt.Push(p2);
		cxt.Push(p3);
		cxt.Push(p4);
		cxt.Push(p5);
		cxt.Push(p6);
		cxt.Push(p7);
		cxt.Push(p8);
		cxt.Push(p9);
		cxt.Push(p10);

		Invoke(&cxt, Hash);

		return cxt.GetResult<R>();
	}

	template <uint32_t Hash, typename R, typename T1, typename T2, typename T3, typename T4,
		typename T5, typename T6, typename T7, typename T8, typename T9,
		typename T10, typename T11>
		static inline R Invoke(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8,
		T9 p9, T10 p10, T11 p11)
	{
		NativeContext cxt;

		cxt.Push(p1);
		cxt.Push(p2);
		cxt.Push(p3);
		cxt.Push(p4);
		cxt.Push(p5);
		cxt.Push(p6);
		cxt.Push(p7);
		cxt.Push(p8);
		cxt.Push(p9);
		cxt.Push(p10);
		cxt.Push(p11);

		Invoke(&cxt, Hash);
		return cxt.GetResult<R>();
	}

	template <uint32_t Hash, typename R, typename T1, typename T2, typename T3, typename T4,
		typename T5, typename T6, typename T7, typename T8, typename T9,
		typename T10, typename T11, typename T12>
		static inline R Invoke(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8,
		T9 p9, T10 p10, T11 p11, T12 p12)
	{
		NativeContext cxt;

		cxt.Push(p1);
		cxt.Push(p2);
		cxt.Push(p3);
		cxt.Push(p4);
		cxt.Push(p5);
		cxt.Push(p6);
		cxt.Push(p7);
		cxt.Push(p8);
		cxt.Push(p9);
		cxt.Push(p10);
		cxt.Push(p11);
		cxt.Push(p12);

		Invoke(&cxt, Hash);
		return cxt.GetResult<R>();
	}

	template <uint32_t Hash, typename R, typename T1, typename T2, typename T3, typename T4,
		typename T5, typename T6, typename T7, typename T8, typename T9,
		typename T10, typename T11, typename T12, typename T13>
		static inline R Invoke(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8,
		T9 p9, T10 p10, T11 p11, T12 p12, T13 p13)
	{
		NativeContext cxt;

		cxt.Push(p1);
		cxt.Push(p2);
		cxt.Push(p3);
		cxt.Push(p4);
		cxt.Push(p5);
		cxt.Push(p6);
		cxt.Push(p7);
		cxt.Push(p8);
		cxt.Push(p9);
		cxt.Push(p10);
		cxt.Push(p11);
		cxt.Push(p12);
		cxt.Push(p13);

		Invoke(&cxt, Hash);
		return cxt.GetResult<R>();
	}

	template <uint32_t Hash, typename R, typename T1, typename T2, typename T3, typename T4,
		typename T5, typename T6, typename T7, typename T8, typename T9,
		typename T10, typename T11, typename T12, typename T13, typename T14>
		static inline R Invoke(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8,
		T9 p9, T10 p10, T11 p11, T12 p12, T13 p13, T14 p14)
	{
		NativeContext cxt;

		cxt.Push(p1);
		cxt.Push(p2);
		cxt.Push(p3);
		cxt.Push(p4);
		cxt.Push(p5);
		cxt.Push(p6);
		cxt.Push(p7);
		cxt.Push(p8);
		cxt.Push(p9);
		cxt.Push(p10);
		cxt.Push(p11);
		cxt.Push(p12);
		cxt.Push(p13);
		cxt.Push(p14);

		Invoke(&cxt, Hash);
		return cxt.GetResult<R>();
	}

	template <uint32_t Hash, typename R, typename T1, typename T2, typename T3, typename T4,
		typename T5, typename T6, typename T7, typename T8, typename T9,
		typename T10, typename T11, typename T12, typename T13, typename T14,
		typename T15>
		static inline R Invoke(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8,
		T9 p9, T10 p10, T11 p11, T12 p12, T13 p13, T14 p14, T15 p15)
	{
		NativeContext cxt;

		cxt.Push(p1);
		cxt.Push(p2);
		cxt.Push(p3);
		cxt.Push(p4);
		cxt.Push(p5);
		cxt.Push(p6);
		cxt.Push(p7);
		cxt.Push(p8);
		cxt.Push(p9);
		cxt.Push(p10);
		cxt.Push(p11);
		cxt.Push(p12);
		cxt.Push(p13);
		cxt.Push(p14);
		cxt.Push(p15);

		Invoke(&cxt, Hash);
		return cxt.GetResult<R>();
	}


	template <uint32_t Hash, typename R, typename T1, typename T2, typename T3, typename T4,
		typename T5, typename T6, typename T7, typename T8, typename T9,
		typename T10, typename T11, typename T12, typename T13, typename T14,
		typename T15, typename T16>
		static inline R Invoke(T1 p1, T2 p2, T3 p3, T4 p4, T5 p5, T6 p6, T7 p7, T8 p8,
		T9 p9, T10 p10, T11 p11, T12 p12, T13 p13, T14 p14, T15 p15, T16 p16)
	{
		NativeContext cxt;

		cxt.Push(p1);
		cxt.Push(p2);
		cxt.Push(p3);
		cxt.Push(p4);
		cxt.Push(p5);
		cxt.Push(p6);
		cxt.Push(p7);
		cxt.Push(p8);
		cxt.Push(p9);
		cxt.Push(p10);
		cxt.Push(p11);
		cxt.Push(p12);
		cxt.Push(p13);
		cxt.Push(p14);
		cxt.Push(p15);
		cxt.Push(p16);

		Invoke(&cxt, Hash);
		return cxt.GetResult<R>();
	}
};