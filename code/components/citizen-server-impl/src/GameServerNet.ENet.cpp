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
#include <net/PacketNames.h>

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

			// all-but-disable the backoff-based timeout, and set the hard timeout to 30 seconds
			enet_peer_timeout(peer, 10000000, 10000000, 30000);
		}

	private:
		int m_handle;
		GameServerNetImplENet* m_host;
	};

	class GameServerNetImplENet : public GameServerNetBase
	{
		struct ConnectionUsage
		{
			in6_addr address;
			size_t count = 0;
		};

	public:
		GameServerNetImplENet(GameServer* server)
			: m_server(server), m_basePeerId(1)
		{
			m_clientRegistry = m_server->GetInstance()->GetComponent<fx::ClientRegistry>();

			static ConsoleCommand cmd("force_enet_disconnect", [this](int peerIdx)
			{
				auto client = m_clientRegistry->GetClientByNetID(peerIdx);

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
		void OnDisconnect(ENetPeer* peer)
		{
			console::DPrintf("enet", "Peer %s disconnected from ENet.\n", GetPeerAddress(peer->address).ToString());

			auto peerId = static_cast<int>(reinterpret_cast<uintptr_t>(peer->data));
			m_peerHandles.erase(peerId);

			// reset connection data (in case of a reconnect from the same client)
			if (auto it = m_peerData.find(peerId); it != m_peerData.end())
			{
				m_connectionUsage.erase(it->second);
				m_peerData.erase(it);
			}
		}

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
					m_peerData.emplace(peerId, event.data);

					auto& usage = m_connectionUsage[event.data];
					usage.count = 9999; // set high enough to invalidate down below
					break;
				}
				case ENET_EVENT_TYPE_DISCONNECT:
				{
					OnDisconnect(event.peer);
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

		bool OnValidateData(ENetHost* host, const ENetAddress* address, uint32_t data)
		{
			bool valid = true;

			if (!m_clientRegistry->HasClientByConnectionTokenHash(data))
			{
				valid = false;
			}

			// if it's still valid
			if (valid)
			{
				auto& usage = m_connectionUsage[data];

				// if this is the first use of this token, set the address
				if (!usage.count)
				{
					usage.address = address->host;
				}
				else
				{
					// if it's been used too many times already, stop it
					if (usage.count > 3)
					{
						valid = false;
					}
					// or if the address is suddenly different from a prior retry
					else if (memcmp(&usage.address, &address->host, sizeof(in6_addr)) != 0)
					{
						valid = false;
					}
				}

				// increment usage
				usage.count++;
			}

			return valid;
		}

		void OnTimeout(ENetHost* host, ENetPeer* peer)
		{
			auto currentCommand = enet_list_begin(&peer->sentReliableCommands);
			auto currentTime = host->serviceTime;

			std::map<ENetPacket*, OutgoingCommandInfo> outgoingCommandsMap;
			size_t pendingCommandCount = 0;

			while (currentCommand != enet_list_end(&peer->sentReliableCommands))
			{
				auto outgoingCommand = (ENetOutgoingCommand*)currentCommand;
				currentCommand = enet_list_next(currentCommand);

				if (auto packet = outgoingCommand->packet)
				{
					OutgoingCommandInfo info;
					info.size = packet->dataLength;
					info.timeAgo = currentTime - outgoingCommand->sentTime;
					info.type = *(uint32_t*)packet->data;

					if (info.type == HashRageString("msgNetEvent"))
					{
						info.eventName = std::string{ (const char*)packet->data + 8 };
					}
					else
					{
						for (const auto hash : net::PacketNames)
						{
							if (hash.first == info.type)
							{
								info.eventName = std::string{ hash.second };
								break;
							}
						}
					}

					if (auto outIt = outgoingCommandsMap.find(packet); outIt != outgoingCommandsMap.end())
					{
						if (outIt->second.timeAgo < info.timeAgo)
						{
							outIt->second = std::move(info);
						}
					}
					else
					{
						outgoingCommandsMap[packet] = std::move(info);
					}
				}

				pendingCommandCount++;
			}

			std::vector<OutgoingCommandInfo> outgoingCommands;

			for (const auto& cmd : outgoingCommandsMap)
			{
				outgoingCommands.push_back(cmd.second);
			}

			std::sort(outgoingCommands.begin(), outgoingCommands.end(), [](const OutgoingCommandInfo& left, const OutgoingCommandInfo& right)
			{
				return right.size < left.size;
			});

			if (outgoingCommands.size() > 7)
			{
				outgoingCommands.resize(7);
			}

			TimeoutInfo info;
			info.bigCommandList = std::move(outgoingCommands);
			info.pendingCommands = pendingCommandCount;

			auto peerId = static_cast<int>(reinterpret_cast<uintptr_t>(peer->data));
			m_timeoutInfo[peerId] = std::move(info);
		}

		std::unordered_map<int, TimeoutInfo> m_timeoutInfo;

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

			// save the peer as m_peerHandles will see an erase
			auto peer = peerPair->second;

			// enet_peer_reset will not trigger a disconnect event, so we'll manually run that
			OnDisconnect(peer);

			// reset the peer
			enet_peer_reset(peer);
		}

		virtual TimeoutInfo GatherTimeoutInfo(int peerId) override
		{
			auto timeoutData = m_timeoutInfo.find(peerId);

			if (timeoutData == m_timeoutInfo.end())
			{
				return {};
			}

			auto info = std::move(timeoutData->second);
			m_timeoutInfo.erase(peerId);

			return std::move(info);
		}

		virtual void SendPacket(int peer, int channel, const net::Buffer& buffer, NetPacketType type) override
		{
			auto peerPair = m_peerHandles.find(peer);
			if (peerPair == m_peerHandles.end())
			{
				return;
			}

			auto bufferBytes = buffer.GetBuffer();
			auto bufferLength = buffer.GetCurOffset();

			if (!bufferBytes && bufferLength > 0)
			{
#ifdef _DEBUG
				__debugbreak();
#endif
				return;
			}

			// fewer allocations!!
			auto flags = ENET_PACKET_FLAG_NO_ALLOCATE | ((type == NetPacketType_Reliable) ? ENET_PACKET_FLAG_RELIABLE : (ENetPacketFlag)0);
			auto packet = enet_packet_create(bufferBytes, bufferLength, flags);

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
			ENetHost* host = enet_host_create(&addr, std::min(MAX_CLIENTS + 32, int(ENET_PROTOCOL_MAXIMUM_PEER_ID)), 2, 0, 0);

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

			host->peerTimeoutCb = [](ENetHost* host, ENetPeer* peer)
			{
				return g_hostInstances[host]->OnTimeout(host, peer);
			};

			host->validateDataCb = [](ENetHost* host, const ENetAddress* address, uint32_t data)
			{
				return g_hostInstances[host]->OnValidateData(host, address, data);
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

		fwRefContainer<fx::ClientRegistry> m_clientRegistry;

		std::unordered_map<uint32_t, ConnectionUsage> m_connectionUsage;

	public:
		friend class NetPeerImplENet;

		using THostPtr = std::unique_ptr<ENetHost, enet_host_deleter>;

		int m_basePeerId;

		std::vector<THostPtr> hosts;

		std::map<int, ENetPeer*> m_peerHandles;
		std::map<int, uint32_t> m_peerData;

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
