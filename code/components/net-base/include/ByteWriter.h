#pragma once

#include <memory>
#include <cstdint>

#include "DataStream.h"
#include "Span.h"

namespace net
{
	class ByteWriter : public DataStream
	{
		uint8_t* m_data;
		uint64_t m_capacity;
		uint64_t m_offset = 0;

	public:
		static constexpr Type kType = Type::Writer;

		ByteWriter(uint8_t* data, const uint64_t capacity): m_data(data), m_capacity(capacity)
		{
		}

		ByteWriter(const ByteWriter&) = delete;

		uint64_t GetOffset() const
		{
			return m_offset;
		}

		uint64_t GetCapacity() const
		{
			return m_capacity;
		}

		bool CanWrite(size_t length) const
		{
			return m_offset + length <= m_capacity;
		}

		void Seek(size_t offset)
		{
			m_offset = offset;
		}

		/// <summary>
		/// Writes a T to a byte stream
		/// </summary>
		/// <param name="value">The value to write to the buffer.</param>
		/// <returns>Returns true if the value got written, otherwise false.</returns>
		template <typename T>
		bool Field(T& value)
		{
			if (!CanWrite(sizeof(value)))
			{
				return false;
			}

			memcpy(m_data + m_offset, static_cast<void*>(&value), sizeof(value));
			m_offset += sizeof(value);
			return true;
		}

		/// <summary>
		/// Writes a T to a byte stream
		/// </summary>
		/// <param name="value">The value to write to the buffer.</param>
		/// <param name="size">The size is specifying the size of the amount of memory to read from the given value</param>
		/// <returns>Returns true if the value got written, otherwise false.</returns>
		template <typename T>
		bool Field(T& value, const size_t size)
		{
			if (!CanWrite(size))
			{
				return false;
			}

			memcpy(m_data + m_offset, static_cast<void*>(&value), size);
			m_offset += size;
			return true;
		}

		/// <summary>
		/// Writes a net::Span to the buffer.
		/// </summary>
		/// <param name="value">The Span to read the content from</param>
		/// <param name="size">amount of entities to write</param>
		/// <returns>Returns true when the size can be written/returns>
		template <typename T>
		bool Field(net::Span<T>& value, const size_t size)
		{
			const size_t sizeBytes = size * sizeof(T);
			if (!CanWrite(sizeBytes))
			{
				return false;
			}

			memcpy(m_data + m_offset, value.data(), sizeBytes);
			m_offset += sizeBytes;

			return true;
		}
	};

	template <>
	inline bool ByteWriter::Field<bool>(bool& value)
	{
		return Field<uint8_t>(*reinterpret_cast<uint8_t*>(&value));
	}

	/// <summary>
	/// Writes a std::string_view to the buffer.
	/// </summary>
	/// <param name="value">The string_view to read the content from</param>
	/// <param name="size">length of the string to read in bytes</param>
	/// <returns>Returns true when the requested size can be read</returns>
	template <>
	inline bool ByteWriter::Field<std::string_view>(std::string_view& value, const size_t size)
	{
		if (!CanWrite(size))
		{
			return false;
		}

		memcpy(m_data + m_offset, value.data(), size);
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
	inline bool ByteWriter::Field<std::string>(std::string& value, const size_t size)
	{
		if (!CanWrite(size))
		{
			return false;
		}

		memcpy(m_data + m_offset, value.data(), size);
		m_offset += size;

		return true;
	}
}
