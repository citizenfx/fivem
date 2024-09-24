#include <StdInc.h>
#include <random>

#include <catch_amalgamated.hpp>

#include <state/RlMessageBuffer.h>

#include "BitReader.h"
#include "BitWriter.h"
#include "TestUtils.h"

using namespace fx;

static bool g_lengthHack{true};

namespace rl
{
	bool MessageBufferLengthHack::GetState()
	{
		return g_lengthHack;
	}
}

struct TestValue
{
	uint8_t type;
	uint8_t bitSize;
	uint64_t value;
};

struct TestValue32
{
	uint8_t type;
	uint8_t bitSize;
	uint32_t value;
};

TEST_CASE("rl::MessageBuffer random data test")
{
	// test with and without length hack
	g_lengthHack = GENERATE(true, false);
	// run in both length hacks
	constexpr uint32_t count{200000};
	std::vector<TestValue> testValues;
	testValues.resize(count);
	uint32_t bufferBitSize{0};
	for (uint32_t i = 0; i < count; ++i)
	{
		// generate random type [uint8, uint16, uint32, uint64]
		// RlMessageBuffer does not yet support 64 bit values
		auto type = TestUtils::u64Random(2);
		testValues[i].type = type;
		switch (type)
		{
		case 0:
			{
				// generate random bit sizes between 1 and 8
				testValues[i].bitSize = TestUtils::u64Random(7) + 1;
				testValues[i].value = TestUtils::u64Random((1 << testValues[i].bitSize) - 1);
				bufferBitSize += testValues[i].bitSize;
				break;
			}
		case 1:
			{
				// generate random bit sizes between 1 and 16
				testValues[i].bitSize = TestUtils::u64Random(15) + 1;
				testValues[i].value = TestUtils::u64Random((1 << testValues[i].bitSize) - 1);
				if (g_lengthHack && testValues[i].bitSize == 13)
				{
					bufferBitSize += 16;
				}
				else
				{
					bufferBitSize += testValues[i].bitSize;
				}
				break;
			}
		case 2:
			{
				// generate random bit sizes between 1 and 32
				testValues[i].bitSize = TestUtils::u64Random(31) + 1;
				testValues[i].value = TestUtils::u64Random((1 << testValues[i].bitSize) - 1);
				if (g_lengthHack && testValues[i].bitSize == 13)
				{
					bufferBitSize += 16;
				}
				else
				{
					bufferBitSize += testValues[i].bitSize;
				}
				break;
			}
		//case 3:
		//	{
		//		// generate random bit sizes between 1 and 64
		//		testValues[i].bitSize = TestUtils::u64Random(63) + 1;
		//		testValues[i].value = TestUtils::u64Random((1 << testValues[i].bitSize) - 1);
		//		if (g_lengthHack && testValues[i].bitSize == 13)
		//		{
		//			bufferBitSize += 16;
		//		} else
		//		{
		//			bufferBitSize += testValues[i].bitSize;
		//		}
		//		break;
		//	}
		default:
			REQUIRE(false);
			break;
		}
	}

	std::vector<uint8_t> data{};
	data.resize(((bufferBitSize / 8) + ((bufferBitSize % 8) ? 1 : 0)));
	rl::MessageBuffer buffer{data};
	uint32_t currBitOffset{0};
	for (uint32_t i = 0; i < count; ++i)
	{
		REQUIRE(testValues[i].bitSize > 0);
		switch (testValues[i].type)
		{
		case 0:
			{
				REQUIRE(testValues[i].bitSize < 9);
				uint8_t value = static_cast<uint8_t>(testValues[i].value);
				REQUIRE(buffer.WriteBitsSingle<uint8_t>(&value, testValues[i].bitSize) == true);
				break;
			}
		case 1:
			{
				REQUIRE(testValues[i].bitSize < 17);
				uint16_t value = static_cast<uint16_t>(testValues[i].value);
				REQUIRE(buffer.WriteBitsSingle<uint16_t>(&value, testValues[i].bitSize) == true);
				break;
			}
		case 2:
			{
				REQUIRE(testValues[i].bitSize < 33);
				uint32_t value = static_cast<uint32_t>(testValues[i].value);
				REQUIRE(buffer.WriteBitsSingle<uint32_t>(&value, testValues[i].bitSize) == true);
				break;
			}
		//case 3:
		//	{
		//		REQUIRE(testValues[i].bitSize < 65);
		//		uint64_t value = static_cast<uint64_t>(testValues[i].value);
		//		REQUIRE(buffer.WriteBitsSingle<uint64_t>(reinterpret_cast<uint64_t*>(&testValues[i].value), testValues[i].bitSize) == true);
		//		break;
		//	}
		default:
			REQUIRE(false);
			break;
		}
		if (g_lengthHack && testValues[i].bitSize == 13)
		{
			currBitOffset += 16;
		}
		else
		{
			currBitOffset += testValues[i].bitSize;
		}
		REQUIRE(buffer.GetCurrentBit() == currBitOffset);
	}
	REQUIRE(buffer.GetCurrentBit() == bufferBitSize);
	REQUIRE(buffer.GetDataLength() == ((bufferBitSize / 8) + ((bufferBitSize % 8) ? 1 : 0)));
	// is only at end if the bits are aligned correctly, which is not always the case
	REQUIRE(buffer.IsAtEnd() == (bufferBitSize % 8 ? false : true));
	if (data.size() * 8 != bufferBitSize)
	{
		// read the bits that are basically the padding for the buffer
		uint64_t padding{};
		REQUIRE(buffer.ReadBitsSingle(&padding, (data.size() * 8) - bufferBitSize) == true);
		REQUIRE(buffer.IsAtEnd() == true);
	}

	// reset position to get to the start
	// unlike the net::Buffer, this one does not have a function for resetting the bit to the start, it needs to be manually called with SetCurrentBit(0).
	buffer.SetCurrentBit(0);
	REQUIRE(buffer.IsAtEnd() == false);
	REQUIRE(buffer.GetCurrentBit() == 0);
	currBitOffset = 0;
	for (uint32_t i = 0; i < count; ++i)
	{
		REQUIRE(buffer.GetCurrentBit() == currBitOffset);
		switch (testValues[i].type)
		{
		case 0:
			{
				uint8_t value;
				REQUIRE(buffer.ReadBitsSingle<uint8_t>(&value, testValues[i].bitSize) == true);
				REQUIRE(testValues[i].value == value);
				break;
			}
		case 1:
			{
				uint16_t value;
				REQUIRE(buffer.ReadBitsSingle<uint16_t>(&value, testValues[i].bitSize) == true);
				REQUIRE(testValues[i].value == value);
				break;
			}
		case 2:
			{
				uint32_t value;
				REQUIRE(buffer.ReadBitsSingle<uint32_t>(&value, testValues[i].bitSize) == true);
				REQUIRE(testValues[i].value == value);
				break;
			}
		//case 3:
		//	{
		//		uint64_t value;
		//		REQUIRE(buffer.ReadBitsSingle<uint64_t>(&value, testValues[i].bitSize) == true);
		//		REQUIRE(testValues[i].value == value);
		//		break;
		//	}
		default:
			REQUIRE(false);
			break;
		}
		if (g_lengthHack && testValues[i].bitSize == 13)
		{
			currBitOffset += 16;
		}
		else
		{
			currBitOffset += testValues[i].bitSize;
		}
	}
	// is only at end if the bits are aligned correctly, which is not always the case
	REQUIRE(buffer.IsAtEnd() == (bufferBitSize % 8 ? false : true));
	if (data.size() * 8 != bufferBitSize)
	{
		// read the bits that are basically the padding for the buffer
		uint64_t padding{};
		REQUIRE(buffer.ReadBitsSingle(&padding, (data.size() * 8) - bufferBitSize) == true);
		REQUIRE(buffer.IsAtEnd() == true);
	}
}

