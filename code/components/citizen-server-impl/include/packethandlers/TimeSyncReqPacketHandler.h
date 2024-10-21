#pragma once

#include <ServerInstanceBase.h>

#include <Client.h>

#include <ServerTime.h>

#include "ByteReader.h"
#include "ByteWriter.h"
#include "TimeSync.h"
#include "PacketHandler.h"

class TimeSyncReqPacketHandler : public net::PacketHandler<net::packet::TimeSyncRequest, HashRageString("msgTimeSyncReq")>
{
public:
	TimeSyncReqPacketHandler(fx::ServerInstanceBase*)
	{
	}

	template<typename Client = fx::ClientSharedPtr>
	bool Process(fx::ServerInstanceBase* instance, const Client& client, net::ByteReader& reader, fx::ENetPacketPtr packet)
	{
		static size_t kMaxResponseSize = net::SerializableComponent::GetMaxSize<net::packet::TimeSyncResponsePacket>();
		thread_local net::packet::TimeSyncResponsePacket timeSyncResponse;

		return ProcessPacket(reader, [](const net::packet::TimeSyncRequest& timeSyncRequest, fx::ServerInstanceBase* instance, const Client& client)
		{
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
		}, instance, client);
	}
};
