#pragma once

#include <vector>
#include <Client.h>

#include "NetGameEventV2.h"

namespace fx
{
	class ServerGameStatePublic;

	class ServerGameStatePublicInstance
	{
	public:
		struct ParseGameStatePacketData
		{
			const ClientSharedPtr client;
			const std::vector<uint8_t> packetData;

			ParseGameStatePacketData(const fx::ClientSharedPtr& _client, const std::vector<uint8_t>& _packetData)
				: client(_client),
				  packetData(_packetData)
			{
			}
		};

		struct GameEventHandler
		{
			const ClientSharedPtr client;
			const std::vector<uint16_t> targetPlayers;
			const uint32_t eventNameHash;
			const std::vector<uint8_t> data;
			const uint16_t eventId;
			const bool isReply;

			GameEventHandler(const fx::ClientSharedPtr& _client, const std::vector<uint16_t>& _targetPlayers,
			                 net::packet::ClientNetGameEventV2& _netGameEvent)
				: client(_client),
				  targetPlayers(_targetPlayers),
				  eventNameHash(_netGameEvent.eventNameHash),
				  data(_netGameEvent.data.GetValue().begin(), _netGameEvent.data.GetValue().end()),
				  eventId(_netGameEvent.eventId),
				  isReply(_netGameEvent.isReply)
			{
			}
		};

		static ServerGameStatePublic* Create();

		static void SetClientRoutingBucket(const ClientSharedPtr& client, uint32_t routingBucket);

		static std::optional<ParseGameStatePacketData>& GetParseGameStatePacketDataLastCall();

		static std::optional<GameEventHandler>& GetGameEventHandlerLastCall();

		static std::function<bool()>& GetGameEventHandler();
	};
}