TEST_CASE("new bitbuffer random data test")
{
	constexpr uint32_t count{2000};
	std::vector<TestValue> testValues;
	testValues.resize(count);
	uint32_t bufferBitSize{0};
	for (uint32_t i = 0; i < count; ++i)
	{
		// generate random type [uint8, uint16, uint32, uint64]
		// RlMessageBuffer does not yet support 64 bit values
		auto type = TestUtils::u64Random(2);
		testValues[i].type = type;
		switch (type)
		{
		case 0:
			{
				// generate random bit sizes between 1 and 8
				testValues[i].bitSize = TestUtils::u64Random(7) + 1;
				testValues[i].value = TestUtils::u64Random((1 << testValues[i].bitSize) - 1);
				bufferBitSize += testValues[i].bitSize;
				break;
			}
		case 1:
			{
				// generate random bit sizes between 1 and 16
				testValues[i].bitSize = TestUtils::u64Random(15) + 1;
				testValues[i].value = TestUtils::u64Random((1 << testValues[i].bitSize) - 1);
				bufferBitSize += testValues[i].bitSize;
				break;
			}
		case 2:
			{
				// generate random bit sizes between 1 and 32
				testValues[i].bitSize = TestUtils::u64Random(31) + 1;
				testValues[i].value = TestUtils::u64Random((1 << testValues[i].bitSize) - 1);
				bufferBitSize += testValues[i].bitSize;
				break;
			}
		case 3:
			{
				// generate random bit sizes between 1 and 64
				testValues[i].bitSize = TestUtils::u64Random(63) + 1;
				testValues[i].value = TestUtils::u64Random((1 << testValues[i].bitSize) - 1);
				bufferBitSize += testValues[i].bitSize;
				break;
			}
		default:
			REQUIRE(false);
			break;
		}
	}

	std::vector<uint8_t> data{};
	data.resize(((bufferBitSize / 8) + ((bufferBitSize % 8) ? 1 : 0)));
	net::BitWriter buffer{data.data(), data.size() * 8};
	uint32_t currBitOffset{0};
	for (uint32_t i = 0; i < count; ++i)
	{
		REQUIRE(testValues[i].bitSize > 0);
		switch (testValues[i].type)
		{
		case 0:
			{
				REQUIRE(testValues[i].bitSize < 9);
				REQUIRE(buffer.Field(*reinterpret_cast<uint8_t*>(&testValues[i].value), testValues[i].bitSize) == true);
				break;
			}
		case 1:
			{
				REQUIRE(testValues[i].bitSize < 17);
				REQUIRE(buffer.Field(*reinterpret_cast<uint16_t*>(&testValues[i].value), testValues[i].bitSize) == true);
				break;
			}
		case 2:
			{
				REQUIRE(testValues[i].bitSize < 33);
				REQUIRE(buffer.Field(*reinterpret_cast<uint32_t*>(&testValues[i].value), testValues[i].bitSize) == true);
				break;
			}
		case 3:
			{
				REQUIRE(testValues[i].bitSize < 65);
				REQUIRE(buffer.Field(testValues[i].value, testValues[i].bitSize) == true);
				break;
			}
		default:
			REQUIRE(false);
			break;
		}
		currBitOffset += testValues[i].bitSize;
		REQUIRE(buffer.GetOffset() == currBitOffset);
	}
	REQUIRE(buffer.GetOffset() == bufferBitSize);
	REQUIRE(buffer.GetCapacity() == ((bufferBitSize / 8) + ((bufferBitSize % 8) ? 1 : 0)) * 8);
	// is only at end if the bits are aligned correctly, which is not always the case
	REQUIRE(buffer.IsAtEnd() == (bufferBitSize % 8 ? false : true));
	if (data.size() * 8 != bufferBitSize)
	{
		// read the bits that are basically the padding for the buffer
		uint32_t padding{};
		REQUIRE(buffer.Field(padding, (data.size() * 8) - bufferBitSize) == true);
		REQUIRE(buffer.IsAtEnd() == true);
	}

	// reset position to get to the start
	// unlike the net::Buffer, this one does not have a function for resetting the bit to the start, it needs to be manually called with SetCurrentBit(0).
	buffer.Reset();
	REQUIRE(buffer.IsAtEnd() == false);
	REQUIRE(buffer.GetOffset() == 0);
	currBitOffset = 0;

	net::BitReader bufferReader{data.data(), data.size() * 8};

	for (uint32_t i = 0; i < count; ++i)
	{
		REQUIRE(bufferReader.GetOffset() == currBitOffset);
		switch (testValues[i].type)
		{
		case 0:
			{
				uint8_t value;
				REQUIRE(bufferReader.Field(value, testValues[i].bitSize) == true);
				REQUIRE(testValues[i].value == value);
				break;
			}
		case 1:
			{
				uint16_t value;
				REQUIRE(bufferReader.Field(value, testValues[i].bitSize) == true);
				REQUIRE(testValues[i].value == value);
				break;
			}
		case 2:
			{
				uint32_t value;
				REQUIRE(bufferReader.Field(value, testValues[i].bitSize) == true);
				REQUIRE(testValues[i].value == value);
				break;
			}
		case 3:
			{
				uint64_t value;
				REQUIRE(bufferReader.Field(value, testValues[i].bitSize) == true);
				REQUIRE(testValues[i].value == value);
				break;
			}
		default:
			REQUIRE(false);
			break;
		}
		currBitOffset += testValues[i].bitSize;
	}
	// is only at end if the bits are aligned correctly, which is not always the case
	REQUIRE(bufferReader.IsAtEnd() == (bufferBitSize % 8 ? false : true));
	if (data.size() * 8 != bufferBitSize)
	{
		// read the bits that are basically the padding for the buffer
		uint32_t padding{};
		REQUIRE(bufferReader.Field(padding, (data.size() * 8) - bufferBitSize) == true);
		REQUIRE(bufferReader.IsAtEnd() == true);
	}
}

