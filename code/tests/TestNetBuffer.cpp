#include <StdInc.h>
#include <random>

#include <catch_amalgamated.hpp>

#include <NetBuffer.h>

#include "TestUtils.h"

using namespace fx;

struct TestValue
{
	uint8_t size;
	uint64_t value;
};

TEST_CASE("Buffer random data test")
{
	constexpr uint32_t count{200000};
	std::vector<TestValue> testValues;
	testValues.resize(count);
	net::Buffer buffer{};
	uint32_t bufferSize{0};
	for (uint32_t i = 0; i < count; ++i)
	{
		auto size = TestUtils::u64Random(3);
		testValues[i].size = size;
		switch (size)
		{
		case 0:
			testValues[i].value = TestUtils::u64Random(UINT8_MAX);
			buffer.Write<uint8_t>(testValues[i].value);
			bufferSize += 1;
			break;
		case 1:
			testValues[i].value = TestUtils::u64Random(UINT16_MAX);
			buffer.Write<uint16_t>(testValues[i].value);
			bufferSize += 2;
			break;
		case 2:
			testValues[i].value = TestUtils::u64Random(UINT32_MAX);
			buffer.Write<uint32_t>(testValues[i].value);
			bufferSize += 4;
			break;
		case 3:
			testValues[i].value = TestUtils::u64Random(UINT64_MAX);
			buffer.Write<uint64_t>(testValues[i].value);
			bufferSize += 8;
			break;
		default:
			REQUIRE(false);
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
		default:
			REQUIRE(false);
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
	REQUIRE(buffer.Read<uint32_t>() == 0);
	REQUIRE(buffer.IsAtEnd() == true);
}

struct ValueHolder
{
	uint8_t tempValue;
	uint8_t padding{123};
};

TEST_CASE("Buffer overflow")
{
	net::Buffer buffer{};
	ValueHolder holder{};
	REQUIRE(buffer.Read(&holder.tempValue, sizeof(uint8_t)) == false);
	REQUIRE(holder.tempValue == 0);
	REQUIRE(holder.padding == 123); // was previously 0xCE
}

TEST_CASE("Buffer ReadTo")
{
	net::Buffer bufferToRead{};
	net::Buffer bufferToWrite{};
	bufferToRead.Write<uint8_t>(1);
	bufferToRead.Write<uint8_t>(2);
	bufferToRead.Reset();
	REQUIRE(bufferToRead.ReadTo(bufferToWrite, 2) == true);
	bufferToWrite.Reset();
	REQUIRE(bufferToWrite.Read<uint8_t>() == 1);
	REQUIRE(bufferToWrite.Read<uint8_t>() == 2);
}

TEST_CASE("Buffer read string and string_view")
{
	net::Buffer buffer{};
	const char* str = "buffer";
	buffer.Write(str, 6);
	buffer.Reset();
	std::string_view view = buffer.Read<std::string_view>(6);
	REQUIRE(str == view);
	buffer.Reset();
	std::string string = buffer.Read<std::string>(6);
	REQUIRE(str == string);
}

TEST_CASE("Buffer can read")
{
	net::Buffer buffer{};
	const uint64_t writeAmountOfBytes = TestUtils::u64Random(99) + 1;
	for (uint64_t i = writeAmountOfBytes; i--;)
	{
		buffer.Write<uint8_t>(1);
	}
	buffer.Reset();
	REQUIRE(buffer.CanRead(writeAmountOfBytes) == true);
	REQUIRE(buffer.CanRead(writeAmountOfBytes + 1) == false);
}
