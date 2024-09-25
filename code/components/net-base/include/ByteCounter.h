#pragma once

#include <memory>
#include <cstdint>

#include "DataStream.h"
#include "Span.h"

namespace net
{
	class ByteCounter : public DataStream
	{		
		uint64_t m_counter = 0;

	public:
		static constexpr Type kType = Type::Counter;

		ByteCounter()
		{
		}

		// GetOffset and GetCapacity to match the ByteReader and ByteWriter
		uint64_t GetOffset() const
		{
			return m_counter;
		}

		uint64_t GetCapacity() const
		{
			return m_counter;
		}

		/// <summary>
		/// Counts the size of T
		/// </summary>
		/// <param name="value">The value to count the size from.</param>
		/// <returns>Returns a bool to match the DataStream, but its always true for the ByteCounter</returns>
		template <typename T>
		bool Field(T& value)
		{
			const uint64_t size = sizeof(value);
			if (m_counter > m_counter + size)
			{
				m_counter = std::numeric_limits<uint64_t>::max();
				return true;
			}

			m_counter += size;
			return true;
		}

		/// <summary>
		/// Counts the size of T
		/// </summary>
		/// <param name="value">The value to count the size from.</param>
		/// <param name="size">The size specifies the amount of bytes to count.</param>
		/// <returns>Returns a bool to match the DataStream, but its always true for the ByteCounter</returns>
		template <typename T>
		bool Field(T& value, const size_t size)
		{
			if (m_counter > m_counter + size)
			{
				m_counter = std::numeric_limits<uint64_t>::max();
				return true;
			}

			m_counter += size;
			return true;
		}

		/// <summary>
		/// Counts the size of Span
		/// </summary>
		/// <param name="value">The Span to count the size from</param>
		/// <param name="size">Amount of elements to count</param>
		/// <returns>Returns a bool to match the DataStream, but its always true for the ByteCounter</returns>
		template <typename T>
		bool Field(Span<T>& value, const size_t size)
		{
			const uint64_t valueSize = size * sizeof(T);
			if (m_counter > m_counter + valueSize)
			{
				m_counter = std::numeric_limits<uint64_t>::max();
				return true;
			}
	
			m_counter += valueSize;
			return true;
		}
	};

	template <>
	inline bool ByteCounter::Field<bool>(bool& value)
	{
		const uint64_t valueSize = 1;
		if (m_counter > m_counter + valueSize)
		{
			m_counter = std::numeric_limits<uint64_t>::max();
			return true;
		}

		m_counter += valueSize;
		return true;
	}
	
	template <>
	inline bool ByteCounter::Field<std::string_view>(std::string_view& value, const size_t size)
	{
		if (m_counter > m_counter + size)
		{
			m_counter = std::numeric_limits<uint64_t>::max();
			return true;
		}

		m_counter += size;
		return true;
	}
	
	template <>
	inline bool ByteCounter::Field<std::string>(std::string& value, const size_t size)
	{
		if (m_counter > m_counter + size)
		{
			m_counter = std::numeric_limits<uint64_t>::max();
			return true;
		}

		m_counter += size;
		return true;
	}
}
