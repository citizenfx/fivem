#pragma once

#include <cstdint>

namespace fx
{
	class TestUtils
	{
	public:
		TestUtils() = delete;

		static uint64_t u64Random(uint64_t range);
	};
}
