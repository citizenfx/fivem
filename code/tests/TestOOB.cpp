#include <StdInc.h>

#include <catch_amalgamated.hpp>

#include <GameServer.h>

#include <decorators/WithOutOfBand.h>

#include <EventCore.h>

#include "TestUtils.h"

class TestServer : public fwRefCountable, public fx::ComponentHolderImpl<TestServer>
{
};

class TestGameServer : public fwRefCountable, public fx::ComponentHolderImpl<TestGameServer>
{
public:
	void AddRawInterceptor(const std::function<bool(const uint8_t*, size_t, const net::PeerAddress&)>& interceptor)
	{
		m_interceptor = interceptor;
	}

	void SendOutOfBand(const net::PeerAddress& to, const std::string_view& oob, bool prefix = true)
	{
		// test sending at a later point
	}

	TestServer* GetInstance()
	{
		return &m_instance;
	}

	// only here for testing, that function is not part of the actual game server implementation

	bool TriggerRawInterceptor(const uint8_t* receivedData, size_t receivedDataLength,
	                           const net::PeerAddress& receivedAddress)
	{
		return m_interceptor(receivedData, receivedDataLength, receivedAddress);
	}

private:
	std::function<bool(const uint8_t*, size_t, const net::PeerAddress&)> m_interceptor;
	// in reality this server instance is only stored as a pointer inside the game server
	TestServer m_instance;
};

struct ProcessCallbackData
{
	fwRefContainer<TestGameServer> server;
	net::PeerAddress from;
	std::string_view data;

	ProcessCallbackData(const fwRefContainer<TestGameServer>& server, const net::PeerAddress& from,
	                    const std::string_view& data)
		: server(server),
		  from(from),
		  data(data)
	{
	}
};

struct InterceptCallbackData
{
	net::PeerAddress address;
	const uint8_t* data;
	size_t len;
	bool intercepted;

	InterceptCallbackData(const net::PeerAddress& address, const uint8_t* data, size_t len, bool intercepted)
		: address(address),
		  data(data),
		  len(len),
		  intercepted(intercepted)
	{
	}
};

std::string g_OutOfBandProcessorName{};
std::optional<ProcessCallbackData> g_processCallbackLastCall{};
std::optional<InterceptCallbackData> g_interceptCallbackLastCall{};
bool g_interceptNext{false};

TestGameServer* CreateTestGameServer()
{
	TestGameServer* testGameServer = new TestGameServer();
	testGameServer->GetInstance()->SetComponent(new fx::UdpInterceptor());
	testGameServer->GetInstance()->GetComponent<fx::UdpInterceptor>()->OnIntercept.Connect(
		[](const net::PeerAddress& address, const uint8_t* data, size_t len, bool* intercepted)
		{
			*intercepted = g_interceptNext;
			g_interceptCallbackLastCall.emplace(address, data, len, *intercepted);
			// this return value is never used, only the intercepted value is used 
			return true;
		});
	return testGameServer;
}

struct TestOOB
{
	void Process(const fwRefContainer<TestGameServer>& server, const net::PeerAddress& from,
	             const std::string_view& data) const
	{
		g_processCallbackLastCall.emplace(server, from, data);
	}

	const char* GetName() const
	{
		return g_OutOfBandProcessorName.c_str();
	}
};

TestGameServer* g_testGameServer = CreateTestGameServer();

void RegisterOutOfBand(const std::string& name)
{
	g_OutOfBandProcessorName = name;
	fx::ServerDecorators::WithOutOfBandImpl<TestGameServer, TestOOB>(g_testGameServer);
}

bool SendOutOfBand(const std::string& data, const net::PeerAddress& address)
{
	g_interceptCallbackLastCall.reset();
	g_processCallbackLastCall.reset();
	const bool result = g_testGameServer->TriggerRawInterceptor((uint8_t*)data.c_str(), data.size(), address);
	g_interceptNext = false;
	return result;
}

