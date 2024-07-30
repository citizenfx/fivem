#pragma once

#include <ServerInstanceBase.h>

#include <Client.h>

#include <ServerTime.h>

#include "ByteReader.h"
#include "ByteWriter.h"
#include "TimeSync.h"

class TimeSyncReqPacketHandler
{
public:
	TimeSyncReqPacketHandler(fx::ServerInstanceBase*)
	{
	}

	template<typename Client = fx::ClientSharedPtr>
	void Handle(fx::ServerInstanceBase* instance, const Client& client, net::Buffer& packet)
	{
		static size_t kMaxRequestSize = net::SerializableComponent::GetSize<net::packet::TimeSyncRequest>();
		static size_t kMaxResponseSize = net::SerializableComponent::GetSize<net::packet::TimeSyncResponsePacket>();
		thread_local net::packet::TimeSyncRequest timeSyncRequest;
		thread_local net::packet::TimeSyncResponsePacket timeSyncResponse;

		if (packet.GetRemainingBytes() != kMaxRequestSize)
		{
			return;
		}

		net::ByteReader reader { packet.GetRemainingBytesPtr(), packet.GetRemainingBytes() };
		if (!timeSyncRequest.Process(reader))
		{
			return;
		}

		timeSyncResponse.data.request = timeSyncRequest;
		timeSyncResponse.data.serverTimeMillis = msec().count() & 0xFFFFFFFF;

		net::Buffer responseBuffer(kMaxResponseSize);
		net::ByteWriter writer{ responseBuffer.GetBuffer(), kMaxResponseSize };
		if (!timeSyncResponse.Process(writer))
		{
			trace("Serialization of the server time sync response failed. Please report this error at https://github.com/citizenfx/fivem.\n");
			return;
		}

		responseBuffer.Seek(writer.GetOffset());

		client->SendPacket(1, responseBuffer, NetPacketType_Reliable);
	}

	static constexpr const char* GetPacketId()
	{
		return "msgTimeSyncReq";
	}
};
