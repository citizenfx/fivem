#include <type_traits>
#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include <outofbandhandlers/GetInfoOutOfBand.h>

#include "TestUtils.h"

struct FarLockImpl : fx::FarLock::ImplBase
{
	// actual implementation uses folly's shared mutex
	std::shared_mutex mutex;

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

	++m_amountConnectedClients;

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

struct OutOfBandSendData
{
	const net::PeerAddress to;
	const std::string oob;
	const bool prefix;

	OutOfBandSendData(const net::PeerAddress& to, const std::string_view& oob, const bool prefix)
		: to(to),
		  oob(oob),
		  prefix(prefix)
	{
	}
};

class TestServer : public fx::ServerInstanceBase
{
	const std::string& GetRootPath() override { return "test2"; }
};

class TestGameServer : public fwRefCountable
{
public:
	TestServer testServer;
	std::string maxClients;
	std::string hostname;
	std::string gametype;
	std::string mapname;
	std::string iv;
	std::optional<OutOfBandSendData> outOfBandSendData;

	void SendOutOfBand(const net::PeerAddress& to, const std::string_view& oob, bool prefix = true)
	{
		outOfBandSendData.emplace(to, oob, prefix);
	}

	std::string GetVariable(const std::string& key)
	{
		if (key == "sv_maxclients")
		{
			return maxClients;
		}
		if (key == "sv_hostname")
		{
			return hostname;
		}
		if (key == "gametype")
		{
			return gametype;
		}
		if (key == "mapname")
		{
			return mapname;
		}
		if (key == "sv_infoVersion")
		{
			return iv;
		}
		REQUIRE(false);
		return fx::TestUtils::asciiRandom(100);
	}

	fx::ServerInstanceBase* GetInstance() { return &testServer; }
};

TEST_CASE("getinfo oob test")
{
	fwRefContainer testServerContainer = {new TestGameServer()};
	testServerContainer->GetInstance()->SetComponent(new fx::ClientRegistry());
	testServerContainer->GetInstance()->SetComponent(new fx::PeerAddressRateLimiterStore(console::GetDefaultContext()));

	REQUIRE(GetInfoOutOfBand::GetName() == "getinfo");

	net::PeerAddress from = net::PeerAddress::FromString("127.0.0.1").get();
	std::string challenge = fx::TestUtils::asciiRandom(50);
	testServerContainer->maxClients = std::to_string(fx::TestUtils::u64Random(100) + 1);
	testServerContainer->hostname = fx::TestUtils::asciiRandom(10);
	testServerContainer->gametype = fx::TestUtils::asciiRandom(10);
	testServerContainer->mapname = fx::TestUtils::asciiRandom(10);
	testServerContainer->iv = fx::TestUtils::asciiRandom(10);

	GetInfoOutOfBand::Process<TestGameServer>(testServerContainer, from, challenge);

	const uint32_t connectedClientsCount = testServerContainer->GetInstance()->GetComponent<fx::ClientRegistry>()->
	                                                            GetAmountOfConnectedClients();

	REQUIRE(testServerContainer->outOfBandSendData.has_value() == true);
	REQUIRE(testServerContainer->outOfBandSendData.value().to == from);
	REQUIRE(
		testServerContainer->outOfBandSendData.value().oob == std::string("infoResponse\n\\sv_maxclients\\") +
		testServerContainer->maxClients +
		"\\clients\\" + std::to_string(connectedClientsCount) + "\\challenge\\" + challenge + "\\gamename\\" +
		"CitizenFX" + "\\protocol\\4\\hostname\\" + testServerContainer->hostname + "\\gametype\\" + testServerContainer
		->gametype +
		"\\mapname\\" + testServerContainer->mapname + "\\iv\\" + testServerContainer->iv);
	REQUIRE(testServerContainer->outOfBandSendData.value().prefix == true);
}
