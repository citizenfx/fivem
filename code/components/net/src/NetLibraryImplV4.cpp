#include "StdInc.h"
#include "NetLibrary.h"
#include "NetLibraryImplBase.h"

#include <ICoreGameInit.h>

#include <slikenet/BitStream.h>
#include <slikenet/peerinterface.h>
#include <slikenet/MessageIdentifiers.h>
#include <slikenet/statistics.h>

#include <mmsystem.h>
#include <CoreConsole.h>

#pragma comment(lib, "winmm.lib")

class NetLibraryImplV4 : public NetLibraryImplBase
{
public:
	NetLibraryImplV4(INetLibraryInherit* base);

	virtual ~NetLibraryImplV4() override;

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

private:
	std::string m_connectData;

	INetLibraryInherit* m_base;

	SLNet::RakPeerInterface* m_peer;

	SLNet::SystemAddress m_server;

	bool m_timedOut;

	std::shared_ptr<ConVar<int>> m_maxPackets;

	uint32_t m_lastKeepaliveSent;
};

NetLibraryImplV4::NetLibraryImplV4(INetLibraryInherit* base)
	: m_base(base), m_timedOut(false), m_peer(nullptr), m_lastKeepaliveSent(0)
{
	m_maxPackets = std::make_shared<ConVar<int>>("net_maxPackets", ConVar_Archive, 50);
	m_maxPackets->GetHelper()->SetConstraints(1, 200);

	CreateResources();

	Reset();
}

NetLibraryImplV4::~NetLibraryImplV4()
{

}

void NetLibraryImplV4::CreateResources()
{
	m_peer = SLNet::RakPeerInterface::GetInstance();

	SLNet::SocketDescriptor sd{ 0, nullptr };
	m_peer->Startup(1, &sd, 1);
	m_peer->SetMaximumIncomingConnections(0);

	static NetLibraryImplV4* nl = this;
	static INetLibraryInherit* base = m_base;

	m_peer->SetIncomingDatagramEventHandler([](SLNet::RNS2RecvStruct* rrs)
	{
		if (*(int*)rrs->data == -1)
		{
			base->ProcessOOB(NetAddress{ (sockaddr*)&rrs->systemAddress.address }, &rrs->data[4], rrs->bytesRead - 4);
			return false;
		}

		return true;
	});
}

void NetLibraryImplV4::SendReliableCommand(uint32_t type, const char* buffer, size_t length)
{
	SLNet::BitStream bs;
	bs.Write((uint8_t)(ID_USER_PACKET_ENUM + 1));
	bs.Write(htonl(type));
	bs.Write(buffer, length);

	if (!m_timedOut && m_peer)
	{
		m_peer->Send(&bs, MEDIUM_PRIORITY, RELIABLE_ORDERED, 0, m_server, false);
	}

	m_base->GetMetricSink()->OnOutgoingCommand(type, length);
}

void NetLibraryImplV4::SendData(const NetAddress& netAddress, const char* data, size_t length)
{
	sockaddr_storage sa;
	int salen;

	netAddress.GetSockAddr(&sa, &salen);

	auto to = net::PeerAddress{(sockaddr*)&sa, salen};

	auto rsa = SLNet::SystemAddress{
		to.GetHost().c_str(),
		uint16_t(to.GetPort())
	};

	SLNet::RNS2_SendParameters rsp;
	rsp.systemAddress = rsa;
	rsp.data = const_cast<char*>(data);
	rsp.length = length;

	DataStructures::List<SLNet::RakNetSocket2*> sockets;
	m_peer->GetSockets(sockets);

	sockets[0]->Send(&rsp, __FILE__, __LINE__);
}

bool NetLibraryImplV4::HasTimedOut()
{
	return m_timedOut;
}

void NetLibraryImplV4::Reset()
{
	m_timedOut = false;

	m_peer->CloseConnection(m_server, true);
}

void NetLibraryImplV4::Flush()
{
	// TODO: find a way to properly flush any packets (this is used in quit!)

	for (int i = 0; i < 5; i++)
	{
		SLNet::BitStream ubs(MAXIMUM_MTU_SIZE);
		m_peer->RunUpdateCycle(ubs);
	}
}

