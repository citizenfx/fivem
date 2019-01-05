#include "StdInc.h"

#include <NetAddress.h>
#include <NetBuffer.h>

#include <CoreConsole.h>

#include <netcode.h>

#define NDEBUG
#include <yojimbo.h>

#include <YojimboHelpers.h>

#include <mmsystem.h>

#include "NetLibrary.h"
#include "NetLibraryImplBase.h"

static const uint8_t DEFAULT_PRIVATE_KEY[yojimbo::KeyBytes] = { 0 };

class NetYojimboClient : public yojimbo::Client
{
public:
	NetYojimboClient(yojimbo::Allocator & allocator, const yojimbo::Address & address, const yojimbo::ClientServerConfig & config, yojimbo::Adapter & adapter, double time)
		: yojimbo::Client(allocator, address, config, adapter, time)
	{

	}

	void CreateClient(const yojimbo::Address & address) override
	{
		DestroyClient();
		char addressString[yojimbo::MaxAddressLength];
		address.ToString(addressString, yojimbo::MaxAddressLength);

		struct netcode_client_config_t netcodeConfig;
		netcode_default_client_config(&netcodeConfig);
		netcodeConfig.allocator_context = &GetClientAllocator();
		netcodeConfig.allocate_function = StaticAllocateFunction;
		netcodeConfig.free_function = StaticFreeFunction;
		netcodeConfig.callback_context = this;
		netcodeConfig.state_change_callback = StaticStateChangeCallbackFunction;
		netcodeConfig.send_loopback_packet_callback = StaticSendLoopbackPacketCallbackFunction;
		netcodeConfig.intercept = StaticIntercept;
		m_client = netcode_client_create(addressString, &netcodeConfig, GetTime());

		if (m_client)
		{
			m_boundAddress.SetPort(netcode_client_get_port(m_client));
		}
	}

	inline uint64_t GetSocket()
	{
		return netcode_client_get_ipv4_socket(m_client);
	}

