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
	
	std::optional<InterceptCallbackData> m_interceptCallbackLastCall{};
	bool m_interceptNext{false};
	std::function<bool(const uint8_t*, size_t, const net::PeerAddress&)> m_interceptor;
	TestServer m_instance;

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

namespace
{
	TestGameServer* CreateTestGameServer()
	{
		TestGameServer* testGameServer = new TestGameServer();
		testGameServer->GetInstance()->SetComponent(new fx::UdpInterceptor());
		testGameServer->GetInstance()->GetComponent<fx::UdpInterceptor>()->OnIntercept.Connect(
			[testGameServer](const net::PeerAddress& address, const uint8_t* data, size_t len, bool* intercepted)
			{
				*intercepted = testGameServer->m_interceptNext;
				 testGameServer->m_interceptCallbackLastCall.emplace(address, data, len, *intercepted);
				// this return value is never used, only the intercepted value is used 
				return true;
			});
		return testGameServer;
	}

	TestGameServer* g_testGameServer = CreateTestGameServer();
}

class TestOutOfBand
{
public:
	// the actual out of band handler has a instance created internally inside WithOutOfBandImpl
	// so this one needs to be static
	static inline std::optional<ProcessCallbackData> processCallbackLastCall{};
	
	template<typename ServerImpl>
	TestOutOfBand(const fwRefContainer<ServerImpl>& server)
	{
	}
	
	TestOutOfBand()
	{
	}
	
	template<typename ServerImpl>
	void Process(const fwRefContainer<ServerImpl>& server, const net::PeerAddress& from,
	             const std::string_view& data)
	{
		processCallbackLastCall.emplace(server, from, data);
	}

	static constexpr const char* GetName()
	{
		return "test";
	}

	void RegisterOutOfBand()
	{
		fx::ServerDecorators::WithOutOfBandImpl<TestGameServer, TestOutOfBand>(g_testGameServer);
	}

	bool SendOutOfBand(const std::string& data, const net::PeerAddress& address)
	{
		g_testGameServer->m_interceptCallbackLastCall.reset();
		processCallbackLastCall.reset();
		const bool result = g_testGameServer->TriggerRawInterceptor(reinterpret_cast<const uint8_t*>(data.c_str()), data.size(), address);
		g_testGameServer->m_interceptNext = false;
		return result;
	}
};

TEST_CASE("Out of band protocol test")
{
	TestOutOfBand testOutOfBand;
	GIVEN("A out of band handler")
	{
		testOutOfBand.RegisterOutOfBand();
		WHEN("Sending a out of band message")
		{
			std::string randomData = fx::TestUtils::asciiRandom(fx::TestUtils::u64Random(100) + 1);
			std::string testMsg = std::string("test") + "\n" + randomData;
			auto oobMsg = "\xFF\xFF\xFF\xFF" + std::string(testMsg);
			auto local = net::PeerAddress::FromString("127.0.0.1").get();
			REQUIRE(testOutOfBand.SendOutOfBand(oobMsg, local) == true);
			THEN("The intercept callback won't be triggered")
			{
				REQUIRE(g_testGameServer->m_interceptCallbackLastCall.has_value() == false);
			}
			THEN("The process callback of the registered handler will be triggered")
			{
				REQUIRE(testOutOfBand.processCallbackLastCall.has_value() == true);
				REQUIRE(testOutOfBand.processCallbackLastCall.value().server.GetRef() == g_testGameServer);
				REQUIRE(testOutOfBand.processCallbackLastCall.value().data == randomData);
				REQUIRE(testOutOfBand.processCallbackLastCall.value().from == local);
			}
		}
		WHEN("Sending without key")
		{
			std::string randomData = fx::TestUtils::asciiRandom(fx::TestUtils::u64Random(100) + 1);
			std::string testMsg = "\n" + randomData;
			auto oobMsg = "\xFF\xFF\xFF\xFF" + std::string(testMsg);
			auto local = net::PeerAddress::FromString("127.0.0.1").get();
			REQUIRE(testOutOfBand.SendOutOfBand(oobMsg, local) == true);
			THEN("The intercept callback won't be triggered, because the message header was -1")
			{
				REQUIRE(g_testGameServer->m_interceptCallbackLastCall.has_value() == false);
			}
			THEN("The process callback won't be triggered")
			{
				REQUIRE(testOutOfBand.processCallbackLastCall.has_value() == false);
			}
		}
		WHEN("Sending without data")
		{
			std::string testMsg = std::string("test") + "\n";
			auto oobMsg = "\xFF\xFF\xFF\xFF" + std::string(testMsg);
			auto local = net::PeerAddress::FromString("127.0.0.1").get();
			REQUIRE(testOutOfBand.SendOutOfBand(oobMsg, local) == true);
			THEN("The intercept callback won't be triggered, because the message header was -1")
			{
				REQUIRE(g_testGameServer->m_interceptCallbackLastCall.has_value() == false);
			}
			THEN("The process callback will be triggered with empty data")
			{
				REQUIRE(testOutOfBand.processCallbackLastCall.has_value() == true);
				REQUIRE(testOutOfBand.processCallbackLastCall.value().data == "");
				REQUIRE(testOutOfBand.processCallbackLastCall.value().from == local);
			}
		}
		WHEN("Sending without seperator")
		{
			std::string testMsg = "test";
			auto oobMsg = "\xFF\xFF\xFF\xFF" + std::string(testMsg);
			auto local = net::PeerAddress::FromString("127.0.0.1").get();
			REQUIRE(testOutOfBand.SendOutOfBand(oobMsg, local) == true);
			THEN("The intercept callback won't be triggered, because the message header was -1")
			{
				REQUIRE(g_testGameServer->m_interceptCallbackLastCall.has_value() == false);
			}
			THEN("The process callback will be triggered without data")
			{
				REQUIRE(testOutOfBand.processCallbackLastCall.has_value() == true);
				// last data was matching the randomKey before the fix
				REQUIRE(testOutOfBand.processCallbackLastCall.value().data == "");
				REQUIRE(testOutOfBand.processCallbackLastCall.value().from == local);
			}
		}
		WHEN("Sending with one char data")
		{
			std::string data = fx::TestUtils::asciiRandom(1);
			std::string testMsg = std::string("test") + "\n" + data;
			auto oobMsg = "\xFF\xFF\xFF\xFF" + std::string(testMsg);
			auto local = net::PeerAddress::FromString("127.0.0.1").get();
			REQUIRE(testOutOfBand.SendOutOfBand(oobMsg, local) == true);
			THEN("The intercept callback won't be triggered, because the message header was -1")
			{
				REQUIRE(g_testGameServer->m_interceptCallbackLastCall.has_value() == false);
			}
			THEN("The process callback will be triggered and the data will match")
			{
				REQUIRE(testOutOfBand.processCallbackLastCall.has_value() == true);
				REQUIRE(testOutOfBand.processCallbackLastCall.value().data == data);
				REQUIRE(testOutOfBand.processCallbackLastCall.value().from == local);
			}
		}
		WHEN("Sending without key and data")
		{
			std::string data = fx::TestUtils::asciiRandom(1);
			auto oobMsg = "\xFF\xFF\xFF\xFF";
			auto local = net::PeerAddress::FromString("127.0.0.1").get();
			REQUIRE(testOutOfBand.SendOutOfBand(oobMsg, local) == true);
			THEN("The intercept callback won't be triggered, because the message header was -1")
			{
				REQUIRE(g_testGameServer->m_interceptCallbackLastCall.has_value() == false);
			}
			THEN("The process callback won't be called, because the key and data is empty")
			{
				// caused a cpp exception before checking len to be 0
				REQUIRE(testOutOfBand.processCallbackLastCall.has_value() == false);
			}
		}
	}
}

