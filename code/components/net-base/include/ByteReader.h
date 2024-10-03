#pragma once

#include <memory>
#include <cstdint>

#include "DataStream.h"
#include "Span.h"

namespace net
{
	class ByteReader : public DataStream
	{
		const uint8_t* m_data;
		uint64_t m_capacity;
		uint64_t m_offset = 0;

	public:
		static constexpr Type kType = Type::Reader;

		ByteReader(const uint8_t* data, const uint64_t capacity): m_data(data), m_capacity(capacity)
		{
		}

		ByteReader(const ByteReader&) = delete;

		uint64_t GetOffset() const
		{
			return m_offset;
		}

		uint64_t GetCapacity() const
		{
			return m_capacity;
		}

		const uint8_t* GetData() const
		{
			return m_data;
		}

		bool CanRead(const size_t length) const
		{
			return m_offset + length <= m_capacity;
		}

		void Seek(uint64_t offset)
		{
			m_offset = offset;
		}

		/// <summary>
		/// Reads a T from a byte stream
		/// </summary>
		/// <param name="value">The value to write the read content into.</param>
		/// <returns>Returns true if the value is filled, otherwise false.</returns>
		template <typename T>
		bool Field(T& value)
		{
			if (!CanRead(sizeof(value)))
			{
				return false;
			}

			memcpy(static_cast<void*>(&value), m_data + m_offset, sizeof(value));
			m_offset += sizeof(value);
			return true;
		}

		/// <summary>
		/// Reads a T from a byte stream
		/// </summary>
		/// <param name="value">The value to write the read content into.</param>
		/// <param name="size">The size is specifying the size of the amount of memory to write to the given value</param>
		/// <returns>Returns true if the value is filled, otherwise false.</returns>
		template <typename T>
		bool Field(T& value, const size_t size)
		{
			if (!CanRead(size))
			{
				return false;
			}

			memcpy(static_cast<void*>(&value), m_data + m_offset, size);
			m_offset += size;
			return true;
		}

		/// <summary>
		/// Reads a net::Span from the buffer. std::Span is read allocation free
		/// </summary>
		/// <param name="value">The Span to read the content into</param>
		/// <param name="size">Amount of elements to read</param>
		/// <returns>Returns true when the requested size can be read</returns>
		template <typename T>
		bool Field(Span<T>& value, const size_t size)
		{
			const size_t sizeBytes = size * sizeof(T);
			if (!CanRead(sizeBytes))
			{
				return false;
			}

			value = net::Span(const_cast<T*>(reinterpret_cast<const T*>(m_data + m_offset)), size);
			m_offset += sizeBytes;

			return true;
		}

		size_t GetRemaining() const
		{
			return m_capacity - m_offset;
		}
	};

	template <>
	inline bool ByteReader::Field<bool>(bool& value)
	{
		return Field<uint8_t>(*reinterpret_cast<uint8_t*>(&value));
	}

	/// <summary>
	/// Reads a std::string_view from the buffer. std::string_view is read allocation free
	/// </summary>
	/// <param name="value">The string_view to read the content into</param>
	/// <param name="size">length of the string to read in bytes</param>
	/// <returns>Returns true when the requested size can be read</returns>
	template <>
	inline bool ByteReader::Field<std::string_view>(std::string_view& value, const size_t size)
	{
		if (!CanRead(size))
		{
			return false;
		}

		value = std::string_view(reinterpret_cast<const char*>(m_data + m_offset), size);
		m_offset += size;
		
		return true;
	}

	/// <summary>
	/// Reads a std::string from the buffer.
	/// </summary>
	/// <param name="value">The string to read the content into</param>
	/// <param name="size">length of the string to read in bytes</param>
	/// <returns>Returns true when the requested size can be read</returns>
	template <>
	inline bool ByteReader::Field<std::string>(std::string& value, const size_t size)
	{
		if (!CanRead(size))
		{
			return false;
		}

		value = std::string(reinterpret_cast<const char*>(m_data + m_offset), size);
		m_offset += size;
		
		return true;
	}
}
