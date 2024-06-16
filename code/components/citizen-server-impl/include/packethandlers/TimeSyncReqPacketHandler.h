#pragma once

#include <ServerInstanceBase.h>

#include <Client.h>

#include <ServerTime.h>

class TimeSyncReqPacketHandler
{
public:
	TimeSyncReqPacketHandler(fx::ServerInstanceBase*)
	{
	}

	template<typename Client = fx::ClientSharedPtr>
	void Handle(fx::ServerInstanceBase* instance, const Client& client, net::Buffer& packet)
	{
		const uint32_t reqTime = packet.Read<uint32_t>();
		const uint32_t reqSeq = packet.Read<uint32_t>();

		net::Buffer netBuffer;
		netBuffer.Write<uint32_t>(HashRageString("msgTimeSync"));
		netBuffer.Write<uint32_t>(reqTime);
		netBuffer.Write<uint32_t>(reqSeq);
		netBuffer.Write<uint32_t>((msec().count()) & 0xFFFFFFFF);

		client->SendPacket(1, netBuffer, NetPacketType_Reliable);
	}

	static constexpr const char* GetPacketId()
	{
		return "msgTimeSyncReq";
	}
};