TEST_CASE("test exact size read at end")
{
	std::vector<uint8_t> data;
	data.resize(2);
	net::BitWriter writer{data.data(), data.size() * 8};
	uint32_t value = 15;
	REQUIRE(writer.Field(value, 4) == true);
	uint32_t value2 = 4095;
	REQUIRE(writer.Field(value2, 12) == true);
	net::BitReader reader{data.data(), data.size() * 8};
	uint32_t readValue{0};
	REQUIRE(reader.Field(readValue, 4) == true);
	REQUIRE(readValue == 15);
	uint32_t readValue2{0};
	REQUIRE(reader.Field(readValue2, 12) == true);
	REQUIRE(readValue2 == 4095);
}

TEST_CASE("test exact size read at end part 2")
{
	std::vector<uint8_t> data;
	data.resize(8);
	net::BitWriter writer{data.data(), data.size() * 8};
	uint32_t value = 15;
	REQUIRE(writer.Field(value, 16) == true);
	uint32_t value2 = 4095;
	REQUIRE(writer.Field(value2, 16) == true);
	net::BitReader reader{data.data(), data.size() * 8};
	uint32_t readValue{0};
	REQUIRE(reader.Field(readValue, 16) == true);
	REQUIRE(readValue == 15);
	uint32_t readValue2{0};
	REQUIRE(reader.Field(readValue2, 16) == true);
	REQUIRE(readValue2 == 4095);
}

