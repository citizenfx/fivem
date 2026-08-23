#pragma once

#include <StdInc.h>

#include <Hooking.h>

struct PatternPair
{
	std::string_view pattern;
	int offset;
	int operand_remaining = 4;
};

struct PatternClampPair
{
	std::string_view pattern;
	int offset;
	bool clamp;
};

struct PatternPatchPair
{
	std::string_view pattern;
	int offset;
	int intendedValue;
	int newValue;
};

struct PatternAbsolutePair
{
	std::string_view pattern;
	int offset;
	uintptr_t address;
	int instrLen = 7;
};

inline void RelocateRelative(void* base, std::initializer_list<PatternPair> list, int intendedEntries = -1)
{
	void* oldAddress = nullptr;

	if (intendedEntries >= 0 && static_cast<int>(list.size()) != intendedEntries)
	{
		__debugbreak();
		return;
	}

	for (auto& entry : list)
	{
		auto location = hook::get_pattern<int32_t>(entry.pattern, entry.offset);

		if (!oldAddress)
		{
			oldAddress = hook::get_address<void*>(location, 0, entry.operand_remaining);
		}

		auto curTarget = hook::get_address<void*>(location, 0, entry.operand_remaining);
		assert(curTarget == oldAddress);

		hook::put<int32_t>(location, (intptr_t)base - (intptr_t)location - entry.operand_remaining);
	}
}

inline void RelocateRelativeLocation(std::initializer_list<PatternAbsolutePair> list)
{
	for (auto& entry : list)
	{
		uint8_t* instructions = reinterpret_cast<uint8_t*>(hook::get_pattern(entry.pattern));
		uintptr_t instrNext = reinterpret_cast<uintptr_t>(instructions) + entry.instrLen;
		int32_t newDisp = (int32_t)((int64_t)entry.address - (int64_t)instrNext);
		hook::put<int32_t>(instructions + entry.offset, newDisp);
	}
}

template<int instrLen = 7, int instrOffset = 3>
inline void PatchRelativeLocation(uintptr_t address, uintptr_t newLocation)
{
	uint8_t* instructions = reinterpret_cast<uint8_t*>(address);
	uintptr_t instrNext = address + instrLen;
	int32_t newDisp = (int32_t)((int64_t)newLocation - (int64_t)instrNext);
	hook::put<int32_t>(instructions + instrOffset, newDisp);
}

template<class T>
inline void PatchValue(std::initializer_list<PatternPatchPair> list)
{
	for (auto& entry : list)
	{
		auto location = hook::pattern(entry.pattern).count(1).get(0).get<T>(entry.offset);
		auto origVal = *location;
		assert(origVal == entry.intendedValue);
		hook::put<T>(location, (T)entry.newValue);
	}
}

template<class T>
inline void PatchValue(uintptr_t address, size_t offset, uint64_t origValue, uint64_t newValue)
{
	T newData = (T)newValue;
	T* addr = (T*)(address + offset);

	assert(*addr == (T)origValue);
	hook::put<T>(addr, newData);
	assert(*addr == newData);
}
