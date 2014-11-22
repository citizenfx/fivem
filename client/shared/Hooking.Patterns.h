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
			return get(0);
		}
	};

	class pattern
	{
	private:
		std::string m_bytes;
		std::string m_mask;

		size_t m_size;

		std::vector<pattern_match> m_matches;

	private:
		void Initialize(const char* pattern, size_t length);

		void ConsiderMatch(uintptr_t offset);

	public:
		template<size_t Len>
		pattern(const char (&pattern)[Len])
		{
			Initialize(pattern, Len);
		}

		inline pattern& count(int expected)
		{
			assert(m_matches.size() == expected);

			return *this;
		}

		inline pattern_match& get(int index)
		{
			return m_matches[index];
		}

	public:
		// define a hint
		static void hint(uint64_t hash, uintptr_t address);
	};
}