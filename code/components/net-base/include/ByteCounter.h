#pragma once

#include <memory>
#include <cstdint>

#include "DataStream.h"

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
			m_counter += sizeof(value);
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
			m_counter += size;
			return true;
		}
	};

	template <>
	inline bool ByteCounter::Field<bool>(bool& value)
	{
		m_counter += 1;
		return true;
	}
	
	template <>
	inline bool ByteCounter::Field<std::string_view>(std::string_view& value, const size_t size)
	{
		m_counter += size;
		return true;
	}
	
	template <>
	inline bool ByteCounter::Field<std::string>(std::string& value, const size_t size)
	{
		m_counter += size;
		return true;
	}
}
