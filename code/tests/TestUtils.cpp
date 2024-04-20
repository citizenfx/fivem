#include "TestUtils.h"

#include <random>

#include <catch_amalgamated.hpp>

uint64_t fx::TestUtils::u64Random(const uint64_t range)
{
	static std::seed_seq ssq{Catch::getSeed()};
	static std::default_random_engine eng{ssq};
	std::uniform_int_distribution<uint64_t> dist(0, range);
	return dist(eng);
}

std::string fx::TestUtils::asciiRandom(const uint64_t count)
{
	static std::seed_seq ssq{Catch::getSeed()};
	static std::default_random_engine eng{ssq};
	std::string val;
	val.resize(count);
	std::uniform_int_distribution<uint64_t> dist(0, 61);
	for (uint64_t i = 0; i < count; ++i)
	{
		const uint64_t generatedValue = dist(eng);
		// 'a' - 'z'
		if (generatedValue < 26)
		{
			val[i] = static_cast<char>('a' + generatedValue);
			continue;
		}
		// 'A' - 'Z'
		if (generatedValue < 52)
		{
			val[i] = static_cast<char>('A' + generatedValue - 26);
			continue;
		}
		// '0' - '9'
		if (generatedValue < 62)
		{
			val[i] = static_cast<char>('0' + generatedValue - 52);
		}
	}

	return val;
}

void fx::TestUtils::fillVectorU8Random(std::vector<uint8_t>& data)
{
	static std::seed_seq ssq{Catch::getSeed()};
	static std::default_random_engine eng{ssq};
	std::uniform_int_distribution<uint64_t> dist(0, 255);
	for (uint64_t i = 0, length = data.size(); i < length; ++i)
	{
		data[i] = static_cast<uint8_t>(dist(eng));
	}
}
