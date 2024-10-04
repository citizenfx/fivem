#pragma once

#include <memory>
#include <cstdint>

#include "DataStream.h"
#include "ForceConsteval.h"

namespace net
{
class BitWriter : public DataStream
{
	uint8_t* m_data;
	uint64_t m_capacity;
	uint64_t m_offset = 0;

	/// <summary>
	/// Bitmask generation function.
	/// </summary>
	template<typename T>
	static T GetBitMask(const uint8_t bitCount)
	{
		return (1ULL << bitCount) - 1;
	}

	/// <summary>
	/// Generic value adjustment.
	/// To prevent the value from writing more bits than intended.
	/// </summary>
	template<typename T>
	static T AdjustValueBitSize(T value, const uint8_t bitSize)
	{
		return value & GetBitMask<T>(bitSize);
	}

	/// <summary>
	/// Generic write implementation for all types.
	/// TWriteType should be in general always one type larger than T.
	/// </summary>
	template<typename T, typename TWriteType, bool UseMultiElement = sizeof(T) == sizeof(TWriteType)>
	bool Write(T& value, const uint8_t bitSize)
	{
		static_assert(sizeof(T) < sizeof(TWriteType) || UseMultiElement, "To write the type without multi element it needs to be larger than the write type.");

		if (bitSize > force_consteval<uint8_t, sizeof(T) * 8>)
		{
			return false;
		}

		const uint64_t newPosition = m_offset + bitSize;

		if (newPosition > m_capacity)
		{
			return false;
		}

		// the amount of bits that are currently used inside the current TReadType
		const uint8_t usedBits = m_offset % 8;
		const uint64_t byteIndex = m_offset >> 3;
		// TWriteType pointer of the current byte position to start writing to
		TWriteType* start = reinterpret_cast<TWriteType*>(m_data + byteIndex);
		const uint8_t maxWrittenBits = usedBits + static_cast<uint8_t>(bitSize);
		// multi element means the new value does not fit into the current TWriteType
		const bool multiElement = maxWrittenBits > force_consteval<uint8_t, sizeof(TWriteType) * 8>;

		// value doesn't fit into one element
		if (net::force_consteval<bool, UseMultiElement> && multiElement)
		{
			// usedBits is always > 0 otherwise maxWrittenBits > sizeof(TWriteType) * 8 is not possible
			// the amount of bits to write to *start to fill the current uint8_t
			const uint8_t bitsToFill = 8 - usedBits;
			// writes the first bytes
			*reinterpret_cast<uint8_t*>(start) = (*reinterpret_cast<uint8_t*>(start) & GetBitMask<uint64_t>(usedBits)) | static_cast<uint8_t>((value & GetBitMask<uint64_t>(bitsToFill)) << usedBits);
			// now write the remaining bits to the buffer with sizeof(TWriteType) * 8 size
			// for uint64_t the value (value >> bitsToFill) is never smaller than 57bit, it is always 64bit and always needs to be written as uint64_t
			*reinterpret_cast<uint64_t*>(reinterpret_cast<uint8_t*>(start) + 1) = (value >> bitsToFill);
			m_offset = newPosition;
			return true;
		}

		// when less than sizeof(TWriteType) * 8 free bits are available, the value needs to be carefully read to not over read the buffer area
		const uint64_t freeBits = m_capacity - (byteIndex << 3);
		if (freeBits < force_consteval<uint8_t, sizeof(TWriteType) * 8>)
		{
			// the current bits are holding the bits that are present in the current byte at the offset before the write operation
			const TWriteType currentBits = *reinterpret_cast<const uint8_t*>(start) & GetBitMask<TWriteType>(usedBits);
			// the new bits are holding the bits that get added to the current byte with the write operation
			const TWriteType newBits = static_cast<TWriteType>(AdjustValueBitSize(value, bitSize));
			// the temp value is holding the current bits and the new bits
			// the new bits are shifted to make room for the current bits
			const TWriteType temp = currentBits | static_cast<TWriteType>(newBits << static_cast<TWriteType>(usedBits));
			// converts the writable bits to bytes by adding 7 and dividing by 8
			const uint8_t bytesToWrite = (maxWrittenBits + 7) >> 3;
			// copy the temp value to the buffer with the actual used bytes
			memcpy(start, &temp, bytesToWrite);
			m_offset = newPosition;
			return true;
		}

		*start = (*start & GetBitMask<TWriteType>(usedBits)) | (static_cast<TWriteType>(AdjustValueBitSize(value, bitSize)) << usedBits);
		m_offset = newPosition;
		return true;
	}

public:
	static constexpr Type Type = Type::Writer;

