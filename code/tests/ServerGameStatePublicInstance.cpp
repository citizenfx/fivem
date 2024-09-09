#include "StdInc.h"

#include "ServerGameStatePublicInstance.h"

#include <NetBuffer.h>

#include <Client.h>
#include <GameServer.h>

#include <state/ServerGameStatePublic.h>

class ServerGameStatePublicImpl : public fx::ServerGameStatePublic
{
public:
	static inline std::optional<fx::ServerGameStatePublicInstance::ParseGameStatePacketData> parseGameStatePacketDataLastCall{};

	static inline std::optional<fx::ServerGameStatePublicInstance::GameEventHandler> gameEventHandlerLastCall{};

	static inline std::optional<fx::ServerGameStatePublicInstance::ArrayUpdateData> arrayUpdateLastCall{};

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

std::function<bool()>& fx::ServerGameStatePublicInstance::GetGameEventHandler()
{
	return ServerGameStatePublicImpl::gameEventHandler;
}
