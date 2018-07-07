#include "StdInc.h"
#include "NetLibrary.h"
#include "NetLibraryImplBase.h"

#include <ICoreGameInit.h>

#include <enet/enet.h>

#include <mmsystem.h>
#include <CoreConsole.h>

#pragma comment(lib, "winmm.lib")

class NetLibraryImplV2 : public NetLibraryImplBase
{
public:
	NetLibraryImplV2(INetLibraryInherit* base);

	virtual ~NetLibraryImplV2() override;

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

	ENetHost* m_host;

	ENetPeer* m_serverPeer;

	bool m_timedOut;

	std::shared_ptr<ConVar<int>> m_maxPackets;

	uint32_t m_lastKeepaliveSent;
};

NetLibraryImplV2::NetLibraryImplV2(INetLibraryInherit* base)
	: m_base(base), m_host(nullptr), m_timedOut(false), m_serverPeer(nullptr), m_lastKeepaliveSent(0)
{
	m_maxPackets = std::make_shared<ConVar<int>>("net_maxPackets", ConVar_Archive, 50);
	m_maxPackets->GetHelper()->SetConstraints(1, 200);

	CreateResources();

	Reset();
}

NetLibraryImplV2::~NetLibraryImplV2()
{
	if (m_host)
	{
		enet_host_destroy(m_host);
	}
}

void NetLibraryImplV2::CreateResources()
{
	// TODO: dynamically rate limit based on bandwidth estimates
	m_host = enet_host_create(
		nullptr, // this is a client
		1, // 1 peer max
		2, // support 2 channels
		0, // no rate limiting for now
		0);

	static NetLibraryImplV2* nl = this;
	static INetLibraryInherit* base = m_base;

	m_host->intercept = [](ENetHost* host, ENetEvent*)
	{
		if (*(int*)host->receivedData == -1)
		{
			base->ProcessOOB(NetAddress(&host->receivedAddress), (char*)host->receivedData + 4, host->receivedDataLength - 4);
			return 1;
		}

		return 0;
	};
}

void NetLibraryImplV2::SendReliableCommand(uint32_t type, const char* buffer, size_t length)
{
	NetBuffer msg(131072);
	msg.Write(type);
	msg.Write(buffer, length);

	if (!m_timedOut && m_serverPeer)
	{
		ENetPacket* packet = enet_packet_create(msg.GetBuffer(), msg.GetCurLength(), ENET_PACKET_FLAG_RELIABLE);

		enet_peer_send(m_serverPeer, 0, packet);
	}
}

void NetLibraryImplV2::SendData(const NetAddress& netAddress, const char* data, size_t length)
{
	auto addr = netAddress.GetENetAddress();

	ENetBuffer buffer;
	buffer.data = (uint8_t*)data;
	buffer.dataLength = length;

	enet_socket_send(m_host->socket, &addr, &buffer, 1);
}

bool NetLibraryImplV2::HasTimedOut()
{
	return m_timedOut;
}

void NetLibraryImplV2::Reset()
{
	if (m_serverPeer)
	{
		enet_peer_disconnect(m_serverPeer, 0);
	}
}

void NetLibraryImplV2::Flush()
{
	enet_host_flush(m_host);
}

void NetLibraryImplV2::RunFrame()
{
	uint32_t inDataSize = 0;

	ENetEvent event;

	while (enet_host_service(m_host, &event, 0) > 0)
	{
		switch (event.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
		{
			NetBuffer buf(1300);
			buf.Write<int>(1);
			buf.Write(m_connectData.c_str(), m_connectData.size());

			ENetPacket* packet = enet_packet_create(buf.GetBuffer(), buf.GetCurLength(), ENET_PACKET_FLAG_RELIABLE);
			enet_peer_send(event.peer, 0, packet);
			break;
		}
		case ENET_EVENT_TYPE_RECEIVE:
		{
			ProcessPacket(event.packet->data, event.packet->dataLength);
			inDataSize += event.packet->dataLength;

			break;
		}
		case ENET_EVENT_TYPE_DISCONNECT:
			m_timedOut = true;
			break;
		}
	}

	if (m_serverPeer && !m_timedOut)
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

			enet_peer_send(m_serverPeer, 1, enet_packet_create(msg.GetBuffer(), msg.GetCurLength(), ENET_PACKET_FLAG_UNSEQUENCED));

			m_base->GetMetricSink()->OnOutgoingRoutePackets(1);
		}

		if ((timeGetTime() - m_lastKeepaliveSent) > static_cast<uint32_t>(1000 / m_maxPackets->GetValue()))
		{
			NetBuffer msg(1300);
			msg.Write(0xCA569E63); // msgEnd

			enet_peer_send(m_serverPeer, 1, enet_packet_create(msg.GetBuffer(), msg.GetCurLength(), ENET_PACKET_FLAG_UNSEQUENCED));

			m_lastKeepaliveSent = timeGetTime();
		}

		// update ping metrics
		m_base->GetMetricSink()->OnPingResult(m_serverPeer->lastRoundTripTime);

		// update received metrics
		if (m_host->totalReceivedData != 0)
		{
			for (uint32_t i = 0; i < m_host->totalReceivedPackets; ++i)
			{
				NetPacketMetrics m;
				m.AddElementSize(NET_PACKET_SUB_MISC, inDataSize);

				// actually: overhead
				m.AddElementSize(NET_PACKET_SUB_RELIABLES, m_host->totalReceivedData - inDataSize);

				m_base->GetMetricSink()->OnIncomingPacket(m);

				m_host->totalReceivedData = 0;
				inDataSize = 0;
			}

			m_host->totalReceivedPackets = 0;
		}

		// update sent metrics
		if (m_host->totalSentData != 0)
		{
			for (uint32_t i = 0; i < m_host->totalSentPackets; ++i)
			{
				NetPacketMetrics m;
				m.AddElementSize(NET_PACKET_SUB_MISC, m_host->totalSentData);

				m_base->GetMetricSink()->OnOutgoingPacket(m);

				m_host->totalSentData = 0;
			}

			m_host->totalSentPackets = 0;
		}

		NetLibrary::OnBuildMessage(std::bind(&NetLibraryImplV2::SendReliableCommand, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}
}

void NetLibraryImplV2::SendConnect(const std::string& connectData)
{
	m_connectData = connectData;

	auto addr = m_base->GetCurrentServer().GetENetAddress();
	m_serverPeer = enet_host_connect(m_host, &addr, 2, 0);

#ifdef _DEBUG
	enet_peer_timeout(m_serverPeer, 86400 * 1000, 86400 * 1000, 86400 * 1000);
#endif
}

void NetLibraryImplV2::ProcessPacket(const uint8_t* data, size_t size)
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

std::unique_ptr<NetLibraryImplBase> CreateNetLibraryImplV2(INetLibraryInherit* base)
{
	return std::make_unique<NetLibraryImplV2>(base);
}