TEST_CASE("udp interceptor test")
{
	TestOutOfBand testOutOfBand;
	GIVEN("A out of band handler")
	{
		testOutOfBand.RegisterOutOfBand();
		WHEN("Sending a message that does not contain the out of band prefix")
		{
			std::string randomData = fx::TestUtils::asciiRandom(fx::TestUtils::u64Random(100) + 1);
			auto local = net::PeerAddress::FromString("127.0.0.1").get();
			REQUIRE(testOutOfBand.SendOutOfBand(randomData, local) == false);
			THEN("The intercept callback will be triggered")
			{
				REQUIRE(g_testGameServer->m_interceptCallbackLastCall.has_value() == true);
				REQUIRE(
					memcmp(g_testGameServer->m_interceptCallbackLastCall.value().data, reinterpret_cast<uint8_t*>(randomData.data()),
						randomData.size()) == 0);
				REQUIRE(g_testGameServer->m_interceptCallbackLastCall.value().len == randomData.size());
				REQUIRE(g_testGameServer->m_interceptCallbackLastCall.value().intercepted == false);
				REQUIRE(g_testGameServer->m_interceptCallbackLastCall.value().address == local);
			}
			THEN("The process callback won't be triggered, because there was no -1 header")
			{
				REQUIRE(testOutOfBand.processCallbackLastCall.has_value() == false);
			}
		}
		WHEN("Marking the message as intercepted inside the udp interceptor")
		{
			std::string randomData = fx::TestUtils::asciiRandom(fx::TestUtils::u64Random(100) + 1);
			auto local = net::PeerAddress::FromString("127.0.0.1").get();
			g_testGameServer->m_interceptNext = true;
			REQUIRE(testOutOfBand.SendOutOfBand(randomData, local) == true);
			THEN("The intercept callback will be triggered")
			{
				REQUIRE(g_testGameServer->m_interceptCallbackLastCall.has_value() == true);
				REQUIRE(
					memcmp(g_testGameServer->m_interceptCallbackLastCall.value().data, reinterpret_cast<uint8_t*>(randomData.data()),
						randomData.size()) == 0);
				REQUIRE(g_testGameServer->m_interceptCallbackLastCall.value().len == randomData.size());
				REQUIRE(g_testGameServer->m_interceptCallbackLastCall.value().intercepted == true);
				REQUIRE(g_testGameServer->m_interceptCallbackLastCall.value().address == local);
			}
			THEN("The process callback won't be triggered, because there was no -1 header")
			{
				REQUIRE(testOutOfBand.processCallbackLastCall.has_value() == false);
			}
		}
	}
}
