#pragma once

#include <memory>
#include <cstdint>

#include "DataStream.h"
#include "ForceConsteval.h"

namespace net
{
class BitReader : public DataStream
{
	const uint8_t* m_data;
	uint64_t m_capacity;
	uint64_t m_offset = 0;

	template<typename T>
	static T GetBitMask(uint8_t bitCount)
	{
		return (1ULL << bitCount) - 1;
	}

	/// <summary>
	/// Read a value from a bit area.
	/// The area value is shifted to the start bit.
	/// The result value is adjusted to the maximum amount of bits to read.
	/// </summary>
	template<typename TResult, typename TReadValue>
	static TResult ReadBitArea(TReadValue value, const uint8_t startBit, const uint8_t bitSize)
	{
		return static_cast<TResult>(value >> startBit) & GetBitMask<TResult>(bitSize);
	}

	/// <summary>
	/// Generic read implementation for all types.
	/// TReadType should be in general always one type larger than T.
	/// </summary>
	template<typename T, typename TReadType, bool UseMultiElement = sizeof(T) == sizeof(TReadType)>
	bool Read(T& value, const uint8_t bitSize)
	{
		static_assert(sizeof(T) < sizeof(TReadType) || UseMultiElement, "To read the type without multi element it needs to be larger than the read type.");

		if (bitSize > force_consteval<uint8_t, sizeof(T) * 8>)
		{
			return false;
		}

		const uint64_t newPosition = m_offset + bitSize;

		// check if the amount of bits to read is available
		if (newPosition > m_capacity)
		{
			return false;
		}

		// the amount of bits that are currently used inside the current TReadType
		const uint8_t usedBits = m_offset % 8;
		const uint64_t byteIndex = m_offset >> 3;
		// TReadType pointer of the current byte position to start reading from
		const TReadType* start = reinterpret_cast<const TReadType*>(m_data + (byteIndex));
		const uint8_t maxReadableBits = usedBits + static_cast<uint8_t>(bitSize);
		// multi element means the new value does not fit into the current TReadType
		const bool multiElement = maxReadableBits > force_consteval<uint8_t, sizeof(TReadType) * 8>;

		// value to read does not fit into one element
		if (net::force_consteval<bool, UseMultiElement> && multiElement)
		{
			// the remaining bits to read after the remaining TReadType is fully read
			const uint8_t bitsToFill = 8 - usedBits;
			// reads the remaining bits from the current T*
			const T startElement = (((static_cast<T>(*start) >> usedBits) >> (8 - bitSize)) & GetBitMask<T>(bitsToFill));
			// then reads the remaining bits from the next uint8_t*
			const T endElement = ((*reinterpret_cast<const T*>(reinterpret_cast<const uint8_t*>(start) + 1)) << bitsToFill);
			// and the combine is the final value
			value = startElement | endElement;
			m_offset = newPosition;
			return true;
		}

		// careful read with sizeReads to prevent reading over the buffer limit, even when it's with a bit mask
		// this happens on the last calls
		const uint64_t freeBits = m_capacity - (byteIndex << 3);
		if (freeBits < force_consteval<uint8_t, sizeof(TReadType) * 8>)
		{
			// writes the used bits and the remaining bites to read as the full bytes to a temp value
			TReadType temp{ 0 };
			// converts the readable bits to bytes by adding 7 and dividing by 8
			const uint8_t bytesToRead = (maxReadableBits + 7) >> 3;
			memcpy(&temp, start, bytesToRead);

			// the temp value is now readable
			value = ReadBitArea<T, TReadType>(temp, usedBits, bitSize);
			m_offset = newPosition;
			return true;
		}

		// most simple read, the value is fully inside the current uint64_t
		// and the uint64_t is not reading above the buffer limit
		value = ReadBitArea<T, TReadType>(*start, usedBits, bitSize);
		m_offset = newPosition;

		return true;
	}

public:
	static constexpr Type Type = Type::Reader;

	BitReader(const uint8_t* _data, const uint64_t _capacity)
		: m_data(_data), m_capacity(_capacity)
	{
	}

	BitReader(const BitReader&) = delete;

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

	/// <summary>
	/// Reads a T from a bit stream
	/// </summary>
	/// <param name="value">The value to write the read content into.</param>
	/// <param name="bitSize">The value size to read in bits.</param>
	/// <returns>Returns true if the value is filled, otherwise false.</returns>
	template<typename T>
	bool Field(T& value, const size_t bitSize)
	{
		return Read<T, uint64_t>(value, static_cast<uint8_t>(bitSize));
	}

	/// <summary>
	/// Reads a T from a bit stream
	/// </summary>
	/// <param name="value">The value to write the read content into.</param>
	/// <returns>Returns false, because the generic Field implementation needs always a explicit size.</returns>
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
/// Reads a uint64_t from a bit stream
/// </summary>
/// <param name="value">The value to write the read content into.</param>
/// <param name="bitSize">The value size to read in bits.</param>
/// <returns>Returns true if the value is filled, otherwise false.</returns>
template<>
inline bool BitReader::Field<uint64_t>(uint64_t& value, const size_t bitSize)
{
	return Read<uint64_t, uint64_t, true>(value, static_cast<uint8_t>(bitSize));
}

/// <summary>
/// Reads a uint16_t from a bit stream
/// </summary>
/// <param name="value">The value to write the read content into.</param>
/// <param name="bitSize">The value size to read in bits.</param>
/// <returns>Returns true if the value is filled, otherwise false.</returns>
template<>
inline bool BitReader::Field<uint16_t>(uint16_t& value, const size_t bitSize)
{
	return Read<uint16_t, uint32_t>(value, static_cast<uint8_t>(bitSize));
}

/// <summary>
/// Reads a uint8_t from a bit stream
/// </summary>
/// <param name="value">The value to write the read content into.</param>
/// <param name="bitSize">The value size to read in bits.</param>
/// <returns>Returns true if the value is filled, otherwise false.</returns>
template<>
inline bool BitReader::Field<uint8_t>(uint8_t& value, const size_t bitSize)
{
	return Read<uint8_t, uint16_t>(value, static_cast<uint8_t>(bitSize));
}

/// <summary>
/// Reads a bool from a bit stream. This is always read as a single bit, that is why it does not need a bit size.
/// </summary>
/// <param name="val">The value to write the read content into.</param>
/// <returns>Returns true if the value is filled, otherwise false.</returns>
template<>
inline bool BitReader::Field<bool>(bool& val)
{
	const uint64_t newPosition = m_offset + 1;

	if (newPosition > m_capacity)
	{
		return false;
	}

	const uint64_t startByteIndex = m_offset >> 3;
	const uint8_t shift = (7 - (m_offset % 8));
	val = static_cast<uint8_t>((m_data[startByteIndex] >> shift) & 1);
	m_offset = newPosition;
	return true;
}

/// <summary>
/// Reads a bool from a bit stream. This is always read as a single bit, that is why it does not need a bit size.
/// This is adding the bitSize parameter, to stay compatible with the Field method.
/// </summary>
/// <param name="val">The value to write the read content into.</param>
/// <returns>Returns true if the value is filled, otherwise false.</returns>
template<>
inline bool BitReader::Field<bool>(bool& val, const size_t bitSize)
{
	if (bitSize != 1)
	{
		return false;
	}

	return Field<bool>(val);
}
}
