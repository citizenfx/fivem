/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "Hooking.h"

namespace hook
{
	class pattern_match
	{
	private:
		void* m_pointer;

	public:
		inline pattern_match(void* pointer)
		{
			m_pointer = pointer;
		}

		template<typename T>
		T* get(int offset)
		{
			char* ptr = reinterpret_cast<char*>(m_pointer);
			return reinterpret_cast<T*>(ptr + offset);
		}

		template<typename T>
		T* get()
		{
			return get<T>(0);
		}
	};

	class pattern
	{
	private:
		std::string m_bytes;
		std::string m_mask;

		uint64_t m_hash;

		size_t m_size;

		std::vector<pattern_match> m_matches;

		bool m_matched;

	private:
		void Initialize(const char* pattern, size_t length);

		bool ConsiderMatch(uintptr_t offset);

		void EnsureMatches(int maxCount);

	public:
		template<size_t Len>
		pattern(const char (&pattern)[Len])
		{
			Initialize(pattern, Len);
		}

		inline pattern& count(int expected)
		{
			if (!m_matched)
			{
				EnsureMatches(expected);
			}

			assert(m_matches.size() == expected);

			return *this;
		}

		inline size_t size()
		{
			if (!m_matched)
			{
				EnsureMatches(INT_MAX);
			}

			return m_matches.size();
		}

		inline pattern_match& get(int index)
		{
			if (!m_matched)
			{
				EnsureMatches(INT_MAX);
			}

			return m_matches[index];
		}

	public:
		// define a hint
		static void hint(uint64_t hash, uintptr_t address);
	};
}

void Citizen_PatternSaveHint(uint64_t hash, uintptr_t hint);