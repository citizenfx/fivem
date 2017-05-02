#include "StdInc.h"

#include <ServerInstanceBase.h>

#include <NetAddress.h>

#include <enet/enet.h>

template<void* Fn>
struct enet_deleter
{
	template<typename T>
	void operator()(T* data)
	{
		((void(*)(T*))Fn)(data);
	}
};

static ENetAddress GetENetAddress(const net::PeerAddress& peerAddress)
{
	ENetAddress addr = { 0 };

	auto sa = peerAddress.GetSocketAddress();

	if (sa->sa_family == AF_INET)
	{
		auto in4 = (sockaddr_in*)sa;

		addr.host.u.Byte[10] = 0xFF;
		addr.host.u.Byte[11] = 0xFF;

		memcpy(&addr.host.u.Byte[12], &in4->sin_addr.S_un.S_un_b.s_b1, 4);
		addr.port = ntohs(in4->sin_port);
		addr.sin6_scope_id = 0;
	}
	else if (sa->sa_family == AF_INET6)
	{
		auto in6 = (sockaddr_in6*)sa;

		addr.host = in6->sin6_addr;
		addr.port = ntohs(in6->sin6_port);
		addr.sin6_scope_id = in6->sin6_scope_id;
	}

	return addr;
}

static net::PeerAddress GetPeerAddress(const ENetAddress& enetAddress)
{
	char name[128];
	enet_address_get_host_ip(&enetAddress, name, sizeof(name));

	return net::PeerAddress::FromString(fmt::sprintf("[%s]", name), enetAddress.port).get();
}

inline static uint64_t msec()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

namespace fx
{
	class GameServer : public fwRefCountable, public IAttached<ServerInstanceBase>
	{
	public:
		using AddressPair = std::tuple<ENetHost*, net::PeerAddress>;

	public:
		GameServer();

		virtual ~GameServer() override;

		virtual void AttachToObject(ServerInstanceBase* instance) override;

		virtual void SendOutOfBand(const AddressPair& to, const std::string_view& oob);

	private:
		void Run();

		void ProcessServerFrame();

		void ProcessHost(ENetHost* host);

		void Initialize(const boost::property_tree::ptree& pt);

		void ProcessOOB(const AddressPair& from, const std::vector<uint8_t>& data);

		void ProcessGetInfo(const AddressPair& from, const std::string_view& data);

		int Intercept(ENetHost* host);

	private:
		using THostPtr = std::unique_ptr<ENetHost, enet_deleter<&enet_host_destroy>>;

		std::vector<THostPtr> m_hosts;

		std::thread m_thread;

		uint64_t m_residualTime;

		uint64_t m_serverTime;
	};

	static std::map<ENetHost*, GameServer*> g_hostInstances;

	GameServer::GameServer()
		: m_residualTime(0)
	{

	}

	GameServer::~GameServer()
	{
		// TODO: fix if ever multi-instancing
		m_thread.detach();
	}

	void GameServer::AttachToObject(ServerInstanceBase* instance)
	{
		instance->OnReadConfiguration.Connect([=](const boost::property_tree::ptree& pt)
		{
			Initialize(pt);

			m_thread = std::thread([=]()
			{
				Run();
			});
		});
	}

	void GameServer::Run()
	{
		auto lastTime = msec();

		while (true)
		{
			auto now = msec() - lastTime;

			// is it time for a server frame yet?
			m_residualTime += now;

			// service enet with our remaining waits
			ENetSocketSet readfds;
			ENET_SOCKETSET_EMPTY(readfds);

			for (auto& host : m_hosts)
			{
				ENET_SOCKETSET_ADD(readfds, host->socket);
			}

			enet_socketset_select(m_hosts.size(), &readfds, nullptr, 50 - m_residualTime);

			for (auto& host : m_hosts)
			{
				ProcessHost(host.get());
			}

			// 20 FPS = 50msec intervals
			while (m_residualTime > 50)
			{
				m_residualTime -= 50;
				m_serverTime += 50;

				ProcessServerFrame();
			}

			lastTime = msec();
		}
	}

	void GameServer::ProcessHost(ENetHost* host)
	{
		ENetEvent event;

		while (enet_host_service(host, &event, 0) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_RECEIVE:
				trace("got an enet packet\n");
				break;
			}
		}
	}

	void GameServer::ProcessServerFrame()
	{

	}

	void GameServer::Initialize(const boost::property_tree::ptree& pt)
	{
		// for each defined endpoint
		for (auto& child : pt.get_child("server.endpoints"))
		{
			// parse the endpoint to a peer address
			boost::optional<net::PeerAddress> peerAddress = net::PeerAddress::FromString(child.second.get_value<std::string>());

			// if a peer address is set
			if (peerAddress.is_initialized())
			{
				// create an ENet host
				ENetAddress addr = GetENetAddress(*peerAddress);
				ENetHost* host = enet_host_create(&addr, 64, 2, 0, 0);

				// ensure the host exists
				assert(host);

				// set an interceptor callback
				host->intercept = [] (ENetHost* host, ENetEvent* event)
				{
					return g_hostInstances[host]->Intercept(host);
				};

				// register the global host
				g_hostInstances[host] = this;

				m_hosts.push_back(THostPtr{ host });
			}
		}
	}

	void GameServer::ProcessOOB(const AddressPair& from, const std::vector<uint8_t>& data)
	{
		std::string_view dataStr(reinterpret_cast<const char*>(data.data()), data.size());

		// TODO: tokenize command and handle from a list
		if (dataStr.compare(0, 7, "getinfo") == 0)
		{
			ProcessGetInfo(from, dataStr.substr(8));
		}
	}

	void GameServer::ProcessGetInfo(const AddressPair& from, const std::string_view& data)
	{
		// TODO: make proper infostring function
		SendOutOfBand(from, fmt::format(
			"infoResponse\n"
			"\\sv_maxclients\\24\\clients\\0\\challenge\\{0}\\gamename\\CitizenFX\\protocol\\4\\hostname\\empty\\gametype\\\\mapname\\\\iv\\0",
			std::string(data.substr(0, data.find_first_of(" \n")))
		));
	}

	void GameServer::SendOutOfBand(const AddressPair& to, const std::string_view& oob)
	{
		auto addr = GetENetAddress(std::get<net::PeerAddress>(to));

		auto oobMsg = "\xFF\xFF\xFF\xFF" + std::string(oob);

		ENetBuffer buffer;
		buffer.data = (uint8_t*)oobMsg.c_str();
		buffer.dataLength = oobMsg.size();

		enet_socket_send(std::get<ENetHost*>(to)->socket, &addr, &buffer, 1);
	}

	int GameServer::Intercept(ENetHost* host)
	{
		if (host->receivedDataLength >= 4 && *reinterpret_cast<int*>(host->receivedData) == -1)
		{
			auto begin = host->receivedData + 4;
			auto end = begin + (host->receivedDataLength - 4);

			ProcessOOB({ host, GetPeerAddress(host->receivedAddress) }, { begin, end });
			return 1;
		}

		return 0;
	}
}

DECLARE_INSTANCE_TYPE(fx::GameServer);

static InitFunction initFunction([]()
{
	enet_initialize();

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->SetComponent(new fx::GameServer());
	});
});