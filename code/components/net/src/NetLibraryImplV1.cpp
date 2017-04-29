/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include "NetLibrary.h"
#include "NetLibraryImplBase.h"

#include <ICoreGameInit.h>

#include <mmsystem.h>

class NetLibraryImplV1 : public NetLibraryImplBase
{
public:
	NetLibraryImplV1(INetLibraryInherit* base);

	virtual ~NetLibraryImplV1() override;

	virtual void CreateResources() override;

	virtual void RunFrame() override;

	virtual void SendData(const NetAddress& netAddress, const char* data, size_t length) override;

	virtual void SendReliableCommand(uint32_t type, const char* buffer, size_t length) override;

	virtual bool HasTimedOut() override;

	virtual void Reset() override;

	virtual void Flush() override;

	virtual void SendConnect(const std::string& connectData) override;

private:
	void ProcessSend();

	void ProcessPackets();

	void ProcessPacketsInternal(NetAddressType addrType);

	void ProcessServerMessage(NetBuffer& message);

private:
	struct OutReliableCommand
	{
		uint32_t id;
		uint32_t type;
		std::string command;
	};

private:
	INetLibraryInherit* m_base;

	SOCKET m_socket;

	SOCKET m_socket6;

	NetChannel m_netChannel;

	uint32_t m_lastReceivedAt;

	uint32_t m_lastReceivedReliableCommand;

	uint32_t m_outReliableAcknowledged;

	uint32_t m_outReliableSequence;

	std::list<OutReliableCommand> m_outReliableCommands;

	uint32_t m_lastFrameNumber;

	uint32_t m_lastSend;

	std::string m_iHost;
};

NetLibraryImplV1::NetLibraryImplV1(INetLibraryInherit* base)
	:m_lastSend(0), m_lastReceivedReliableCommand(0), m_outReliableAcknowledged(0), m_outReliableSequence(0),
	 m_lastReceivedAt(0), m_base(base)
{
	CreateResources();
	Reset();
}

NetLibraryImplV1::~NetLibraryImplV1()
{
	closesocket(m_socket);
	closesocket(m_socket6);
}

bool NetLibraryImplV1::HasTimedOut()
{
	return (GetTickCount() - m_lastReceivedAt) > 15000;
}

void NetLibraryImplV1::Reset()
{
	m_outReliableAcknowledged = 0;
	m_outReliableSequence = 0;
	m_lastReceivedReliableCommand = 0;
	m_outReliableCommands.clear();

	m_lastFrameNumber = 0;
}

void NetLibraryImplV1::RunFrame()
{
	ProcessPackets();
	ProcessSend();
}

void NetLibraryImplV1::Flush()
{
	m_lastSend = 0;
	ProcessSend();

	m_lastSend = 0;
	ProcessSend();
}

void NetLibraryImplV1::SendReliableCommand(uint32_t type, const char* buffer, size_t length)
{
	// dirty hack
	if (type == 0xB3EA30DE) // msgIHost
	{
		m_iHost = std::string(buffer, length);
		return;
	}

	uint32_t unacknowledged = m_outReliableSequence - m_outReliableAcknowledged;

	if (unacknowledged > MAX_RELIABLE_COMMANDS)
	{
		static bool errored = false;

		if (!errored)
		{
			GlobalError("Reliable client command overflow.");

			errored = true;
		}

		return;
	}

	m_outReliableSequence++;

	OutReliableCommand cmd;
	cmd.type = type;
	cmd.id = m_outReliableSequence;
	cmd.command = std::string(buffer, length);

	m_outReliableCommands.push_back(cmd);
}

