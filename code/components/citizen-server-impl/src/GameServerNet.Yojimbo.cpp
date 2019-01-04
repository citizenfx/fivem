#include <StdInc.h>
#include <GameServerNet.h>

#include <GameServer.h>

#include <NetAddress.h>
#include <NetBuffer.h>

#include <netcode.h>

#define NDEBUG
#include <yojimbo.h>

static const uint8_t DEFAULT_PRIVATE_KEY[yojimbo::KeyBytes] = { 0 };

namespace fx
{
	yojimbo::Address GetYojimboAddress(const net::PeerAddress& peerAddress)
	{
		auto sa = peerAddress.GetSocketAddress();

		if (sa->sa_family == AF_INET)
		{
			auto in4 = (sockaddr_in*)sa;

			uint8_t addr[4];
			memcpy(addr, &in4->sin_addr.s_addr, 4);

			return yojimbo::Address{ addr, ntohs(in4->sin_port) };
		}
		else if (sa->sa_family == AF_INET6)
		{
			auto in6 = (sockaddr_in6*)sa;

			uint16_t addr[8];

			for (int i = 0; i < 8; ++i)
			{
				addr[i] = htons(((uint16_t*)&in6->sin6_addr)[i]);
			}

			return yojimbo::Address{ addr, ntohs(in6->sin6_port) };
		}

		return yojimbo::Address{};
	}

	net::PeerAddress GetPeerAddress(const yojimbo::Address& yjAddress)
	{
		if (yjAddress.GetType() == yojimbo::ADDRESS_IPV4)
		{
			sockaddr_in in = { 0 };
			in.sin_family = AF_INET;
			memcpy(&in.sin_addr.s_addr, yjAddress.GetAddress4(), 4);
			in.sin_port = htons(yjAddress.GetPort());

			return net::PeerAddress{ (sockaddr*)&in, sizeof(in) };
		}
		else if (yjAddress.GetType() == yojimbo::ADDRESS_IPV6)
		{
			sockaddr_in6 in6 = { 0 };
			in6.sin6_family = AF_INET6;

			auto addr = yjAddress.GetAddress6();
			
			for (int i = 0; i < 8; ++i)
			{
				((uint16_t*)&in6.sin6_addr)[i] = ntohs(addr[i]);
			}

			in6.sin6_port = htons(yjAddress.GetPort());			

			return net::PeerAddress{ (sockaddr*)&in6, sizeof(in6) };
		}
		else
		{
			return net::PeerAddress{};
		}
	}

	class NetConnectionConfig : public yojimbo::ClientServerConfig
	{
	public:
		NetConnectionConfig()
		{
			// Cfx exposes 2 channels, we need to distinguish between reliable and unreliable using this
			numChannels = 4;
			channel[0].type = yojimbo::CHANNEL_TYPE_UNRELIABLE_UNORDERED;
			channel[1].type = yojimbo::CHANNEL_TYPE_RELIABLE_ORDERED;
			channel[2].type = yojimbo::CHANNEL_TYPE_UNRELIABLE_UNORDERED;
			channel[3].type = yojimbo::CHANNEL_TYPE_RELIABLE_ORDERED;
		}
	};

	class NetCfxBaseMessage
	{
	public:
		virtual const uint8_t* GetData() = 0;

		virtual size_t GetDataLength() = 0;
	};

	class NetCfxMessage : public yojimbo::Message, public NetCfxBaseMessage
	{
	public:
		int m_length;
		uint8_t m_inlineData[512];

		NetCfxMessage() :
			m_length(0) {}

		template <typename Stream>
		bool Serialize(Stream& stream) {
			serialize_int(stream, m_length, 0, 512);
			serialize_bytes(stream, m_inlineData, m_length);

			return true;
		}

		inline void SetData(const uint8_t* data, size_t size)
		{
			m_length = size;
			memcpy(m_inlineData, data, size);
		}

		inline const uint8_t* GetData()
		{
			return m_inlineData;
		}

