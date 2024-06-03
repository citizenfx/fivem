#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include "GameServer.h"
#include "ServerInstance.h"
#include "packethandlers/TimeSyncReqPacketHandler.h"

#include "TestUtils.h"

namespace fx
{
	class FakeClient: public fwRefCountable
	{
	public:
		class TimeSyncResponse
		{
		public:
			const int channel;
			const net::Buffer buffer;
			const NetPacketType flags;

			TimeSyncResponse(const int channel, const net::Buffer& buffer,
							 const NetPacketType flags)
				: channel(channel),
				  buffer(buffer),
				  flags(flags)
			{
			}
		};

		static inline std::optional<TimeSyncResponse> lastTimeSyncResponse;

		void SendPacket(int channel, const net::Buffer& buffer, NetPacketType type /* = (ENetPacketFlag)0 */)
		{
			lastTimeSyncResponse.emplace(channel, buffer, type);
		}
	};
}

TEST_CASE("time sync request handler test")
{
	REQUIRE(std::string(TimeSyncReqPacketHandler::GetPacketId()) == "msgTimeSyncReq");
	REQUIRE(HashRageString(TimeSyncReqPacketHandler::GetPacketId()) == 0x1c1303f8);

	fx::ServerInstanceBase* serverInstance = ServerInstance::Create();

	uint32_t reqTime = fx::TestUtils::u64Random(0xFFFFFFFF);
	uint32_t reqSeq = fx::TestUtils::u64Random(0xFFFFFFFF);
	
	net::Buffer requestBuffer;
	requestBuffer.Write<uint32_t>(reqTime);
	requestBuffer.Write<uint32_t>(reqSeq);
	requestBuffer.Reset();

	fx::FakeClient::lastTimeSyncResponse.reset();

	uint32_t before = (msec().count()) & 0xFFFFFFFF;
	fwRefContainer client = {new fx::FakeClient()};
	TimeSyncReqPacketHandler handler(serverInstance);
	handler.Handle(serverInstance, client, requestBuffer);
	uint32_t after = (msec().count()) & 0xFFFFFFFF;

	REQUIRE(fx::FakeClient::lastTimeSyncResponse.has_value() == true);

	net::Buffer responseBuffer = fx::FakeClient::lastTimeSyncResponse.value().buffer;
	responseBuffer.Reset();
	REQUIRE(responseBuffer.Read<uint32_t>() == HashRageString("msgTimeSync"));
	REQUIRE(responseBuffer.Read<uint32_t>() == reqTime);
	REQUIRE(responseBuffer.Read<uint32_t>() == reqSeq);
	uint32_t time = responseBuffer.Read<uint32_t>();
	REQUIRE(time <= before);
	REQUIRE(time >= after);
	REQUIRE(fx::FakeClient::lastTimeSyncResponse.value().channel == 1);
	REQUIRE(fx::FakeClient::lastTimeSyncResponse.value().flags == NetPacketType::NetPacketType_Reliable);

	delete serverInstance;
}
