#include <StdInc.h>
#include <random>

#include <catch_amalgamated.hpp>

#include <NetBuffer.h>

struct TestValue 
{
	uint8_t size;
	uint64_t value;
};

uint64_t u64Random(uint64_t range)
{
	static std::random_device randomDevice;
	static std::mt19937 gen(randomDevice());
	std::uniform_int_distribution<uint64_t> dist(0, range - 1);
	return dist(gen);
}

TEST_CASE("Buffer random data test")
{
	constexpr uint32_t count{ 200000 };
	std::vector<TestValue> testValues;
	testValues.resize(count);
	net::Buffer buffer{};
	uint32_t bufferSize{ 0 };
	for (uint32_t i = 0; i < count; ++i) 
	{
		auto size = u64Random(3);
		testValues[i].size = size;
		switch (size) 
		{
		case 0:
			testValues[i].value = u64Random(0xff);
			buffer.Write<uint8_t>(testValues[i].value);
			bufferSize += 1;
			break;
		case 1:
			testValues[i].value = u64Random(0xffff);
			buffer.Write<uint16_t>(testValues[i].value);
			bufferSize += 2;
			break;
		case 2:
			testValues[i].value = u64Random(0xffffffff);
			buffer.Write<uint32_t>(testValues[i].value);
			bufferSize += 4;
			break;
		case 3:
			testValues[i].value = u64Random(0xffffffffffffffff);
			buffer.Write<uint64_t>(testValues[i].value);
			bufferSize += 8;
			break;
		}
	}
	REQUIRE(buffer.GetCurOffset() == bufferSize);
	REQUIRE(buffer.GetBytes().get()->size() == bufferSize);
	REQUIRE(buffer.IsAtEnd() == true);
	// reset position to get to the start
	buffer.Reset();
	REQUIRE(buffer.IsAtEnd() == false);
	for (uint32_t i = 0; i < count; ++i)
	{
		switch (testValues[i].size)
		{
		case 0:
			REQUIRE(testValues[i].value == buffer.Read<uint8_t>());
			break;
		case 1:
			REQUIRE(testValues[i].value == buffer.Read<uint16_t>());
			break;
		case 2:
			REQUIRE(testValues[i].value == buffer.Read<uint32_t>());
			break;
		case 3:
			REQUIRE(testValues[i].value == buffer.Read<uint64_t>());
			break;
		}
	}
	REQUIRE(buffer.IsAtEnd() == true);
}

TEST_CASE("Buffer read empty")
{
	net::Buffer buffer{};
	REQUIRE(buffer.Read<uint64_t>() == 0);
}

TEST_CASE("Buffer end when data can not be read")
{
	net::Buffer buffer{};
	buffer.Write<uint16_t>(15);
	uint16_t test{ 123 };
	REQUIRE(buffer.Read<uint32_t>() == 0);
	REQUIRE(buffer.IsAtEnd() == true);
}

struct ValueHolder 
{
	uint8_t tempValue;
	uint8_t padding{ 123 };
};

TEST_CASE("Buffer overflow")
{
	net::Buffer buffer{};
	ValueHolder holder{};
	REQUIRE(buffer.Read(&holder.tempValue, sizeof(uint8_t)) == false);
	REQUIRE(holder.tempValue == 0);
	REQUIRE(holder.padding == 123); // was previously 0xCE
}