	BitWriter(uint8_t* data, const uint64_t capacity)
		: m_data(data), m_capacity(capacity)
	{
	}

	BitWriter(const BitWriter&) = delete;

	uint64_t GetOffset() const
	{
		return m_offset;
	}

	uint64_t GetCapacity() const
	{
		return m_capacity;
	}

	bool CanRead(const size_t length) const
	{
		return m_offset + length <= m_capacity;
	}

	/// <summary>
	/// Writes a T to a bit stream
	/// </summary>
	/// <param name="value">The value to write to the buffer.</param>
	/// <param name="bitSize">The amount of bits to write.</param>
	/// <returns>Returns true if the value got written, otherwise false.</returns>
	template<typename T>
	bool Field(T& value, const size_t bitSize)
	{
		return Write<T, uint64_t>(value, static_cast<uint8_t>(bitSize));
	}

	template<typename T>
	bool Field(T& value)
	{
		static_assert("BitReader needs explicit size for generic Field.");
		return false;
	}

	size_t GetRemaining() const
	{
		return m_capacity - m_offset;
	}

	bool IsAtEnd() const
	{
		return m_offset >= m_capacity;
	}

	void Reset()
	{
		m_offset = 0;
	}
};

/// <summary>
/// Writes a uint64_t to a bit stream
/// </summary>
/// <param name="value">The value to write to the buffer.</param>
/// <param name="bitSize">The amount of bits to write.</param>
/// <returns>Returns true if the value got written, otherwise false.</returns>
template<>
inline bool BitWriter::Field<uint64_t>(uint64_t& value, const size_t bitSize)
{
	return Write<uint64_t, uint64_t, true>(value, static_cast<uint8_t>(bitSize));
}

/// <summary>
/// Writes a uint16_t to a bit stream. This function is a reduced version of the Field function that accepts any.
/// To do optimizations for smaller values.
/// </summary>
/// <param name="value">The value to write to the buffer.</param>
/// <param name="bitSize">The amount of bits to write.</param>
/// <returns>Returns true if the value got written, otherwise false.</returns>
template<>
inline bool BitWriter::Field<uint16_t>(uint16_t& value, const size_t bitSize)
{
	return Write<uint16_t, uint32_t>(value, static_cast<uint8_t>(bitSize));
}

/// <summary>
/// Writes a uint8_t to a bit stream. This function is a reduced version of the Field function that accepts any.
/// To do optimizations for smaller values.
/// </summary>
/// <param name="value">The value to write to the buffer.</param>
/// <param name="bitSize">The amount of bits to write.</param>
/// <returns>Returns true if the value got written, otherwise false.</returns>
template<>
inline bool BitWriter::Field<uint8_t>(uint8_t& value, const size_t bitSize)
{
	return Write<uint8_t, uint16_t>(value, static_cast<uint8_t>(bitSize));
}

/// <summary>
/// Reads a bool from a bit stream. This is always read as a single bit, that is why it does not need a bit size.
/// </summary>
/// <param name="val">The value to write the read content into.</param>
/// <returns>Returns true if the value is filled, otherwise false.</returns>
template<>
inline bool BitWriter::Field<bool>(bool& val)
{
	const uint64_t newPosition = m_offset + 1;

	if (newPosition > m_capacity)
	{
		return false;
	}

	const uint64_t startByteIndex = m_offset >> 3;

	if (startByteIndex >= m_capacity)
	{
		return false;
	}

	const uint8_t shift = (7 - (m_offset % 8));
	m_data[startByteIndex] = static_cast<uint8_t>(m_data[startByteIndex] & ~(1 << shift)) | static_cast<uint8_t>(val << shift);
	m_offset = newPosition;
	return true;
}
}
