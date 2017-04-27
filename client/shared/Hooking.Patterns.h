/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#pragma once

#include "Hooking.h"

#include <cassert>
#include <vector>

#pragma warning(push)
#pragma warning(disable:4201)

#define PATTERNS_USE_HINTS 1

namespace hook
{
	class pattern_match
	{
	private:
		void* m_pointer;

	public:
		inline pattern_match(void* pointer)
			: m_pointer(pointer)
		{
		}

		template<typename T>
		T* get(ptrdiff_t offset = 0) const
		{
			char* ptr = reinterpret_cast<char*>(m_pointer);
			return reinterpret_cast<T*>(ptr + offset);
		}
	};

	class pattern
	{
	private:
		std::string m_bytes;
		std::string m_mask;

#if PATTERNS_USE_HINTS
		uint64_t m_hash;
#endif

		std::vector<pattern_match> m_matches;

		bool m_matched;

		union
		{
			void* m_module;
			struct
			{
				uintptr_t m_rangeStart;
				uintptr_t m_rangeEnd;
			};
		};

	protected:
		inline pattern(void* module)
			: m_module(module), m_rangeEnd(0), m_matched(false)
		{
		}

		inline pattern(uintptr_t begin, uintptr_t end)
			: m_rangeStart(begin), m_rangeEnd(end), m_matched(false)
		{
		}

		void Initialize(const char* pattern, size_t length);

	private:
		bool ConsiderMatch(uintptr_t offset);

		void EnsureMatches(uint32_t maxCount);

		inline pattern_match _get_internal(size_t index) const
		{
			return m_matches[index];
		}

	public:
		template<size_t Len>
		pattern(const char(&pattern)[Len])
			: pattern(getRVA<void>(0))
		{
			Initialize(pattern, Len);
		}

		inline pattern& count(uint32_t expected) &
		{
			EnsureMatches(expected);
			assert(m_matches.size() == expected);
			return *this;
		}

		inline pattern& count_hint(uint32_t expected) &
		{
			EnsureMatches(expected);
			return *this;
		}

		inline pattern& clear() &
		{
			m_matches.clear();
			m_matched = false;
			return *this;
		}

		inline pattern&& count(uint32_t expected) &&
		{
			EnsureMatches(expected);
			assert(m_matches.size() == expected);
			return std::move(*this);
		}

		inline pattern&& count_hint(uint32_t expected) &&
		{
			EnsureMatches(expected);
			return std::move(*this);
		}

		inline pattern&& clear() &&
		{
			m_matches.clear();
			m_matched = false;
			return std::move(*this);
		}

		inline size_t size()
		{
			EnsureMatches(UINT32_MAX);
			return m_matches.size();
		}

		inline bool empty()
		{
			return size() == 0;
		}

		inline pattern_match get(size_t index)
		{
			EnsureMatches(UINT32_MAX);
			return _get_internal(index);
		}

		inline pattern_match get_one()
		{
			return std::forward<pattern>(*this).count(1)._get_internal(0);
		}

		template<typename T = void>
		inline auto get_first(ptrdiff_t offset = 0)
		{
			return get_one().get<T>(offset);
		}

	public:
#if PATTERNS_USE_HINTS
		// define a hint
		static void hint(uint64_t hash, uintptr_t address);
#endif
	};

	class module_pattern
		: public pattern
	{
	public:
		template<size_t Len>
		module_pattern(void* module, const char(&pattern)[Len])
			: pattern(module)
		{
			Initialize(pattern, Len);
		}
	};

	class range_pattern
		: public pattern
	{
	public:
		template<size_t Len>
		range_pattern(uintptr_t begin, uintptr_t end, const char(&pattern)[Len])
			: pattern(begin, end)
		{
			Initialize(pattern, Len);
		}
	};


	template<typename T = void, size_t Len>
	auto get_pattern(const char(&pattern_string)[Len], ptrdiff_t offset = 0)
	{
		return pattern(pattern_string).get_first<T>(offset);
	}
}

#pragma warning(pop)