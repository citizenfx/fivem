#include <StdInc.h>
#include <GameServerNet.h>

#include <GameServer.h>

#include <NetAddress.h>
#include <NetBuffer.h>

#include <enet/enet.h>

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
		NetPeerImplENet(int handle, ENetPeer* peer)
		{
			m_handle = handle;
			m_peer = peer;
		}

		virtual int GetId() override
		{
			return m_handle;
		}

		virtual int GetPing() override
		{
			return m_peer->roundTripTime;
		}

		virtual int GetPingVariance() override
		{
			return m_peer->roundTripTimeVariance;
		}

		virtual net::PeerAddress GetAddress() override
		{
			return GetPeerAddress(m_peer->address);
		}

		virtual void OnSendConnectOK() override
		{
			// disable peer throttling
			enet_peer_throttle_configure(m_peer, 1000, ENET_PEER_PACKET_THROTTLE_SCALE, 0);

#ifdef _DEBUG
			enet_peer_timeout(m_peer, 86400 * 1000, 86400 * 1000, 86400 * 1000);
#endif
		}

	private:
		int m_handle;
		ENetPeer* m_peer;
	};

	class GameServerNetImplENet : public GameServerNetBase
	{
	public:
		GameServerNetImplENet(GameServer* server)
			: m_server(server), m_basePeerId(1)
		{

		}

		virtual void Process() override
		{
			for (auto& host : this->hosts)
			{
				this->ProcessHost(host.get());
			}
		}

	private:
		void ProcessHost(ENetHost* host)
		{
			ENetEvent event;

			while (enet_host_service(host, &event, 0) > 0)
			{
				switch (event.type)
				{
				case ENET_EVENT_TYPE_CONNECT:
					m_peerHandles.left.insert({ ++m_basePeerId, event.peer });
					break;
				case ENET_EVENT_TYPE_RECEIVE:
				{
					auto peerId = m_peerHandles.right.find(event.peer)->get_left();

					m_server->ProcessPacket(new NetPeerImplENet(peerId, event.peer), event.packet->data, event.packet->dataLength);
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

		virtual fwRefContainer<NetPeerBase> GetPeer(int peerId) override
		{
			return new NetPeerImplENet(peerId, m_peerHandles.left.find(peerId)->get_right());
		}

		virtual void ResetPeer(int peerId) override
		{
			enet_peer_reset(m_peerHandles.left.find(peerId)->get_right());
		}

		virtual void SendPacket(int peer, int channel, const net::Buffer& buffer, NetPacketType type) override
		{
			auto packet = enet_packet_create(buffer.GetBuffer(), buffer.GetCurOffset(), (type == NetPacketType_Reliable) ? ENET_PACKET_FLAG_RELIABLE : (ENetPacketFlag)0);
			enet_peer_send(m_peerHandles.left.find(peer)->get_right(), channel, packet);
		}

		virtual void SendOutOfBand(const net::PeerAddress & to, const std::string_view & oob) override
		{
			auto addr = GetENetAddress(to);

			auto oobMsg = "\xFF\xFF\xFF\xFF" + std::string(oob);

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
			ENetHost* host = enet_host_create(&addr, 64, 2, 0, 0);

			// ensure the host exists
			if (!host)
			{
				trace("Could not bind on %s - is this address valid and not already in use?\n", address.ToString());
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
		using THostPtr = std::unique_ptr<ENetHost, enet_host_deleter>;

		int m_basePeerId;

		std::vector<THostPtr> hosts;

		boost::bimap<int, ENetPeer*> m_peerHandles;

		fwEvent<ENetHost*> OnHostRegistered;

		std::vector<std::function<bool(const uint8_t *, size_t, const net::PeerAddress&)>> m_interceptors;
	};

	fwRefContainer<GameServerNetBase> CreateGSNet_ENet(fx::GameServer* server)
	{
		return new GameServerNetImplENet(server);
	}
}

static InitFunction initFunction([]()
{
	enet_initialize();
});