void NetLibraryImplV1::ProcessSend()
{
	// is it time to send a packet yet?
	bool continueSend = false;

	if (!continueSend)
	{
		uint32_t diff = timeGetTime() - m_lastSend;

		if (diff >= (1000 / 60))
		{
			continueSend = true;
		}
	}

	if (!continueSend)
	{
		return;
	}

	// do we have data to send?
	if (m_base->GetConnectionState() != NetLibrary::CS_ACTIVE)
	{
		return;
	}

	// metrics
	NetPacketMetrics metrics;

	// build a nice packet
	NetBuffer msg(24000);

	msg.Write(m_lastReceivedReliableCommand);

	if (m_base->GetServerProtocol() >= 2)
	{
		msg.Write(m_lastFrameNumber);
	}

	RoutingPacket packet;

	while (m_base->GetOutgoingPacket(packet))
	{
		msg.Write(0xE938445B); // msgRoute
		msg.Write(packet.netID);
		msg.Write<uint16_t>(packet.payload.size());

		//trace("sending msgRoute to %d len %d\n", packet.netID, packet.payload.size());

		msg.Write(packet.payload.c_str(), packet.payload.size());

		metrics.AddElementSize(NET_PACKET_SUB_ROUTED_MESSAGES, packet.payload.size() + 2 + 2 + 4);
	}

	// send pending reliable commands
	for (auto& command : m_outReliableCommands)
	{
		msg.Write(command.type);

		if (command.command.size() > UINT16_MAX)
		{
			msg.Write(command.id | 0x80000000);

			msg.Write<uint32_t>(command.command.size());

			metrics.AddElementSize(NET_PACKET_SUB_RELIABLES, 4);
		}
		else
		{
			msg.Write(command.id);

			msg.Write<uint16_t>(command.command.size());

			metrics.AddElementSize(NET_PACKET_SUB_RELIABLES, 2);
		}

		msg.Write(command.command.c_str(), command.command.size());

		metrics.AddElementSize(NET_PACKET_SUB_RELIABLES, command.command.size() + 8);
	}

	//OnBuildMessage(msg);
	NetLibrary::OnBuildMessage(std::bind(&NetLibraryImplV1::SendReliableCommand, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	if (!m_iHost.empty())
	{
		msg.Write<uint32_t>(0xB3EA30DE);
		msg.Write(m_iHost.c_str(), m_iHost.size());
	}

	msg.Write(0xCA569E63); // msgEnd

	m_netChannel.Send(msg);

	m_lastSend = timeGetTime();

	if (m_base->GetMetricSink())
	{
		m_base->GetMetricSink()->OnOutgoingPacket(metrics);
	}
}


void NetLibraryImplV1::ProcessPackets()
{
	ProcessPacketsInternal(NA_INET4);
	ProcessPacketsInternal(NA_INET6);
}

void NetLibraryImplV1::ProcessPacketsInternal(NetAddressType addrType)
{
	char buf[2048];
	memset(buf, 0, sizeof(buf));

	sockaddr_storage from;
	memset(&from, 0, sizeof(from));

	int fromlen = sizeof(from);

	auto socket = (addrType == NA_INET4) ? m_socket : m_socket6;

	while (true)
	{
		int len = recvfrom(socket, buf, 2048, 0, (sockaddr*)&from, &fromlen);

		NetAddress fromAddr((sockaddr*)&from);

		if (len == SOCKET_ERROR)
		{
			int error = WSAGetLastError();

			if (error != WSAEWOULDBLOCK)
			{
				trace("recv() failed - %d\n", error);
			}

			return;
		}

		if (*(int*)buf == -1)
		{
			char* oob = &buf[4];

			if (!_strnicmp(oob, "connectOK", 9))
			{
				char* clientNetIDStr = &oob[10];
				char* hostIDStr = strchr(clientNetIDStr, ' ');

				hostIDStr[0] = '\0';
				hostIDStr++;

				char* hostBaseStr = strchr(hostIDStr, ' ');

				hostBaseStr[0] = '\0';
				hostBaseStr++;

				m_lastReceivedReliableCommand = 0;

				m_base->HandleConnected(atoi(clientNetIDStr), atoi(hostIDStr), atoi(hostBaseStr));

				m_netChannel.Reset(m_base->GetCurrentServer(), this);
			}
			else
			{
				m_base->ProcessOOB(fromAddr, &buf[4], len - 4);
			}
		}
		else
		{
			if (fromAddr != m_base->GetCurrentServer())
			{
				trace("invalid from address for server msg\n");
				return;
			}

			NetBuffer* msg;

			if (m_netChannel.Process(buf, len, &msg))
			{
				ProcessServerMessage(*msg);

				delete msg;
			}
		}
	}
}

void NetLibraryImplV1::ProcessServerMessage(NetBuffer& msg)
{
	// update received-at time
	m_lastReceivedAt = GetTickCount();

	// metrics bits
	NetPacketMetrics metrics;

	// receive the message
	uint32_t msgType;

	uint32_t curReliableAck = msg.Read<uint32_t>();

	if (curReliableAck != m_outReliableAcknowledged)
	{
		for (auto it = m_outReliableCommands.begin(); it != m_outReliableCommands.end();)
		{
			if (it->id <= curReliableAck)
			{
				it = m_outReliableCommands.erase(it);
			}
			else
			{
				it++;
			}
		}

		m_outReliableAcknowledged = curReliableAck;
	}

	if (m_base->GetConnectionState() == NetLibrary::CS_CONNECTED)
	{
		m_base->SetConnectionState(NetLibrary::CS_ACTIVE);
	}

	if (m_base->GetConnectionState() != NetLibrary::CS_ACTIVE)
	{
		return;
	}

	do
	{
		if (msg.End())
		{
			break;
		}

		msgType = msg.Read<uint32_t>();

		if (msgType == 0xE938445B) // 'msgRoute'
		{
			uint16_t netID = msg.Read<uint16_t>();
			uint16_t rlength = msg.Read<uint16_t>();

			//trace("msgRoute from %d len %d\n", netID, rlength);

			char routeBuffer[65536];
			if (!msg.Read(routeBuffer, rlength))
			{
				break;
			}

			m_base->EnqueueRoutedPacket(netID, std::string(routeBuffer, rlength));

			// add to metrics
			metrics.AddElementSize(NET_PACKET_SUB_ROUTED_MESSAGES, 2 + rlength);
		}
		else if (msgType == 0x53FFFA3F) // msgFrame
		{
			// for now, frames are only an identifier - this will change once game features get moved to our code
			// (2014-10-15)

			uint32_t frameNum = msg.Read<uint32_t>();

			m_lastFrameNumber = frameNum;

			// handle ping status
			if (m_base->GetServerProtocol() >= 3)
			{
				int currentPing = msg.Read<int>();

				if (m_base->GetMetricSink())
				{
					m_base->GetMetricSink()->OnPingResult(currentPing);
				}
			}
		}
		else if (msgType != 0xCA569E63) // reliable command
		{
			uint32_t id = msg.Read<uint32_t>();
			uint32_t size;

			if (id & 0x80000000)
			{
				size = msg.Read<uint32_t>();
				id &= ~0x80000000;

				metrics.AddElementSize(NET_PACKET_SUB_RELIABLES, 4);
			}
			else
			{
				size = msg.Read<uint16_t>();

				metrics.AddElementSize(NET_PACKET_SUB_RELIABLES, 2);
			}

			// test for bad scenarios
			if (id > (m_lastReceivedReliableCommand + 64))
			{
				return;
			}

			char* reliableBuf = new(std::nothrow) char[size];

			if (!reliableBuf)
			{
				return;
			}

			if (!msg.Read(reliableBuf, size))
			{
				break;
			}

			// check to prevent double execution
			if (id > m_lastReceivedReliableCommand)
			{
				m_base->HandleReliableCommand(msgType, reliableBuf, size);

				m_lastReceivedReliableCommand = id;
			}

			delete[] reliableBuf;

			// add to metrics
			metrics.AddElementSize(NET_PACKET_SUB_RELIABLES, 4 + size);
		}
	} while (msgType != 0xCA569E63); // 'msgEnd'

	if (m_base->GetMetricSink())
	{
		m_base->GetMetricSink()->OnIncomingPacket(metrics);
	}
}


void NetLibraryImplV1::SendData(const NetAddress& netAddress, const char* data, size_t length)
{
	sockaddr_storage addr;
	int addrLen;
	netAddress.GetSockAddr(&addr, &addrLen);

	if (addr.ss_family == AF_INET)
	{
		sendto(m_socket, data, length, 0, (sockaddr*)&addr, addrLen);
	}
	else if (addr.ss_family == AF_INET6)
	{
		sendto(m_socket6, data, length, 0, (sockaddr*)&addr, addrLen);
	}
}

void NetLibraryImplV1::CreateResources()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		GlobalError("WSAStartup did not succeed.");
	}

	// create IPv4 socket
	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (m_socket == INVALID_SOCKET)
	{
		GlobalError("only one sock in pair");
	}

	// explicit bind
	sockaddr_in localAddr = { 0 };
	localAddr.sin_family = AF_INET;
	localAddr.sin_addr.s_addr = ADDR_ANY;
	localAddr.sin_port = 0;

	bind(m_socket, (sockaddr*)&localAddr, sizeof(localAddr));

	// non-blocking
	u_long arg = true;
	ioctlsocket(m_socket, FIONBIO, &arg);

	// create IPv6 socket
	m_socket6 = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

	if (m_socket6 != INVALID_SOCKET)
	{
		// bind the socket
		sockaddr_in6 ip6Addr = { 0 };
		ip6Addr.sin6_family = AF_INET6;
		ip6Addr.sin6_addr = in6addr_any;
		ip6Addr.sin6_port = 0;

		bind(m_socket6, (sockaddr*)&ip6Addr, sizeof(ip6Addr));

		// make non-blocking
		ioctlsocket(m_socket6, FIONBIO, &arg);
	}
}

void NetLibraryImplV1::SendConnect(const std::string& connectData)
{
	m_base->SendOutOfBand(m_base->GetCurrentServer(), "connect %s", connectData.c_str());
}

std::unique_ptr<NetLibraryImplBase> CreateNetLibraryImplV1(INetLibraryInherit* base)
{
	return std::make_unique<NetLibraryImplV1>(base);
}