		inline size_t GetDataLength()
		{
			return m_length;
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	class NetCfxBlockMessage : public yojimbo::BlockMessage, public NetCfxBaseMessage
	{
	public:
		NetCfxBlockMessage() {}

		template <typename Stream>
		bool Serialize(Stream& stream) {
			return true;
		}

		inline const uint8_t* GetData()
		{
			return GetBlockData();
		}

		inline size_t GetDataLength()
		{
			return GetBlockSize();
		}

		YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
	};

	YOJIMBO_MESSAGE_FACTORY_START(NetMessageFactory, 2);
	YOJIMBO_DECLARE_MESSAGE_TYPE(0, NetCfxMessage);
	YOJIMBO_DECLARE_MESSAGE_TYPE(1, NetCfxBlockMessage);
	YOJIMBO_MESSAGE_FACTORY_FINISH();

	class NetAdapter : public yojimbo::Adapter
	{
	public:
		yojimbo::MessageFactory* CreateMessageFactory(yojimbo::Allocator& allocator) override
		{
			return YOJIMBO_NEW(allocator, NetMessageFactory, allocator);
		}

		void OnServerClientConnected(int clientIndex) override
		{
			OnClientConnected(clientIndex);
		}

		void OnServerClientDisconnected(int clientIndex) override
		{
			OnClientDisconnected(clientIndex);
		}

		fwEvent<int> OnClientConnected;
		fwEvent<int> OnClientDisconnected;
	};

	class GameServerNetImplYojimbo;

	class NetPeerImplYojimbo : public NetPeerBase
	{
	public:
		NetPeerImplYojimbo(GameServerNetImplYojimbo* root, int id)
			: m_root(root), m_id(id)
		{

		}

		virtual int GetId() override
		{
			return m_id;
		}

		virtual void OnSendConnectOK() override
		{
		}

		virtual int GetPing() override;
		virtual int GetPingVariance() override;
		virtual net::PeerAddress GetAddress() override;

	private:
		GameServerNetImplYojimbo* m_root;
		int m_id;
	};

	class NetYojimboServer : public yojimbo::Server
	{
	public:
		NetYojimboServer(yojimbo::Allocator & allocator, const uint8_t privateKey[], const yojimbo::Address & address, const yojimbo::ClientServerConfig & config, yojimbo::Adapter & adapter, double time)
			: yojimbo::Server(allocator, privateKey, address, config, adapter, time)
		{

		}

		void Start(int maxClients) override
		{
			if (IsRunning())
				Stop();

			BaseServer::Start(maxClients);

			char addressString[yojimbo::MaxAddressLength];
			m_address.ToString(addressString, yojimbo::MaxAddressLength);

			struct netcode_server_config_t netcodeConfig;
			netcode_default_server_config(&netcodeConfig);
			netcodeConfig.protocol_id = m_config.protocolId;
			memcpy(netcodeConfig.private_key, m_privateKey, NETCODE_KEY_BYTES);
			netcodeConfig.allocator_context = &GetGlobalAllocator();
			netcodeConfig.allocate_function = StaticAllocateFunction;
			netcodeConfig.free_function = StaticFreeFunction;
			netcodeConfig.callback_context = this;
			netcodeConfig.connect_disconnect_callback = StaticConnectDisconnectCallbackFunction;
			netcodeConfig.send_loopback_packet_callback = StaticSendLoopbackPacketCallbackFunction;
			netcodeConfig.intercept = StaticIntercept;

			m_server = netcode_server_create(addressString, &netcodeConfig, GetTime());

			if (!m_server)
			{
				Stop();
				return;
			}

			netcode_server_start(m_server, maxClients);

			m_boundAddress.SetPort(netcode_server_get_port(m_server));
		}

		inline uint64_t GetSocket()
		{
			return netcode_server_get_ipv4_socket(m_server);
		}

		inline void SetInterceptHandler(const std::function<int(const yojimbo::Address& address, uint8_t* data, int length)> interceptHandler)
		{
			m_interceptHandler = interceptHandler;
		}

	private:
		static int StaticIntercept(void* context, struct netcode_address_t* address, uint8_t* data, int length)
		{
			auto server = (NetYojimboServer*)context;

			return server->Intercept(address, data, length);
		}

		int Intercept(struct netcode_address_t* address, uint8_t* data, int length)
		{
			yojimbo::Address yjAddress;

			if (address->type == NETCODE_ADDRESS_IPV4)
			{
				yjAddress = yojimbo::Address{ address->data.ipv4, address->port };
			}
			else if (address->type == NETCODE_ADDRESS_IPV6)
			{
				yjAddress = yojimbo::Address{ address->data.ipv6, address->port };
			}

			return HandleIntercept(yjAddress, data, length);
		}

		int HandleIntercept(const yojimbo::Address& address, uint8_t* data, int length)
		{
			if (m_interceptHandler)
			{
				return m_interceptHandler(address, data, length);
			}

			return 0;
		}

	private:
		std::function<int(const yojimbo::Address& address, uint8_t* data, int length)> m_interceptHandler;
	};

	class GameServerNetImplYojimbo : public GameServerNetBase
	{
	public:
		GameServerNetImplYojimbo(fx::GameServer* gameServer)
		{
			m_gameServer = gameServer;
		}

		virtual void Process() override
		{
			if (!m_server || !m_server->IsRunning())
			{
				return;
			}

			m_server->AdvanceTime(msec().count() / 1000.0);
			m_server->ReceivePackets();

			for (int client = 0; client < yojimbo::MaxClients; client++)
			{
				if (m_server->IsClientConnected(client))
				{
					for (int channel = 0; channel < m_connectionConfig.numChannels; channel++)
					{
						auto message = m_server->ReceiveMessage(client, channel);

						while (message)
						{
							switch (message->GetType())
							{
							case 0:
							{
								auto msg = static_cast<NetCfxMessage*>(message);
								m_gameServer->ProcessPacket(m_peers[client], msg->GetData(), msg->GetDataLength());

								break;
							}
							case 1:
							{
								auto msg = static_cast<NetCfxBlockMessage*>(message);
								m_gameServer->ProcessPacket(m_peers[client], msg->GetData(), msg->GetDataLength());

								break;
							}
							}

							// next message
							m_server->ReleaseMessage(client, message);
							message = m_server->ReceiveMessage(client, channel);
						}
					}
				}
			}

			m_server->SendPackets();
		}

		virtual void Select(const std::vector<uintptr_t>& addFds, int timeout) override
		{
			std::vector<uint64_t> fds;

			if (m_server)
			{
				fds.push_back(m_server->GetSocket());
			}

			for (auto& fd : addFds)
			{
				fds.push_back(static_cast<uint64_t>(fd));
			}

			fd_set readfds;
			FD_ZERO(&readfds);

			for (auto fd : fds)
			{
				FD_SET(fd, &readfds);
			}

			int nfds = 0;

#ifndef _WIN32
			nfds = *std::max_element(fds.begin(), fds.end());
#endif

			timeval tv;
			tv.tv_sec = timeout / 1000;
			tv.tv_usec = timeout * 1000;
			select(nfds, &readfds, nullptr, nullptr, &tv);
		}

		virtual fwRefContainer<NetPeerBase> GetPeer(int id) override
		{
			return m_peers[id];
		}

		virtual void ResetPeer(int id) override
		{
			m_server->DisconnectClient(id);
		}

		virtual void SendPacket(int peer, int channel, const net::Buffer & buffer, NetPacketType type) override
		{
			yojimbo::Message* message;

			if (buffer.GetLength() > 512)
			{
				auto m = (NetCfxBlockMessage*)m_server->CreateMessage(peer, 1);

				auto block = m_server->AllocateBlock(peer, buffer.GetLength());
				memcpy(block, buffer.GetBuffer(), buffer.GetLength());

				m_server->AttachBlockToMessage(peer, m, block, buffer.GetLength());

				message = m;
			}
			else
			{
				auto m = (NetCfxMessage*)m_server->CreateMessage(peer, 0);
				m->SetData(buffer.GetBuffer(), buffer.GetLength());

				message = m;
			}

			m_server->SendMessage(peer, (channel * 2) + ((type == NetPacketType_Reliable) ? 1 : 0), message);
		}

		virtual void SendOutOfBand(const net::PeerAddress& to, const std::string_view & oob) override
		{
			// TODO: very temporary
			auto oobMsg = "\xFF\xFF\xFF\xFF" + std::string(oob);

			sendto((SOCKET)m_server->GetSocket(), oobMsg.data(), oobMsg.size(), 0, to.GetSocketAddress(), to.GetSocketAddressLength());
		}

		virtual void CreateUdpHost(const net::PeerAddress & address) override
		{
			m_adapter.OnClientConnected.Connect([this](int clientIdx)
			{
				m_peers[clientIdx] = new NetPeerImplYojimbo(this, clientIdx);
			});

			m_adapter.OnClientDisconnected.Connect([this](int clientIdx)
			{
				m_peers[clientIdx] = nullptr;
			});

			auto server = std::make_unique<NetYojimboServer>(yojimbo::GetDefaultAllocator(), DEFAULT_PRIVATE_KEY, GetYojimboAddress(address), m_connectionConfig, m_adapter, msec().count() / 1000.0);

			server->Start(64);

			server->SetInterceptHandler([this](const yojimbo::Address& address, uint8_t* data, int length)
			{
				return (m_interceptor(data, length, GetPeerAddress(address))) ? 1 : 0;
			});

			m_server = std::move(server);
		}

		virtual void AddRawInterceptor(const std::function<bool(const uint8_t*, size_t, const net::PeerAddress&)>& interceptor) override
		{
			m_interceptor = interceptor;
		}

	public:
		friend class NetPeerImplYojimbo;

	private:
		NetConnectionConfig m_connectionConfig;
		NetAdapter m_adapter;

		std::function<bool(const uint8_t*, size_t, const net::PeerAddress&)> m_interceptor;

		fwRefContainer<NetPeerImplYojimbo> m_peers[yojimbo::MaxClients];

		std::unique_ptr<NetYojimboServer> m_server;

		fx::GameServer* m_gameServer;
	};


	int NetPeerImplYojimbo::GetPing()
	{
		yojimbo::NetworkInfo info;
		m_root->m_server->GetNetworkInfo(m_id, info);

		return info.RTT;
	}

	int NetPeerImplYojimbo::GetPingVariance()
	{
		return 0;
	}

	net::PeerAddress NetPeerImplYojimbo::GetAddress()
	{
		return *(net::PeerAddress::FromString(fmt::sprintf("0.0.0.%i", m_id)));
	}

	/*fwRefContainer<GameServerNetBase> CreateGSNet(fx::GameServer* server)
	{
		return new GameServerNetImplYojimbo(server);
	}*/
}

static InitFunction initFunction([]()
{
	InitializeYojimbo();

	yojimbo_set_printf_function([](const char* b, ...)
	{
		static char buffer[16384];

		va_list ap;
		va_start(ap, b);
		vsnprintf(buffer, sizeof(buffer) - 1, b, ap);
		va_end(ap);

		trace("%s", buffer);

		return 0;
	});

	yojimbo_log_level(YOJIMBO_LOG_LEVEL_INFO);
});
