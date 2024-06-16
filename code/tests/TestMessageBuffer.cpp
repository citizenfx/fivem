#include <StdInc.h>
#include <random>

#include <catch_amalgamated.hpp>

#include <state/RlMessageBuffer.h>

#include "TestUtils.h"

using namespace fx;

static bool g_lengthHack{true};

namespace rl
{
	bool MessageBuffer::GetLengthHackState()
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