TEST_CASE("Out of band protocol test")
{
	GIVEN("A out of band handler")
	{
		std::string randomKey = fx::TestUtils::asciiRandom(fx::TestUtils::u64Random(20) + 1);
		RegisterOutOfBand(randomKey);
		WHEN("Sending a out of band message")
		{
			std::string randomData = fx::TestUtils::asciiRandom(fx::TestUtils::u64Random(100) + 1);
			std::string testMsg = randomKey + "\n" + randomData;
			auto oobMsg = "\xFF\xFF\xFF\xFF" + std::string(testMsg);
			auto local = net::PeerAddress::FromString("127.0.0.1").get();
			REQUIRE(SendOutOfBand(oobMsg, local) == true);
			THEN("The intercept callback won't be triggered")
			{
				REQUIRE(g_interceptCallbackLastCall.has_value() == false);
			}
			THEN("The process callback of the registered handler will be triggered")
			{
				REQUIRE(g_processCallbackLastCall.has_value() == true);
				REQUIRE(g_processCallbackLastCall.value().server.GetRef() == g_testGameServer);
				REQUIRE(g_processCallbackLastCall.value().data == randomData);
				REQUIRE(g_processCallbackLastCall.value().from == local);
			}
		}
		WHEN("Sending without key")
		{
			std::string randomData = fx::TestUtils::asciiRandom(fx::TestUtils::u64Random(100) + 1);
			std::string testMsg = "\n" + randomData;
			auto oobMsg = "\xFF\xFF\xFF\xFF" + std::string(testMsg);
			auto local = net::PeerAddress::FromString("127.0.0.1").get();
			REQUIRE(SendOutOfBand(oobMsg, local) == true);
			THEN("The intercept callback won't be triggered, because the message header was -1")
			{
				REQUIRE(g_interceptCallbackLastCall.has_value() == false);
			}
			THEN("The process callback won't be triggered")
			{
				REQUIRE(g_processCallbackLastCall.has_value() == false);
			}
		}
		WHEN("Sending without data")
		{
			std::string testMsg = randomKey + "\n";
			auto oobMsg = "\xFF\xFF\xFF\xFF" + std::string(testMsg);
			auto local = net::PeerAddress::FromString("127.0.0.1").get();
			REQUIRE(SendOutOfBand(oobMsg, local) == true);
			THEN("The intercept callback won't be triggered, because the message header was -1")
			{
				REQUIRE(g_interceptCallbackLastCall.has_value() == false);
			}
			THEN("The process callback will be triggered with empty data")
			{
				REQUIRE(g_processCallbackLastCall.has_value() == true);
				REQUIRE(g_processCallbackLastCall.value().data == "");
				REQUIRE(g_processCallbackLastCall.value().from == local);
			}
		}
		WHEN("Sending without seperator")
		{
			std::string testMsg = randomKey;
			auto oobMsg = "\xFF\xFF\xFF\xFF" + std::string(testMsg);
			auto local = net::PeerAddress::FromString("127.0.0.1").get();
			REQUIRE(SendOutOfBand(oobMsg, local) == true);
			THEN("The intercept callback won't be triggered, because the message header was -1")
			{
				REQUIRE(g_interceptCallbackLastCall.has_value() == false);
			}
			THEN("The process callback will be triggered without data")
			{
				REQUIRE(g_processCallbackLastCall.has_value() == true);
				// last data was matching the randomKey before the fix
				REQUIRE(g_processCallbackLastCall.value().data == "");
				REQUIRE(g_processCallbackLastCall.value().from == local);
			}
		}
		WHEN("Sending with one char data")
		{
			std::string data = fx::TestUtils::asciiRandom(1);
			std::string testMsg = randomKey + "\n" + data;
			auto oobMsg = "\xFF\xFF\xFF\xFF" + std::string(testMsg);
			auto local = net::PeerAddress::FromString("127.0.0.1").get();
			REQUIRE(SendOutOfBand(oobMsg, local) == true);
			THEN("The intercept callback won't be triggered, because the message header was -1")
			{
				REQUIRE(g_interceptCallbackLastCall.has_value() == false);
			}
			THEN("The process callback will be triggered and the data will match")
			{
				REQUIRE(g_processCallbackLastCall.has_value() == true);
				REQUIRE(g_processCallbackLastCall.value().data == data);
				REQUIRE(g_processCallbackLastCall.value().from == local);
			}
		}
		WHEN("Sending without key and data")
		{
			std::string data = fx::TestUtils::asciiRandom(1);
			auto oobMsg = "\xFF\xFF\xFF\xFF";
			auto local = net::PeerAddress::FromString("127.0.0.1").get();
			REQUIRE(SendOutOfBand(oobMsg, local) == true);
			THEN("The intercept callback won't be triggered, because the message header was -1")
			{
				REQUIRE(g_interceptCallbackLastCall.has_value() == false);
			}
			THEN("The process callback won't be called, because the key and data is empty")
			{
				// caused a cpp exception before checking len to be 0
				REQUIRE(g_processCallbackLastCall.has_value() == false);
			}
		}
	}
}

TEST_CASE("udp interceptor test")
{
	GIVEN("A out of band handler")
	{
		std::string randomKey = fx::TestUtils::asciiRandom(fx::TestUtils::u64Random(20) + 1);
		RegisterOutOfBand(randomKey);
		WHEN("Sending a message that does not contain the out of band prefix")
		{
			std::string randomData = fx::TestUtils::asciiRandom(fx::TestUtils::u64Random(100) + 1);
			auto local = net::PeerAddress::FromString("127.0.0.1").get();
			REQUIRE(SendOutOfBand(randomData, local) == false);
			THEN("The intercept callback will be triggered")
			{
				REQUIRE(g_interceptCallbackLastCall.has_value() == true);
				REQUIRE(
					memcmp(g_interceptCallbackLastCall.value().data, reinterpret_cast<uint8_t*>(randomData.data()),
						randomData.size()) == 0);
				REQUIRE(g_interceptCallbackLastCall.value().len == randomData.size());
				REQUIRE(g_interceptCallbackLastCall.value().intercepted == false);
				REQUIRE(g_interceptCallbackLastCall.value().address == local);
			}
			THEN("The process callback won't be triggered, because there was no -1 header")
			{
				REQUIRE(g_processCallbackLastCall.has_value() == false);
			}
		}
		WHEN("Marking the message as intercepted inside the udp interceptor")
		{
			std::string randomData = fx::TestUtils::asciiRandom(fx::TestUtils::u64Random(100) + 1);
			auto local = net::PeerAddress::FromString("127.0.0.1").get();
			g_interceptNext = true;
			REQUIRE(SendOutOfBand(randomData, local) == true);
			THEN("The intercept callback will be triggered")
			{
				REQUIRE(g_interceptCallbackLastCall.has_value() == true);
				REQUIRE(
					memcmp(g_interceptCallbackLastCall.value().data, reinterpret_cast<uint8_t*>(randomData.data()),
						randomData.size()) == 0);
				REQUIRE(g_interceptCallbackLastCall.value().len == randomData.size());
				REQUIRE(g_interceptCallbackLastCall.value().intercepted == true);
				REQUIRE(g_interceptCallbackLastCall.value().address == local);
			}
			THEN("The process callback won't be triggered, because there was no -1 header")
			{
				REQUIRE(g_processCallbackLastCall.has_value() == false);
			}
		}
	}
}
