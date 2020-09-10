#include <StdInc.h>
#include <GameServerNet.h>

#include <GameServer.h>

#include <CoreConsole.h>

#include <NetAddress.h>
#include <NetBuffer.h>

#include <ClientRegistry.h>
#include <ServerInstanceBase.h>

#include <Error.h>
#include <StructuredTrace.h>

#include <enet/enet.h>

#include <FixedBuffer.h>

namespace fx
{
	struct enet_host_deleter
	{
		inline void operator()(ENetHost* data)
		{
			enet_host_destroy(data);
		}
	};

	ENetAddress GetENetAddress(const net::PeerAddress& peerAddress)
	{
		ENetAddress addr = { 0 };

		auto sa = peerAddress.GetSocketAddress();

		if (sa->sa_family == AF_INET)
		{
			auto in4 = (sockaddr_in*)sa;

			addr.host.s6_addr[10] = 0xFF;
			addr.host.s6_addr[11] = 0xFF;

			memcpy(&addr.host.s6_addr[12], &in4->sin_addr.s_addr, 4);
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

	net::PeerAddress GetPeerAddress(const ENetAddress& enetAddress)
	{
		sockaddr_in6 in6 = { 0 };
		in6.sin6_family = AF_INET6;
		in6.sin6_addr = enetAddress.host;
		in6.sin6_port = htons(enetAddress.port);
		in6.sin6_scope_id = enetAddress.sin6_scope_id;

		return net::PeerAddress((sockaddr*)&in6, sizeof(in6));
	}

	class GameServerNetImplENet;

	static std::map<ENetHost*, GameServerNetImplENet*> g_hostInstances;

	class NetPeerImplENet : public NetPeerBase
	{
	public:
		NetPeerImplENet(GameServerNetImplENet* host, int handle)
		{
			m_handle = handle;
			m_host = host;
		}

	private:
		ENetPeer* GetPeer();

	public:
		virtual int GetId() override
		{
			return m_handle;
		}

		virtual int GetPing() override
		{
			auto peer = GetPeer();

			if (!peer)
			{
				return -1;
			}

			return peer->roundTripTime;
		}

		virtual int GetPingVariance() override
		{
			auto peer = GetPeer();

			if (!peer)
			{
				return 0;
			}

			return peer->roundTripTimeVariance;
		}

		virtual net::PeerAddress GetAddress() override
		{
			auto peer = GetPeer();

			if (!peer)
			{
				return *net::PeerAddress::FromString("127.0.0.1", 30120, net::PeerAddress::LookupType::NoResolution);
			}

			return GetPeerAddress(peer->address);
		}

		virtual void OnSendConnectOK() override
		{
			auto peer = GetPeer();

			if (!peer)
			{
				return;
			}

			// disable peer throttling
			enet_peer_throttle_configure(peer, 1000, ENET_PEER_PACKET_THROTTLE_SCALE, 0);

#ifdef _DEBUG
			//enet_peer_timeout(peer, 86400 * 1000, 86400 * 1000, 86400 * 1000);
#endif

			enet_peer_timeout(peer, ENET_PEER_TIMEOUT_LIMIT, ENET_PEER_TIMEOUT_MINIMUM, 10000);
		}

	private:
		int m_handle;
		GameServerNetImplENet* m_host;
	};

	class GameServerNetImplENet : public GameServerNetBase
	{
	public:
		GameServerNetImplENet(GameServer* server)
			: m_server(server), m_basePeerId(1)
		{
			static ConsoleCommand cmd("force_enet_disconnect", [this](int peerIdx)
			{
				auto client = m_server->GetInstance()->GetComponent<fx::ClientRegistry>()->GetClientByNetID(peerIdx);

				if (!client)
				{
					return;
				}

				peerIdx = client->GetPeer();

				auto peer = m_peerHandles.find(peerIdx);

				if (peer != m_peerHandles.end())
				{
					enet_peer_disconnect(peer->second, 0);
				}
			});
		}

		virtual void Process() override
		{
			for (auto& host : this->hosts)
			{
				this->ProcessHost(host.get());
			}
		}

		virtual bool SupportsUvUdp() override
		{
			return true;
		}

	private:
		void ProcessHost(ENetHost* host)
		{
			ENetEvent event;
			int enetEntry = 0;

			while ((enetEntry = enet_host_service(host, &event, 0)) != 0)
			{
				switch (event.type)
				{
				case ENET_EVENT_TYPE_CONNECT:
				{
					console::DPrintf("enet", "Peer %s connected to ENet (id %d).\n", GetPeerAddress(event.peer->address).ToString(), m_basePeerId + 1);

					auto peerId = ++m_basePeerId;
					event.peer->data = reinterpret_cast<void*>(peerId);
					m_peerHandles.emplace(peerId, event.peer);
					break;
				}
				case ENET_EVENT_TYPE_DISCONNECT:
				{
					console::DPrintf("enet", "Peer %s disconnected from ENet.\n", GetPeerAddress(event.peer->address).ToString());

					m_peerHandles.erase(static_cast<int>(reinterpret_cast<uintptr_t>(event.peer->data)));
					break;
				}
				case ENET_EVENT_TYPE_RECEIVE:
				{
					auto peerId = static_cast<int>(reinterpret_cast<uintptr_t>(event.peer->data));

					NetPeerImplENet netPeer(this, peerId);
					m_server->ProcessPacket(&netPeer, event.packet->data, event.packet->dataLength);
					enet_packet_destroy(event.packet);
					break;
				}
				}
			}
		}

	public:

		virtual void Select(const std::vector<uintptr_t>& addFds, int timeout) override
		{
			std::vector<ENetSocket> fds;

			for (auto& host : this->hosts)
			{
				fds.push_back(host->socket);
			}

			for (auto& fd : addFds)
			{
				fds.push_back(static_cast<ENetSocket>(fd));
			}

			ENetSocketSet readfds;
			ENET_SOCKETSET_EMPTY(readfds);

			for (auto fd : fds)
			{
				ENET_SOCKETSET_ADD(readfds, fd);
			}

			int nfds = 0;

#ifndef _WIN32
			nfds = *std::max_element(fds.begin(), fds.end());
#endif

			enet_socketset_select(nfds, &readfds, nullptr, timeout);
		}

		virtual void GetPeer(int peerId, NetPeerStackBuffer& stackBuffer) override
		{
			stackBuffer.Construct<NetPeerImplENet>(this, peerId);
		}

		virtual void ResetPeer(int peerId) override
		{
			auto peerPair = m_peerHandles.find(peerId);

			if (peerPair == m_peerHandles.end())
			{
				return;
			}

			enet_peer_reset(peerPair->second);
		}

		virtual void SendPacket(int peer, int channel, const net::Buffer& buffer, NetPacketType type) override
		{
			auto peerPair = m_peerHandles.find(peer);
			if (peerPair == m_peerHandles.end())
			{
				return;
			}

			// fewer allocations!!
			auto flags = ENET_PACKET_FLAG_NO_ALLOCATE | ((type == NetPacketType_Reliable || type == NetPacketType_ReliableReplayed) ? ENET_PACKET_FLAG_RELIABLE : (ENetPacketFlag)0);
			auto packet = enet_packet_create(buffer.GetBuffer(), buffer.GetCurOffset(), flags);

			using NetBufferSharedPtr = std::shared_ptr<std::vector<uint8_t>>;

			static object_pool<NetBufferSharedPtr> sharedPtrPool;
			packet->userData = sharedPtrPool.construct(buffer.GetBytes());
			packet->freeCallback = [](ENetPacket* packet)
			{
				sharedPtrPool.destruct((NetBufferSharedPtr*)packet->userData);
			};
			enet_peer_send(peerPair->second, channel, packet);
		}

		virtual void SendOutOfBand(const net::PeerAddress & to, const std::string_view & oob, bool prefix) override
		{
			auto addr = GetENetAddress(to);

			auto oobMsg = (prefix ? "\xFF\xFF\xFF\xFF" : "") + std::string(oob);

			ENetBuffer buffer;
			buffer.data = (uint8_t*)oobMsg.c_str();
			buffer.dataLength = oobMsg.size();

			for (auto& host : this->hosts)
			{
				enet_socket_send(host->socket, &addr, &buffer, 1);
			}
		}

	private:
		int OnIntercept(ENetHost* host)
		{
			for (const auto& interceptor : m_interceptors)
			{
				if (interceptor(host->receivedData, host->receivedDataLength, GetPeerAddress(host->receivedAddress)))
				{
					return 1;
				}
			}

			return 0;
		}

	public:
		virtual void CreateUdpHost(const net::PeerAddress& address) override
		{
			// create an ENet host
			ENetAddress addr = GetENetAddress(address);
			ENetHost* host = enet_host_create(&addr, 1024, 2, 0, 0);

			// ensure the host exists
			if (!host)
			{
				StructuredTrace({ "type", "bind_error" }, { "type", "enet" }, { "address", address.ToString() });
				FatalError("Could not bind on %s - is this address valid and not already in use?\n", address.ToString());
				return;
			}

			// register the global host
			g_hostInstances[host] = this;

			host->intercept = [](ENetHost* host, ENetEvent* event)
			{
				return g_hostInstances[host]->OnIntercept(host);
			};

			this->hosts.push_back(THostPtr{ host });

			this->OnHostRegistered(host);
		}

		virtual void AddRawInterceptor(const std::function<bool(const uint8_t *, size_t, const net::PeerAddress&)>& interceptor) override
		{
			m_interceptors.push_back(interceptor);
		}

		virtual int GetClientVersion() override
		{
			return 2;
		}

	private:
		GameServer* m_server;

	public:
		friend class NetPeerImplENet;

		using THostPtr = std::unique_ptr<ENetHost, enet_host_deleter>;

		int m_basePeerId;

		std::vector<THostPtr> hosts;

		std::map<int, ENetPeer*> m_peerHandles;

		fwEvent<ENetHost*> OnHostRegistered;

		std::vector<std::function<bool(const uint8_t *, size_t, const net::PeerAddress&)>> m_interceptors;
	};

	ENetPeer* NetPeerImplENet::GetPeer()
	{
		auto it = m_host->m_peerHandles.find(m_handle);

		if (it == m_host->m_peerHandles.end())
		{
			return nullptr;
		}

		return it->second;
	}

	fwRefContainer<GameServerNetBase> CreateGSNet_ENet(fx::GameServer* server)
	{
		return new GameServerNetImplENet(server);
	}
}

static InitFunction initFunction([]()
{
	enet_initialize();
});