void NetLibraryImplV4::RunFrame()
{
	uint32_t inDataSize = 0;

	SLNet::Packet* packet = m_peer->Receive();

	while (packet)
	{
		switch (packet->data[0])
		{
		case ID_USER_PACKET_ENUM + 1:
		{
			ProcessPacket(packet->data + 1, packet->length - 1);
			inDataSize += packet->length;

			break;
		}
		case ID_CONNECTION_REQUEST_ACCEPTED:
		{
			m_server = packet->systemAddress;

			SLNet::BitStream bs;
			bs.Write((uint8_t)(ID_USER_PACKET_ENUM + 1));
			bs.Write((uint8_t)1); // 1 but little endian
			bs.Write((uint8_t)0);
			bs.Write((uint8_t)0);
			bs.Write((uint8_t)0);
			bs.Write(m_connectData.c_str(), m_connectData.size());

			m_peer->Send(&bs, IMMEDIATE_PRIORITY, RELIABLE_ORDERED, 0, m_server, false);
			break;
		}
		case ID_CONNECTION_LOST:
		case ID_DISCONNECTION_NOTIFICATION:
			m_timedOut = true;
			break;
		}

		m_peer->DeallocatePacket(packet);
		packet = m_peer->Receive();
	}

	if (!m_timedOut)
	{
		RoutingPacket packet;

		while (m_base->GetOutgoingPacket(packet))
		{
			net::Buffer b;
			b.Write(0xE938445B); // msgRoute
			b.Write(packet.netID);
			b.Write((uint16_t)packet.payload.size());

			SLNet::BitStream msg;
			msg.Write((uint8_t)(ID_USER_PACKET_ENUM + 1));
			msg.Write((char*)b.GetData().data(), b.GetCurOffset());
			msg.Write(packet.payload.c_str(), packet.payload.size());

			m_peer->Send(&msg, HIGH_PRIORITY, UNRELIABLE, 1, m_server, false);

			m_base->GetMetricSink()->OnOutgoingCommand(0xE938445B, packet.payload.size() + 4);
			m_base->GetMetricSink()->OnOutgoingRoutePackets(1);
		}

		if ((timeGetTime() - m_lastKeepaliveSent) > static_cast<uint32_t>(1000 / m_maxPackets->GetValue()))
		{
			SLNet::BitStream msg;
			msg.Write((uint8_t)(ID_USER_PACKET_ENUM + 1));
			msg.Write(htonl(0xCA569E63)); // msgEnd

			m_peer->Send(&msg, HIGH_PRIORITY, UNRELIABLE, 1, m_server, false);

			m_lastKeepaliveSent = timeGetTime();
		}

		// update ping metrics
		m_base->GetMetricSink()->OnPingResult(m_peer->GetLastPing(m_server));

		SLNet::RakNetStatistics stats;
		m_peer->GetStatistics(m_server, &stats);

		// update received metrics
		static uint64_t lastReceived;

		uint64_t received = stats.runningTotal[SLNet::ACTUAL_BYTES_RECEIVED] - lastReceived;
		lastReceived = stats.runningTotal[SLNet::ACTUAL_BYTES_RECEIVED];

		if (received != 0)
		{
			NetPacketMetrics m;
			m.AddElementSize(NET_PACKET_SUB_MISC, inDataSize);

			// actually: overhead
			m.AddElementSize(NET_PACKET_SUB_RELIABLES, received - inDataSize);

			m_base->GetMetricSink()->OnIncomingPacket(m);

			inDataSize = 0;

			m_base->AddReceiveTick();
		}

		// update sent metrics
		static uint64_t lastSent;

		uint64_t sent = stats.runningTotal[SLNet::ACTUAL_BYTES_SENT] - lastSent;
		lastSent = stats.runningTotal[SLNet::ACTUAL_BYTES_SENT];

		if (sent != 0)
		{
			NetPacketMetrics m;
			m.AddElementSize(NET_PACKET_SUB_MISC, sent);

			m_base->GetMetricSink()->OnOutgoingPacket(m);

			m_base->AddSendTick();
		}

		NetLibrary::OnBuildMessage(std::bind(&NetLibraryImplV4::SendReliableCommand, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}
}

void NetLibraryImplV4::SendConnect(const std::string& connectData)
{
	m_timedOut = false;
	m_connectData = connectData;

	m_peer->Connect(m_base->GetCurrentPeer().GetHost().c_str(), uint16_t(m_base->GetCurrentPeer().GetPort()), nullptr, 0);

#ifdef _DEBUG
	m_peer->SetTimeoutTime(86400 * 1000, SLNet::UNASSIGNED_SYSTEM_ADDRESS);
#endif
}

void NetLibraryImplV4::ProcessPacket(const uint8_t* data, size_t size)
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
	enet_initialize();
});

std::unique_ptr<NetLibraryImplBase> CreateNetLibraryImplV4(INetLibraryInherit* base)
{
	return std::make_unique<NetLibraryImplV4>(base);
}