TEST_CASE("bit buffer benchmark")
{
	constexpr uint32_t count{1000};
	std::vector<TestValue32> testValues;
	testValues.resize(count);
	uint32_t bufferBitSize{0};
	for (uint32_t i = 0; i < count; ++i)
	{
		// generate random type [uint8, uint16, uint32, uint64]
		// RlMessageBuffer does not yet support 64 bit values
		auto type = TestUtils::u64Random(2);
		testValues[i].type = type;
		switch (type)
		{
		case 0:
			{
				// generate random bit sizes between 1 and 8
				testValues[i].bitSize = TestUtils::u64Random(7) + 1;
				testValues[i].value = TestUtils::u64Random((1 << testValues[i].bitSize) - 1);
				bufferBitSize += testValues[i].bitSize;
				break;
			}
		case 1:
			{
				// generate random bit sizes between 1 and 16
				testValues[i].bitSize = TestUtils::u64Random(15) + 1;
				testValues[i].value = TestUtils::u64Random((1 << testValues[i].bitSize) - 1);
				bufferBitSize += testValues[i].bitSize;
				break;
			}
		case 2:
			{
				// generate random bit sizes between 1 and 32
				testValues[i].bitSize = TestUtils::u64Random(31) + 1;
				testValues[i].value = TestUtils::u64Random((1 << testValues[i].bitSize) - 1);
				bufferBitSize += testValues[i].bitSize;
				break;
			}
		default:
			REQUIRE(false);
			break;
		}
	}

	std::vector<uint8_t> data2{};
	data2.resize(((bufferBitSize / 8) + ((bufferBitSize % 8) ? 1 : 0)));
	rl::MessageBuffer bufferOld{data2};

	BENCHMARK("Write old")
	{
		bufferOld.SetCurrentBit(0);
		for (uint32_t i = 0; i < count; ++i)
		{
			switch (testValues[i].type)
			{
			case 0:
				{
					bufferOld.WriteBitsSingle<uint32_t>(&testValues[i].value, testValues[i].bitSize);
					break;
				}
			case 1:
				{
					bufferOld.WriteBitsSingle<uint32_t>(&testValues[i].value, testValues[i].bitSize);
					break;
				}
			case 2:
				{
					bufferOld.WriteBitsSingle<uint32_t>(&testValues[i].value, testValues[i].bitSize);
					break;
				}
			}
		}
	};
	
	std::vector<uint8_t> data{};
	data.resize(((bufferBitSize / 8) + ((bufferBitSize % 8) ? 1 : 0)));
	net::BitWriter buffer{data.data(), data.size() * 8};

	g_lengthHack = false;

	BENCHMARK("Write new")
	{
		buffer.Reset();
		for (uint32_t i = 0; i < count; ++i)
		{
			switch (testValues[i].type)
			{
				case 0:
				{
					buffer.Field(*reinterpret_cast<uint8_t*>(&testValues[i].value), testValues[i].bitSize);
					break;
				}
				case 1:
				{
					buffer.Field(*reinterpret_cast<uint16_t*>(&testValues[i].value), testValues[i].bitSize);
					break;
				}
				case 2:
				{
					buffer.Field(testValues[i].value, testValues[i].bitSize);
					break;
				}
			}
		}
	};

	BENCHMARK("Read old") {
		bufferOld.SetCurrentBit(0);
		for (uint32_t i = 0; i < count; ++i)
		{
			switch (testValues[i].type)
			{
			case 0:
				{
					bufferOld.ReadBitsSingle(&testValues[i].value, testValues[i].bitSize);
					break;
				}
			case 1:
				{
					bufferOld.ReadBitsSingle(&testValues[i].value, testValues[i].bitSize);
					break;
				}
			case 2:
				{
					bufferOld.ReadBitsSingle(&testValues[i].value, testValues[i].bitSize);
					break;
				}
			}
		}
	};

	net::BitReader bufferRead{data.data(), data.size() * 8};

	BENCHMARK("Read new") {
		bufferRead.Reset();
		for (uint32_t i = 0; i < count; ++i)
		{
			switch (testValues[i].type)
			{
				case 0:
				{
					bufferRead.Field(*reinterpret_cast<uint8_t*>(&testValues[i].value), testValues[i].bitSize);
					break;
				}
				case 1:
				{
					bufferRead.Field(*reinterpret_cast<uint16_t*>(&testValues[i].value), testValues[i].bitSize);
					break;
				}
				case 2:
				{
					bufferRead.Field(testValues[i].value, testValues[i].bitSize);
					break;
				}
			}
		}
	};
}
