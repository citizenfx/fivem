#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace fx
{
	class TestUtils
	{
	public:
		TestUtils() = delete;

		static uint64_t u64Random(uint64_t range);
		static std::string asciiRandom(uint64_t count);
		static void fillVectorU8Random(std::vector<uint8_t>& data);
	};
}
