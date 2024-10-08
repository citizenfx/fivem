﻿#pragma once

#include <vector>
#include <Client.h>

#include "ArrayUpdate.h"
#include "GameStateNAck.h"
#include "NetGameEventPacket.h"

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

		struct ArrayUpdateData
		{
			const fx::ClientSharedPtr client;
			const uint8_t handler;
			const uint16_t index;
			const std::vector<uint8_t> data;

			ArrayUpdateData(const fx::ClientSharedPtr& client, const uint8_t handler, const uint16_t index, const std::vector<uint8_t>& data)
				: client(client),
				  handler(handler),
				  index(index),
				  data(data)
			{
			}
		};

		struct GameStateNAckData
		{
			const fx::ClientSharedPtr client;
			const uint8_t flags;
			const uint64_t frameIndex;
			const uint64_t firstMissingFrame;
			const uint64_t lastMissingFrame;
			const std::vector<net::packet::ClientGameStateNAck::IgnoreListEntry> ignoreList;
			const std::vector<uint16_t> recreateList;

			GameStateNAckData(const fx::ClientSharedPtr& client, const uint8_t flags, const uint64_t frameIndex, const uint64_t firstMissingFrame, const uint64_t lastMissingFrame, const std::vector<net::packet::ClientGameStateNAck::IgnoreListEntry> ignoreList, const std::vector<uint16_t> recreateList)
				: client(client),
				  flags(flags),
				  frameIndex(frameIndex),
				  firstMissingFrame(firstMissingFrame),
				  lastMissingFrame(lastMissingFrame),
				  ignoreList(std::move(ignoreList)),
				  recreateList(std::move(recreateList))
			{
			}
		};

		struct GameStateAckData
		{
			const fx::ClientSharedPtr client;
			const uint64_t frameIndex;
			const std::vector<net::packet::ClientGameStateNAck::IgnoreListEntry> ignoreList;
			const std::vector<uint16_t> recreateList;

			GameStateAckData(const fx::ClientSharedPtr& client, const uint64_t frameIndex, const std::vector<net::packet::ClientGameStateNAck::IgnoreListEntry> ignoreList, const std::vector<uint16_t> recreateList)
				: client(client),
				  frameIndex(frameIndex),
				  ignoreList(std::move(ignoreList)),
				  recreateList(std::move(recreateList))
			{
			}
		};

		struct SendObjectIdsData
		{
			const ClientSharedPtr client;
			const int numIds;

			SendObjectIdsData(const fx::ClientSharedPtr& _client, const int _numIds)
				: client(_client),
				  numIds(_numIds)
			{
			}
		};

		static ServerGameStatePublic* Create();

		static void SetClientRoutingBucket(const ClientSharedPtr& client, uint32_t routingBucket);

		static std::optional<ParseGameStatePacketData>& GetParseGameStatePacketDataLastCall();

		static std::optional<GameEventHandler>& GetGameEventHandlerLastCall();

		static std::optional<ArrayUpdateData>& GetArrayUpdateLastCall();
		
		static std::optional<GameStateNAckData>& GetGameStateNAckLastCall();

		static std::optional<GameStateAckData>& GetGameStateAckLastCall();

		static std::optional<SendObjectIdsData>& GetFreeObjectIdsLastCall();

		static std::vector<uint16_t>& GetFreeObjectIds();

		static std::function<bool()>& GetGameEventHandler();
	};
}
