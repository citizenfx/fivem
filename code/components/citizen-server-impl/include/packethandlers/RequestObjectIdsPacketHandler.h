#pragma once

#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>

#include "ByteReader.h"
#include "ComponentExport.h"
#include "ENetPacketUniquePtr.h"
#include "ObjectIds.h"
#include "PacketHandler.h"

class RequestObjectIdsPacketHandler : public net::PacketHandler<net::packet::ClientRequestObjectIds, HashRageString("msgRequestObjectIds")>
{
public:
	RequestObjectIdsPacketHandler(fx::ServerInstanceBase* instance)
	{
	}

	bool COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) Process(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::ByteReader& reader, fx::ENetPacketPtr& packet);

	// helper method to send object ids to a client
	static void SendObjectIds(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, uint8_t numIds);
};
