#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include "ByteReader.h"
#include "ByteWriter.h"
#include "GameStateNAck.h"
#include "NetBuffer.h"
#include "TestUtils.h"

TEST_CASE("game state nack test")
{
	// for serialization of the ignore list with a net::Span the struct needs to be correctly matching the size
	REQUIRE(sizeof(net::packet::ClientGameStateNAck::IgnoreListEntry) == 10);

	bool emptyIgnoreList = GENERATE(true, false);
	bool emptyRecreateList = GENERATE(true, false);
	bool isMissingFrames = GENERATE(true, false);
	uint64_t frameIndex = fx::TestUtils::u64Random(UINT64_MAX);
	uint64_t firstMissingFrame = fx::TestUtils::u64Random(UINT64_MAX);
	uint64_t lastMissingFrame = fx::TestUtils::u64Random(UINT64_MAX);
	std::vector<net::packet::ClientGameStateNAck::IgnoreListEntry> ignoreList(emptyIgnoreList ? 0 : fx::TestUtils::u64Random(UINT8_MAX - 1));
	for (uint64_t i = 0; i < ignoreList.size(); ++i)
	{
		ignoreList[i].entry = fx::TestUtils::u64Random(UINT16_MAX);
		ignoreList[i].lastFrame = fx::TestUtils::u64Random(UINT64_MAX);
	}

	std::vector<uint16_t> recreateList(emptyRecreateList ? 0 : fx::TestUtils::u64Random(UINT8_MAX - 1));
	for (uint64_t i = 0; i < recreateList.size(); ++i)
	{
		recreateList[i] = fx::TestUtils::u64Random(UINT16_MAX);
	}

	uint8_t flags = static_cast<uint8_t>(net::packet::ClientGameStateNAck::Flags::HasIgnoreListEntryResendFrame);
	if (isMissingFrames)
	{
		flags |= static_cast<uint8_t>(net::packet::ClientGameStateNAck::Flags::IsMissingFrames);
	}

	if (!ignoreList.empty())
	{
		flags |= static_cast<uint8_t>(net::packet::ClientGameStateNAck::Flags::HasIgnoreList);
	}

	if (!recreateList.empty())
	{
		flags |= static_cast<uint8_t>(net::packet::ClientGameStateNAck::Flags::HasRecreateList);
	}

	net::Buffer outBuffer;
	outBuffer.Write<uint8_t>(flags);

	outBuffer.Write<uint64_t>(frameIndex);

	if (isMissingFrames)
	{
		outBuffer.Write<uint64_t>(firstMissingFrame);
		outBuffer.Write<uint64_t>(lastMissingFrame);
	}

	if (!ignoreList.empty())
	{
		outBuffer.Write<uint8_t>(static_cast<uint8_t>(ignoreList.size()));
		for (auto [entry, lastFrame] : ignoreList)
		{
			outBuffer.Write<uint16_t>(entry);
			outBuffer.Write<uint64_t>(lastFrame);
		}
	}

	if (!recreateList.empty())
	{
		outBuffer.Write<uint8_t>(static_cast<uint8_t>(recreateList.size()));
		for (uint16_t entry : recreateList)
		{
			outBuffer.Write<uint16_t>(entry);
		}
	}

	net::packet::ClientGameStateNAck gameStateNAck;
	gameStateNAck.SetFrameIndex(frameIndex);
	if (isMissingFrames)
	{
		gameStateNAck.SetIsMissingFrames(isMissingFrames, firstMissingFrame, lastMissingFrame);
	}
	gameStateNAck.SetIgnoreList(ignoreList);
	gameStateNAck.SetRecreateList(recreateList);

	REQUIRE(net::SerializableComponent::GetSize<net::packet::ClientGameStateNAck>() == 3087);
	std::vector<uint8_t> buffer(net::SerializableComponent::GetSize<net::packet::ClientGameStateNAck>());
	net::ByteWriter writer {buffer.data(), buffer.size()};
	REQUIRE(gameStateNAck.Process(writer) == true);
	REQUIRE(gameStateNAck.GetFlags() == flags);

	REQUIRE(writer.GetOffset() == outBuffer.GetCurOffset());
	REQUIRE(memcmp(outBuffer.GetBuffer(), buffer.data(), writer.GetOffset()) == 0);

	net::packet::ClientGameStateNAck readGameStateNAck;
	net::ByteReader reader {buffer.data(), buffer.size()};
	REQUIRE(readGameStateNAck.Process(reader) == true);
	REQUIRE(readGameStateNAck.GetFlags() == flags);
	REQUIRE(readGameStateNAck.GetFrameIndex() == frameIndex);
	if (isMissingFrames)
	{
		REQUIRE(readGameStateNAck.GetFirstMissingFrame() == firstMissingFrame);
		REQUIRE(readGameStateNAck.GetLastMissingFrame() == lastMissingFrame);
	}

	REQUIRE(readGameStateNAck.GetIgnoreList().size() == ignoreList.size());
	for (uint16_t i = 0; i < ignoreList.size(); ++i)
	{
		REQUIRE(readGameStateNAck.GetIgnoreList()[i].entry == ignoreList[i].entry);
		REQUIRE(readGameStateNAck.GetIgnoreList()[i].lastFrame == ignoreList[i].lastFrame);
	}

	REQUIRE(readGameStateNAck.GetRecreateList().size() == recreateList.size());
	for (uint16_t i = 0; i < recreateList.size(); ++i)
	{
		REQUIRE(readGameStateNAck.GetRecreateList()[i] == recreateList[i]);
	}

	if (!emptyIgnoreList && !emptyRecreateList && !isMissingFrames)
	{
		BENCHMARK("new game state ack serialization") {
			std::vector<uint8_t> writeBuffer(net::SerializableComponent::GetSize<net::packet::ClientGameStateNAck>());
			net::ByteWriter gameStateNAckWriter {writeBuffer.data(), writeBuffer.size()};
			net::packet::ClientGameStateNAck gameStateNAckData;
			gameStateNAckData.SetFrameIndex(frameIndex);
			if (isMissingFrames)
			{
				gameStateNAckData.SetIsMissingFrames(isMissingFrames, firstMissingFrame, lastMissingFrame);
			}
			gameStateNAckData.SetIgnoreList(ignoreList);
			gameStateNAckData.SetRecreateList(recreateList);
			gameStateNAckData.Process(gameStateNAckWriter);
		};

		BENCHMARK("old game state ack serialization") {
			net::Buffer out;
			out.Write<uint8_t>(flags);

			out.Write<uint64_t>(frameIndex);

			if (isMissingFrames)
			{
				out.Write<uint64_t>(firstMissingFrame);
				out.Write<uint64_t>(lastMissingFrame);
			}

			if (!ignoreList.empty())
			{
				out.Write<uint8_t>(static_cast<uint8_t>(ignoreList.size()));
				for (auto [entry, lastFrame] : ignoreList)
				{
					out.Write<uint16_t>(entry);
					out.Write<uint64_t>(lastFrame);
				}
			}

			if (!recreateList.empty())
			{
				out.Write<uint8_t>(static_cast<uint8_t>(recreateList.size()));
				for (uint16_t entry : recreateList)
				{
					out.Write<uint16_t>(entry);
				}
			}
		};
	}
}
