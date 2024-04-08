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
