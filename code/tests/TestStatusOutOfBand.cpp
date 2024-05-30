#include <type_traits>
#include <StdInc.h>

#include <catch_amalgamated.hpp>
#include <iostream>

#include <outofbandhandlers/GetStatusOutOfBand.h>

#include "ConsoleContextInstance.h"
#include "ServerInstance.h"
#include "TestUtils.h"

struct OutOfBandStatusSendData
{
	net::PeerAddress to;
	std::string oob;
	bool prefix;

	OutOfBandStatusSendData(const net::PeerAddress& to, const std::string& oob, const bool prefix)
		: to(to),
		  oob(oob),
		  prefix(prefix)
	{
	}
};

class TestStatusGameServer : public fwRefCountable
{
public:
	fx::ServerInstanceBase* testServer;
	std::string maxClients;
	std::optional<OutOfBandStatusSendData> outOfBandSendData;

	void SendOutOfBand(const net::PeerAddress& to, const std::string_view& oob, bool prefix = true)
	{
		outOfBandSendData.emplace(to, std::string(oob), prefix);
	}

	~TestStatusGameServer() override
	{
		outOfBandSendData.reset();
	}

	std::string GetVariable(const std::string& key)
	{
		if (key == "sv_maxclients")
		{
			return maxClients;
		}
		REQUIRE(false);
		return fx::TestUtils::asciiRandom(100);
	}

	fx::ServerInstanceBase* GetInstance() const { return testServer; }
};

TEST_CASE("getstatus oob test")
{
	fx::ServerInstanceBase* testServer = ServerInstance::Create();
	TestStatusGameServer* testGameServer = new TestStatusGameServer();
	testGameServer->testServer = testServer;
	fwRefContainer testServerContainer = {testGameServer};
	testServerContainer->GetInstance()->SetComponent(new fx::ClientRegistry());
	testServerContainer->GetInstance()->SetComponent<console::Context>(ConsoleContextInstance::Get());
	testServerContainer->GetInstance()->SetComponent(new fx::PeerAddressRateLimiterStore(console::GetDefaultContext()));

	auto variable = testServer->AddVariable<bool>("sv_returnClientsListInGetStatus", ConVar_None, true);
	variable->GetHelper()->SetValue(GENERATE("false", "true"));
	
	REQUIRE(std::string(GetStatusOutOfBand::GetName()) == "getstatus");

	for (uint32_t i = 0, length = fx::TestUtils::u64Random(10); i < length; ++i)
	{
		testServerContainer->GetInstance()->GetComponent<fx::ClientRegistry>()->MakeClient(std::to_string(i))->SetName(
			fx::TestUtils::asciiRandom(10));
	}

	net::PeerAddress from = net::PeerAddress::FromString("127.0.0.1").get();
	std::string challenge = fx::TestUtils::asciiRandom(50);
	testServerContainer->maxClients = std::to_string(fx::TestUtils::u64Random(100) + 1);

	std::string clientsList = "";
	if (variable->GetValue())
	{
		testServerContainer->GetInstance()->GetComponent<fx::ClientRegistry>()->ForAllClients(
			[&](const fx::ClientSharedPtr& client)
			{
				if (client->GetNetId() < 0xFFFF)
				{
					clientsList += "0 0 \"" + client->GetName() + "\"\n";
				}
			});
	}

	GetStatusOutOfBand getStatusHandler (testServerContainer);
	getStatusHandler.Process<TestStatusGameServer>(testServerContainer, from, challenge);

	const uint32_t connectedClientsCount = testServerContainer->GetInstance()->GetComponent<fx::ClientRegistry>()->
	                                                            GetAmountOfConnectedClients();
	REQUIRE(testServerContainer->outOfBandSendData.has_value() == true);
	REQUIRE(testServerContainer->outOfBandSendData.value().to == from);
	REQUIRE(

		testServerContainer->outOfBandSendData.value().oob == std::string("statusResponse\n\\sv_maxclients\\" +
			testServerContainer->maxClients + "\\clients\\" + std::to_string(connectedClientsCount) + "\n" + clientsList
		));
	REQUIRE(testServerContainer->outOfBandSendData.value().prefix == true);
}
