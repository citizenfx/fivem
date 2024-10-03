#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>
#include <state/ServerGameStatePublic.h>

#include "packethandlers/GameStateNAckPacketHandler.h"

#include "ArrayUpdate.h"
#include "ByteReader.h"

bool fx::ServerDecorators::GameStateNAckPacketHandler::Process(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::ByteReader& reader, ENetPacketPtr& packet)
{
	return ProcessPacket(reader, [](net::packet::ClientGameStateNAck& clientGameStateNAck, fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, ENetPacketPtr& packet)
	{
		gscomms_execute_callback_on_sync_thread([clientGameStateNAck, client, instance, packet]() mutable 
		{
			auto sgs = instance->GetComponent<fx::ServerGameStatePublic>();
			sgs->HandleGameStateNAck(instance, client, clientGameStateNAck);
			// packet needs to be moved to prevent packet memory from being freed
			(void)packet;
		});
	}, instance, client, packet);
}
