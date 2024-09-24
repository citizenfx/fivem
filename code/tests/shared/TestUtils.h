#pragma once

#include <cstdint>
#include <random>
#include <string>
#include <vector>

namespace fx
{
	class TestUtils
	{
	public:
		TestUtils() = delete;

		/// <summary>
		/// Generates a random uint64_t between 0 and range
		/// </summary>
		/// <param name="range">max value to generate</param>
		/// <returns>the generated uint64_t</returns>
		static uint64_t u64Random(uint64_t range);

		/// <summary>
		/// Generates a random ascii std::string
		/// </summary>
		/// <param name="count">amount of characters to generate</param>
		/// <returns>the generated ascii string</returns>
		static std::string asciiRandom(uint64_t count);

		/// <summary>
		/// Fills the given std::vector<uint8_t> with random uint8_t values depending on its size
		/// </summary>
		/// <param name="data">std::vector to fill</param>
		static void fillVectorU8Random(std::vector<uint8_t>& data);
	private:
		static std::default_random_engine& getRandomEngine();
	};
}
