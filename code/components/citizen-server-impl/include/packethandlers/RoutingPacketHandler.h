#pragma once

#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <Client.h>
#include <state/ServerGameStatePublic.h>

class RoutingPacketHandler
{
public:
	// check that target and sender is not the same, to prevent small dos
	static void Handle(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::Buffer& packet)
	{
		// TODO: in future net version remove targetNetId when the server is using onesync
		const uint16_t targetNetId = packet.Read<uint16_t>();
		// TODO: in future net version remove packetLength
		packet.Read<uint16_t>();
		const uint32_t remainingBytes = static_cast<uint32_t>(packet.GetRemainingBytes());
		if (!remainingBytes || remainingBytes > UINT16_MAX)
		{
			// invalid packet length
			return;
		}

		// read pointer for now, because client routing does not need the copied buffer
		const uint8_t* packetData = packet.GetRemainingBytesPtr();

		if (fx::IsOneSync())
		{
			std::vector packetDataCopy(packetData, packetData + remainingBytes);
			client->SetHasRouted();

			gscomms_execute_callback_on_sync_thread([instance, client, packetDataCopy = std::move(packetDataCopy)]()
			{
				instance->GetComponent<fx::ServerGameStatePublic>()->ParseGameStatePacket(client, packetDataCopy);
			});

			return;
		}

		if (const auto targetClient = instance->GetComponent<fx::ClientRegistry>()->GetClientByNetID(targetNetId))
		{
			net::Buffer outPacket;
			outPacket.Write(HashRageString("msgRoute"));
			outPacket.Write<uint16_t>(client->GetNetId());
			outPacket.Write(remainingBytes);
			outPacket.Write(packetData, remainingBytes);

			targetClient->SendPacket(1, outPacket, NetPacketType_Unreliable);

			client->SetHasRouted();
		}
	}

	static constexpr const char* GetPacketId()
	{
		return "msgRoute";
	}
};
