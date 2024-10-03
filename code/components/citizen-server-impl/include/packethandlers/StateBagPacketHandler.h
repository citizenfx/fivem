#pragma once

#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>

#include "ByteReader.h"
#include "ComponentExport.h"
#include "ENetPacketUniquePtr.h"
#include "PacketHandler.h"
#include "StateBag.h"

class StateBagPacketHandler : public net::PacketHandler<net::packet::StateBag, HashRageString("msgStateBag")>
{
public:
	StateBagPacketHandler(fx::ServerInstanceBase* instance)
	{
	}

	bool COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) Process(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::ByteReader& reader, fx::ENetPacketPtr& packet);

	static void COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) HandleStateBagMessage(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::ByteReader& reader);
};

class StateBagPacketHandlerV2 : public net::PacketHandler<net::packet::StateBagV2, HashRageString("msgStateBagV2")>
{
public:
	StateBagPacketHandlerV2(fx::ServerInstanceBase* instance)
	{
	}

	bool COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) Process(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::ByteReader& reader, fx::ENetPacketPtr& packet);

	static void COMPONENT_EXPORT(CITIZEN_SERVER_IMPL) HandleStateBagMessage(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::ByteReader& reader);
};
