#include "StdInc.h"

#include "ServerGameStatePublicInstance.h"

#include <NetBuffer.h>

#include <Client.h>
#include <GameServer.h>

#include <state/ServerGameStatePublic.h>

#include "GameStateNAck.h"
#include "GameStateAck.h"

class ServerGameStatePublicImpl : public fx::ServerGameStatePublic
{
public:
	static inline std::optional<fx::ServerGameStatePublicInstance::ParseGameStatePacketData> parseGameStatePacketDataLastCall{};

	static inline std::optional<fx::ServerGameStatePublicInstance::GameEventHandler> gameEventHandlerLastCall{};

	static inline std::optional<fx::ServerGameStatePublicInstance::ArrayUpdateData> arrayUpdateLastCall{};

	static inline std::optional<fx::ServerGameStatePublicInstance::GameStateNAckData> gameStateNAckLastCall{};

	static inline std::optional<fx::ServerGameStatePublicInstance::GameStateAckData> gameStateAckLastCall{};

	static inline std::function<bool()> gameEventHandler;

	static inline std::optional<fx::ServerGameStatePublicInstance::SendObjectIdsData> getFreeObjectIdsLastCall{};

	static inline std::vector<uint16_t> freeObjectIds{};

	ServerGameStatePublicImpl() = default;

	virtual ~ServerGameStatePublicImpl() = default;

	void GetFreeObjectIds(const fx::ClientSharedPtr& client, uint8_t numIds, std::vector<uint16_t>& freeIds) override
	{
		getFreeObjectIdsLastCall.emplace(client, numIds);
		freeIds = freeObjectIds;
	}

	fx::SyncStyle GetSyncStyle() override
	{
		return fx::SyncStyle::NAK;
	}

	fx::EntityLockdownMode GetEntityLockdownMode(const fx::ClientSharedPtr& client) override
	{
		return fx::EntityLockdownMode::Strict;
	}

	void ParseGameStatePacket(const fx::ClientSharedPtr& client, const std::vector<uint8_t>& packetData) override
	{
		parseGameStatePacketDataLastCall.emplace(client, packetData);
	}

	void ForAllEntities(const std::function<void(fx::sync::Entity*)>& cb) override
	{
	}

	bool SetEntityStateBag(uint8_t playerId, uint16_t objectId, std::function<std::shared_ptr<fx::StateBag>()> createStateBag) override
	{
		return false;
	}

	uint32_t GetClientRoutingBucket(const fx::ClientSharedPtr& client) override
	{
		return fx::AnyCast<uint32_t>(client->GetData("routingBucket"));
	}

	std::function<bool()> GetGameEventHandlerWithEvent(const fx::ClientSharedPtr& client, const std::vector<uint16_t>& targetPlayers, net::packet::ClientNetGameEventV2& netGameEvent) override
	{
		gameEventHandlerLastCall.emplace(client, targetPlayers, netGameEvent);
		return gameEventHandler;
	}

	bool IsClientRelevantEntity(const fx::ClientSharedPtr& client, uint32_t objectId) override
	{
		// todo: add test for this
		return true;
	}

	void HandleArrayUpdate(const fx::ClientSharedPtr& client, net::packet::ClientArrayUpdate& buffer) override
	{
		arrayUpdateLastCall.emplace(client, buffer.handler.GetValue(), buffer.index.GetValue(), std::vector<uint8_t>{ buffer.data.GetValue().begin(), buffer.data.GetValue().end() });
	}

	void HandleGameStateNAck(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::packet::ClientGameStateNAck& buffer) override
	{
		gameStateNAckLastCall.emplace(client, buffer.GetFlags(), buffer.GetFrameIndex(), buffer.GetFirstMissingFrame(), buffer.GetLastMissingFrame(), std::vector<net::packet::ClientGameStateNAck::IgnoreListEntry>{ buffer.GetIgnoreList().begin(), buffer.GetIgnoreList().end() }, std::vector<uint16_t>{ buffer.GetRecreateList().begin(), buffer.GetRecreateList().end() });
	}

	void HandleGameStateAck(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::packet::ClientGameStateAck& buffer) override
	{
		gameStateAckLastCall.emplace(client, buffer.GetFrameIndex(), std::vector<net::packet::ClientGameStateNAck::IgnoreListEntry>{ buffer.GetIgnoreList().begin(), buffer.GetIgnoreList().end() }, std::vector<uint16_t>{ buffer.GetRecreateList().begin(), buffer.GetRecreateList().end() });
	}
};

fx::ServerGameStatePublic* fx::ServerGameStatePublicInstance::Create()
{
	return new ServerGameStatePublicImpl();
}

void fx::ServerGameStatePublicInstance::SetClientRoutingBucket(const ClientSharedPtr& client, const uint32_t routingBucket)
{
	client->SetData("routingBucket", routingBucket);
}

std::optional<fx::ServerGameStatePublicInstance::ParseGameStatePacketData>& fx::ServerGameStatePublicInstance::GetParseGameStatePacketDataLastCall()
{
	return ServerGameStatePublicImpl::parseGameStatePacketDataLastCall;
}

std::optional<fx::ServerGameStatePublicInstance::GameEventHandler>& fx::ServerGameStatePublicInstance::GetGameEventHandlerLastCall()
{
	return ServerGameStatePublicImpl::gameEventHandlerLastCall;
}

std::optional<fx::ServerGameStatePublicInstance::SendObjectIdsData>& fx::ServerGameStatePublicInstance::GetFreeObjectIdsLastCall()
{
	return ServerGameStatePublicImpl::getFreeObjectIdsLastCall;
}

std::vector<uint16_t>& fx::ServerGameStatePublicInstance::GetFreeObjectIds()
{
	return ServerGameStatePublicImpl::freeObjectIds;
}

std::optional<fx::ServerGameStatePublicInstance::ArrayUpdateData>& fx::ServerGameStatePublicInstance::GetArrayUpdateLastCall()
{
	return ServerGameStatePublicImpl::arrayUpdateLastCall;
}

std::optional<fx::ServerGameStatePublicInstance::GameStateNAckData>& fx::ServerGameStatePublicInstance::GetGameStateNAckLastCall()
{
	return ServerGameStatePublicImpl::gameStateNAckLastCall;
}

std::optional<fx::ServerGameStatePublicInstance::GameStateAckData>& fx::ServerGameStatePublicInstance::GetGameStateAckLastCall()
{
	return ServerGameStatePublicImpl::gameStateAckLastCall;
}

std::function<bool()>& fx::ServerGameStatePublicInstance::GetGameEventHandler()
{
	return ServerGameStatePublicImpl::gameEventHandler;
}
