#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include "GameServer.h"
#include "packethandlers/RoutingPacketHandler.h"

#include "TestUtils.h"

class ServerInstance : public fx::ServerInstanceBase
{
	const std::string& GetRootPath() override
	{
		return "test";
	}
};

struct FarLockImpl : fx::FarLock::ImplBase
{
	bool TryLock() override
	{
		return mutex.try_lock();
	}

	void LockShared() override
	{
		return mutex.lock_shared();
	}

	void Lock() override
	{
		return mutex.lock();
	}

	void Unlock() override
	{
		return mutex.unlock();
	}

	void UnlockShared() override
	{
		return mutex.unlock_shared();
	}

	// actual implementation uses folly's shared mutex
	std::shared_mutex mutex;
};

fx::FarLock::FarLock()
	: impl(std::make_unique<FarLockImpl>())
{

}

fx::FarLock::~FarLock()
{
	
}

fx::ClientRegistry::ClientRegistry()
	: m_hostNetId(-1), m_curNetId(1), m_instance(nullptr)
{
	m_clientsBySlotId.resize(MAX_CLIENTS);
}

fx::ClientSharedPtr fx::ClientRegistry::MakeClient(const std::string& guid)
{
	fx::ClientSharedPtr client = fx::ClientSharedPtr::Construct(guid);
	fx::ClientWeakPtr weakClient(client);

	m_clientsBySlotId[1] = weakClient;

	return client;
}

void fx::ClientRegistry::HandleConnectingClient(const fx::ClientSharedPtr& client)
{
}

void fx::ClientRegistry::HandleConnectedClient(const fx::ClientSharedPtr& client, uint32_t oldNetID)
{
}

fx::ClientSharedPtr fx::ClientRegistry::GetHost()
{
	return fx::ClientSharedPtr{};
}

void fx::ClientRegistry::SetHost(const fx::ClientSharedPtr& client)
{
}

void fx::ClientRegistry::AttachToObject(ServerInstanceBase* instance)
{
	m_instance = instance;
}

struct ParseGameStatePacketData
{
	const fx::ClientSharedPtr client;
	const std::vector<uint8_t> packetData;

	ParseGameStatePacketData(const fx::ClientSharedPtr& _client, const std::vector<uint8_t>& _packetData)
		: client(_client),
		  packetData(_packetData)
	{
	}
};

std::optional<ParseGameStatePacketData> g_parseGameStatePacketDataLastCall{};

class GameState : public fx::ServerGameStatePublic
{
	void SendObjectIds(const fx::ClientSharedPtr& client, int numIds) override
	{
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
		g_parseGameStatePacketDataLastCall.emplace(client, packetData);
	}

	void ForAllEntities(const std::function<void(fx::sync::Entity*)>& cb) override
	{
		
	}
};

TEST_CASE("Routing handler test")
{
	fx::SetOneSyncGetCallback([] { return true; });
	REQUIRE(RoutingPacketHandler::GetPacketId() == "msgRoute");
	REQUIRE(HashRageString(RoutingPacketHandler::GetPacketId()) == 0xE938445B);
	// test is only implemented for onesync
	REQUIRE(fx::IsOneSync() == true);
	ServerInstance* serverInstance = new ServerInstance();
	serverInstance->SetComponent(new fx::ClientRegistry());
	serverInstance->SetComponent<fx::ServerGameStatePublic>(new GameState());
	const fx::ClientSharedPtr client = serverInstance->GetComponent<fx::ClientRegistry>()->MakeClient("test");
	net::Buffer buffer;
	buffer.Write<uint16_t>(1); // target net id
	std::vector<uint8_t> data (1200);
	fx::TestUtils::fillVectorU8Random(data);
	buffer.Write<uint16_t>(data.size()); // packet length
	buffer.Write(data.data(), data.size());
	g_parseGameStatePacketDataLastCall.reset();
	buffer.Reset();
	RoutingPacketHandler::Handle(serverInstance, client, buffer);
	REQUIRE(g_parseGameStatePacketDataLastCall.has_value() == true);
	REQUIRE(g_parseGameStatePacketDataLastCall.value().client == client);
	REQUIRE(g_parseGameStatePacketDataLastCall.value().packetData == data);
	delete serverInstance;
}
