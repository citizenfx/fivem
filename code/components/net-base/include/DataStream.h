#pragma once

#include <cstdint>

namespace net
{
	class DataStream
	{
	public:
		enum class Type: uint8_t
		{
			Reader,
			Writer,
			Counter,
			MaxCounter,
			MinCounter
		};

		uint64_t GetOffset() const
		{
			return 0;
		}

		uint64_t GetCapacity() const
		{
			return 0;
		}

		template <typename T>
		bool Field(T& value, const size_t size)
		{
			return false;
		}

		template <typename T>
		bool Field(T& value)
		{
			return false;
		}
	};
}