	inline void SetInterceptHandler(const std::function<int(const yojimbo::Address& address, uint8_t* data, int length)> interceptHandler)
	{
		m_interceptHandler = interceptHandler;
	}

private:
	static int StaticIntercept(void* context, struct netcode_address_t* address, uint8_t* data, int length)
	{
		auto server = (NetYojimboClient*)context;

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

class NetLibraryImplV3 : public NetLibraryImplBase
{
public:
	NetLibraryImplV3(INetLibraryInherit* base);

	virtual ~NetLibraryImplV3() override;

	virtual void CreateResources() override;

	virtual void RunFrame() override;

	virtual void SendData(const NetAddress& netAddress, const char* data, size_t length) override;

	virtual void SendReliableCommand(uint32_t type, const char* buffer, size_t length) override;

	virtual bool HasTimedOut() override;

	virtual void Reset() override;

	virtual void Flush() override;

	virtual void SendConnect(const std::string& connectData) override;

private:
	void ProcessPacket(const uint8_t* data, size_t size);

	inline yojimbo::Message* CreateMessage(const uint8_t* data, size_t size)
	{
		if (size > 512)
		{
			auto msg = (fx::NetCfxBlockMessage*)m_client->CreateMessage(1);
			auto block = m_client->AllocateBlock(size);
			memcpy(block, data, size);

			m_client->AttachBlockToMessage(msg, block, size);

			return msg;
		}

		auto msg = (fx::NetCfxMessage*)m_client->CreateMessage(0);
		msg->SetData(data, size);

		return msg;
	}

private:
	INetLibraryInherit* m_base;

	std::string m_connectData;

	fx::NetConnectionConfig m_connectionConfig;
	fx::NetAdapter m_adapter;

	std::unique_ptr<NetYojimboClient> m_client;

	std::unique_ptr<NetYojimboClient> m_fakeClient;

	bool m_timedOut;

	bool m_connecting;

	std::shared_ptr<ConVar<int>> m_maxPackets;

	uint32_t m_lastKeepaliveSent;

	size_t m_outDataSize;
};

NetLibraryImplV3::NetLibraryImplV3(INetLibraryInherit* base)
	: m_timedOut(false), m_lastKeepaliveSent(0), m_base(base), m_connecting(false), m_outDataSize(0)
{
	m_maxPackets = std::make_shared<ConVar<int>>("net_maxPackets", ConVar_Archive, 50);
	m_maxPackets->GetHelper()->SetConstraints(1, 200);

	CreateResources();

	Reset();
}

NetLibraryImplV3::~NetLibraryImplV3()
{
}

void NetLibraryImplV3::CreateResources()
{
	m_client = std::make_unique<NetYojimboClient>(yojimbo::GetDefaultAllocator(), yojimbo::Address("0.0.0.0"), m_connectionConfig, m_adapter, yojimbo_time());

	m_client->SetInterceptHandler([this](const yojimbo::Address& address, uint8_t* receivedData, int receivedDataLength)
	{
		if (*(int*)receivedData == -1)
		{
			m_base->ProcessOOB(NetAddress(fx::GetPeerAddress(address).GetSocketAddress()), (char*)receivedData + 4, receivedDataLength - 4);
			return 1;
		}

		return 0;
	});

	m_fakeClient = std::make_unique<NetYojimboClient>(yojimbo::GetDefaultAllocator(), yojimbo::Address("0.0.0.0"), m_connectionConfig, m_adapter, yojimbo_time());

	m_fakeClient->SetInterceptHandler([this](const yojimbo::Address& address, uint8_t* receivedData, int receivedDataLength)
	{
		if (*(int*)receivedData == -1)
		{
			m_base->ProcessOOB(NetAddress(fx::GetPeerAddress(address).GetSocketAddress()), (char*)receivedData + 4, receivedDataLength - 4);
			return 1;
		}

		return 0;
	});
}

void NetLibraryImplV3::SendReliableCommand(uint32_t type, const char* buffer, size_t length)
{
	NetBuffer msg(131072);
	msg.Write(type);
	msg.Write(buffer, length);

	if (!m_timedOut && m_client && m_client->IsConnected())
	{
		auto message = CreateMessage(reinterpret_cast<const uint8_t*>(msg.GetBuffer()), msg.GetCurLength());
		m_outDataSize += msg.GetCurLength();

		m_client->SendMessage(1, message);
	}

	m_base->GetMetricSink()->OnOutgoingCommand(type, length);
}

void NetLibraryImplV3::SendData(const NetAddress& netAddress, const char* data, size_t length)
{
	sockaddr_storage sa;
	int salen;

	netAddress.GetSockAddr(&sa, &salen);

	// temp workaround
	if (!m_fakeClient->IsConnected() && !m_fakeClient->IsConnecting())
	{
		m_fakeClient->InsecureConnect(DEFAULT_PRIVATE_KEY, 0, yojimbo::Address{ "0.0.0.0:1" });
	}

	sendto((SOCKET)m_fakeClient->GetSocket(), data, length, 0, (sockaddr*)&sa, salen);
}

bool NetLibraryImplV3::HasTimedOut()
{
	return m_timedOut || !m_client->IsConnected();
}

void NetLibraryImplV3::Reset()
{
	m_timedOut = false;

	m_client->Disconnect();
}

void NetLibraryImplV3::Flush()
{
	m_client->AdvanceTime(yojimbo_time());

	m_client->SendPackets();
	m_client->SendPackets();
}

void NetLibraryImplV3::RunFrame()
{
	uint32_t inDataSize = 0;

	if (!m_client)
	{
		return;
	}

	m_client->AdvanceTime(yojimbo_time());
	m_client->ReceivePackets();

	m_fakeClient->AdvanceTime(yojimbo_time());
	m_fakeClient->ReceivePackets();
	m_fakeClient->SendPackets();

	if (m_client->IsConnected())
	{
		if (m_connecting)
		{
			SendReliableCommand(1, m_connectData.c_str(), m_connectData.size());

			m_connecting = false;
		}

		for (int i = 0; i < m_connectionConfig.numChannels; i++)
		{
			auto message = m_client->ReceiveMessage(i);

			while (message)
			{
				if (message->GetType() == 0)
				{
					auto msg = (fx::NetCfxMessage*)message;

					ProcessPacket(msg->GetData(), msg->GetDataLength());
					inDataSize += msg->GetDataLength();
				}
				else if (message->GetType() == 1)
				{
					auto msg = (fx::NetCfxBlockMessage*)message;

					ProcessPacket(msg->GetData(), msg->GetDataLength());
					inDataSize += msg->GetDataLength();
				}

				m_client->ReleaseMessage(message);

				message = m_client->ReceiveMessage(i);
			}
		}
	}
	else if (m_client->IsDisconnected())
	{
		m_timedOut = true;
	}

	m_client->SendPackets();

	if (m_client->IsConnected() && !m_timedOut)
	{
		RoutingPacket packet;

		while (m_base->GetOutgoingPacket(packet))
		{
			NetBuffer msg(1300);
			msg.Write(0xE938445B); // msgRoute
			msg.Write(packet.netID);
			msg.Write<uint16_t>(packet.payload.size());

			//trace("sending msgRoute to %d len %d\n", packet.netID, packet.payload.size());

			msg.Write(packet.payload.c_str(), packet.payload.size());

			auto message = CreateMessage(reinterpret_cast<const uint8_t*>(msg.GetBuffer()), msg.GetCurLength());
			m_outDataSize += msg.GetCurLength();

			m_client->SendMessage(2, message); // 2: unreliable-1
			//enet_peer_send(m_serverPeer, 1, enet_packet_create(msg.GetBuffer(), msg.GetCurLength(), ENET_PACKET_FLAG_UNSEQUENCED));

			m_base->GetMetricSink()->OnOutgoingCommand(0xE938445B, packet.payload.size() + 4);
			m_base->GetMetricSink()->OnOutgoingRoutePackets(1);
		}

		if ((timeGetTime() - m_lastKeepaliveSent) > static_cast<uint32_t>(1000 / m_maxPackets->GetValue()))
		{
			NetBuffer msg(1300);
			msg.Write(0xCA569E63); // msgEnd

			//enet_peer_send(m_serverPeer, 1, enet_packet_create(msg.GetBuffer(), msg.GetCurLength(), ENET_PACKET_FLAG_UNSEQUENCED));

			auto message = CreateMessage(reinterpret_cast<const uint8_t*>(msg.GetBuffer()), msg.GetCurLength());
			m_outDataSize += msg.GetCurLength();

			m_client->SendMessage(2, message);

			m_lastKeepaliveSent = timeGetTime();
		}

		// update ping metrics
		yojimbo::NetworkInfo info;
		m_client->GetNetworkInfo(info);

		static int lastPacketsReceived;
		static int lastPacketsSent;

		auto infoReceivedBandwidth = info.receivedBandwidth;
		auto infoSentBandwidth = info.sentBandwidth;

		m_base->GetMetricSink()->OnPingResult(info.RTT);

		// update received metrics
		if (info.receivedBandwidth != 0)
		{
			int pr = info.numPacketsReceived - lastPacketsReceived;
			lastPacketsReceived = info.numPacketsReceived;

			if (pr < 0)
			{
				pr = 0;
			}

			for (uint32_t i = 0; i < pr; ++i)
			{
				NetPacketMetrics m;
				m.AddElementSize(NET_PACKET_SUB_MISC, inDataSize);

				// actually: overhead
				//m.AddElementSize(NET_PACKET_SUB_RELIABLES, info.receivedBandwidth - inDataSize);

				m_base->GetMetricSink()->OnIncomingPacket(m);

				info.receivedBandwidth = 0;
				inDataSize = 0;
			}

			m_base->AddReceiveTick();
		}

		// update sent metrics
		if (info.sentBandwidth != 0)
		{
			int ps = info.numPacketsSent - lastPacketsSent;
			lastPacketsSent = info.numPacketsSent;

			if (ps < 0)
			{
				ps = 0;
			}

			for (uint32_t i = 0; i < ps; ++i)
			{
				NetPacketMetrics m;
				m.AddElementSize(NET_PACKET_SUB_MISC, m_outDataSize);
				m_outDataSize = 0;

				m_base->GetMetricSink()->OnOutgoingPacket(m);

				info.sentBandwidth = 0;
			}

			info.sentBandwidth = 0;

			m_base->AddSendTick();
		}

		//m_base->GetMetricSink()->OverrideBandwidthStats(infoReceivedBandwidth * 1000.0f, infoSentBandwidth * 1000.0f);

		NetLibrary::OnBuildMessage(std::bind(&NetLibraryImplV3::SendReliableCommand, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}
}

void NetLibraryImplV3::SendConnect(const std::string& connectData)
{
	m_timedOut = false;
	m_connectData = connectData;

	uint64_t clientId;
	yojimbo::random_bytes((uint8_t*)&clientId, 8);

	auto addr = fx::GetYojimboAddress(m_base->GetCurrentPeer());
	m_client->InsecureConnect(DEFAULT_PRIVATE_KEY, clientId, addr);

	m_connecting = true;

#ifdef _DEBUG
	// TODO: disable timeout
#endif
}

void NetLibraryImplV3::ProcessPacket(const uint8_t* data, size_t size)
{
	NetBuffer msg((char*)data, size);
	uint32_t msgType = msg.Read<uint32_t>();

	if (msgType == 1)
	{
		char dataCopy[8192];
		memcpy(dataCopy, data, fwMin(size, sizeof(dataCopy)));
		dataCopy[size] = '\0';

		char* clientNetIDStr = &dataCopy[5];
		char* hostIDStr = strchr(clientNetIDStr, ' ');

		hostIDStr[0] = '\0';
		hostIDStr++;

		char* hostBaseStr = strchr(hostIDStr, ' ');

		hostBaseStr[0] = '\0';
		hostBaseStr++;

		char* slotIDStr = strchr(hostBaseStr, ' ');

		if (slotIDStr)
		{
			slotIDStr[0] = '\0';
			slotIDStr++;
		}
		else
		{
			slotIDStr = "-1";
		}

		char* timeStr = strchr(slotIDStr, ' ');

		if (timeStr)
		{
			timeStr[0] = '\0';
			timeStr++;
		}
		else
		{
			timeStr = "-1";
		}

		m_base->HandleConnected(atoi(clientNetIDStr), atoi(hostIDStr), atoi(hostBaseStr), atoi(slotIDStr), _strtoi64(timeStr, nullptr, 10));

		return;
	}

	if (m_base->GetConnectionState() == NetLibrary::CS_CONNECTED)
	{
		m_base->SetConnectionState(NetLibrary::CS_ACTIVE);
	}

	if (m_base->GetConnectionState() != NetLibrary::CS_ACTIVE)
	{
		return;
	}

	m_base->GetMetricSink()->OnIncomingCommand(msgType, size);

	if (msgType == 0xE938445B) // 'msgRoute'
	{
		uint16_t netID = msg.Read<uint16_t>();
		uint16_t rlength = msg.Read<uint16_t>();

		//trace("msgRoute from %d len %d\n", netID, rlength);

		char routeBuffer[65536];
		if (!msg.Read(routeBuffer, rlength))
		{
			return;
		}

		m_base->EnqueueRoutedPacket(netID, std::string(routeBuffer, rlength));

		// add to metrics
		m_base->GetMetricSink()->OnIncomingRoutePackets(1);
	}
	else if (msgType != 0xCA569E63) // reliable command
	{
		size_t reliableSize = size - 4;

		std::vector<char> reliableBuf(reliableSize);

		if (!msg.Read(reliableBuf.data(), reliableSize))
		{
			return;
		}

		// check to prevent double execution
		m_base->HandleReliableCommand(msgType, reliableBuf.data(), reliableBuf.size());
	}
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

std::unique_ptr<NetLibraryImplBase> CreateNetLibraryImplV3(INetLibraryInherit* base)
{
	return std::make_unique<NetLibraryImplV3>(base);
}
