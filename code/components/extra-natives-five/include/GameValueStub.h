#pragma once

#include <Hooking.h>

/// Utility for replacing game constants
template<typename T>
struct GameValueStub
{
	T m_original;
	T* m_address;

	inline void Init(T original)
	{
		m_original = original;
		m_address = static_cast<T*>(hook::AllocateStubMemory(sizeof(T)));
		*m_address = m_original;
	}

	inline T GetDefault()
	{
		return m_original;
	}

	inline T Get()
	{
		return *m_address;
	}

	inline void Set(T value)
	{
		*m_address = value;
	}

	inline void Reset()
	{
		*m_address = m_original;
	}

	inline void SetLocation(void* location, ptrdiff_t offset = 4)
	{
		hook::put<int32_t>(location, (intptr_t)m_address - (intptr_t)location - offset);
	}

	inline void SetLocation(void* location, ptrdiff_t offset, ptrdiff_t instruction_size)
	{
		intptr_t rip = (intptr_t)location + instruction_size;

		int32_t relOffset = (int32_t)((intptr_t)m_address - rip);

		hook::put<int32_t>((char*)location + offset, relOffset);
	}
};
