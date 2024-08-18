#pragma once

#include "DataStream.h"

#include <ForceConsteval.h>

namespace net::storage_type
{
	class SerializableSizeOption
	{
	public:
		enum class Type : uint8_t
		{
			Value,
			Area
		};
	};

	template <uint8_t BitSize = 0>
	class Bits
	{
	public:
		static constexpr SerializableSizeOption::Type kType = SerializableSizeOption::Type::Value;

		static constexpr uint8_t kValueBitSize = BitSize;
	};

	template <typename ElementType, ElementType Min = 0, ElementType Max = 0, bool Remaining = false>
	class SerializableSizeOptionArea
	{
		static ElementType AdjustSize(ElementType size)
		{
			if constexpr (Min > 0 && Max > 0)
			{
				return std::max(Min, std::min(Max, size));
			}
			else if constexpr (Min > 0)
			{
				return std::max(Min, size);
			}
			else if constexpr (Max > 0)
			{
				return std::min(Max, size);
			}

			return size;
		}

		template<typename SizeType>
		static bool ValidateSize(SizeType size)
		{
			if constexpr (Min > 0 && Max > 0)
			{
				if (size < Min || size > Max)
				{
					return false;
				}
			}
			else if constexpr (Min > 0)
			{
				if (size < Min)
				{
					return false;
				}
			}
			else if constexpr (Max > 0)
			{
				if (size > Max)
				{
					return false;
				}
			}

			return true;
		}

	public:
		static constexpr SerializableSizeOption::Type kType = SerializableSizeOption::Type::Area;

		/// <summary>
		/// Processes the size of the property.
		/// </summary>
		/// <param name="stream">Stream to read/write the size to</param>
		/// <param name="suggestion">Suggested size, only used for writing.</param>
		/// <param name="valid">True when the received size is allowed, false otherwise</param>
		/// <returns>Returns the size</returns>
		template <typename T, typename U>
		static ElementType Process(T& stream, U suggestion, bool& valid)
		{
			if constexpr (T::kType == DataStream::Type::Reader)
			{
				if constexpr (Remaining)
				{
					// remaining does not need size read
					size_t size = stream.GetRemaining();

					// size is out of the given constraints
					if (!ValidateSize(size))
					{
						valid = false;

						return ElementType(0);
					}

					valid = true;

					return ElementType(size);
				}
				else
				{
					ElementType size;
					stream.Field(size);
					
					// size is out of the given constraints
					if (!ValidateSize(size))
					{
						valid = false;

						return ElementType(0);
					}

					valid = true;

					return size;
				}
			}
			else if constexpr (T::kType == DataStream::Type::Writer)
			{
				ElementType size = AdjustSize(ElementType(suggestion));
				if constexpr (!Remaining)
				{
					stream.Field(size);
				}

				valid = true;

				return size;
			}
			else if constexpr (T::kType == DataStream::Type::Counter)
			{
				ElementType size;
				if constexpr (Max > 0)
				{
					size = Max;
				}
				else
				{
					size = net::force_consteval<ElementType, std::numeric_limits<ElementType>::max()>;
				}

				if constexpr (!Remaining)
				{
					stream.Field(size);
				}

				valid = true;

				return size;
			}

			return ElementType(suggestion);
		}
	};

	/// <summary>
	/// Holds up to 255 elements.
	/// </summary>
	class SmallBytesArray : public SerializableSizeOptionArea<uint8_t> {};

	/// <summary>
	/// Holds up to 65535 elements.
	/// </summary>
	class BytesArray : public SerializableSizeOptionArea<uint16_t> {};

	/// <summary>
	/// Holds up to 4294967295 elements.
	/// </summary>
	class BigBytesArray : public SerializableSizeOptionArea<uint32_t> {};

	/// <summary>
	/// Can hold up to 255 elements.
	/// But is limited to the specified Min and Max.
	/// The constrains that are defined with 0 are ignored.
	/// </summary>
	template <uint8_t Min, uint8_t Max>
	class ConstrainedSmallBytesArray : public SerializableSizeOptionArea<uint8_t, Min, Max> {};

	/// <summary>
	/// Can hold up to 65535 elements.
	/// But is limited to the specified Min and Max.
	/// The constrains that are defined with 0 are ignored.
	/// </summary>
	template <uint16_t Min, uint16_t Max>
	class ConstrainedBytesArray : public SerializableSizeOptionArea<uint16_t, Min, Max> {};

	/// <summary>
	/// Can hold up to 4294967295 elements.
	/// But is limited to the specified Min and Max.
	/// The constrains that are defined with 0 are ignored.
	/// </summary>
	template <uint32_t Min, uint32_t Max>
	class ConstrainedBigBytesArray : public SerializableSizeOptionArea<uint32_t, Min, Max> {};

	/// <summary>
	/// Holds up to 2^64-1 elements.
	/// The element count is the remaining size of the stream.
	/// </summary>
	class StreamTail : public SerializableSizeOptionArea<size_t, 0, 0, true> {};

	/// <summary>
	/// Holds up to 2^64-1 elements.
	/// The element count is the remaining size of the stream.
	/// But is limited to the specified Min and Max.
	/// The constrains that are defined with 0 are ignored.
	/// </summary>
	template <uint32_t Min, uint32_t Max>
	class ConstrainedStreamTail : public SerializableSizeOptionArea<size_t, Min, Max, true> {};
}
