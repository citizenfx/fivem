#include <StdInc.h>
#include <GameServerNet.h>

#include <GameServer.h>

#include <NetAddress.h>
#include <NetBuffer.h>

#include <boost/bimap.hpp>

#include <slikenet/BitStream.h>
#include <slikenet/peerinterface.h>
#include <slikenet/MessageIdentifiers.h>
#include <slikenet/socket2.h>

#ifndef _WIN32
#include <sys/select.h>
#endif

namespace fx
{
class GameServerNetImplRakNet;

class NetPeerImplRakNet : public NetPeerBase
{
public:
	NetPeerImplRakNet(GameServerNetImplRakNet* host, int handle)
	{
		m_handle = handle;
		m_host = host;
	}

private:
	const SLNet::RakNetGUID* GetPeer();

	SLNet::RakPeerInterface* GetHost();

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

		return GetHost()->GetLastPing(GetHost()->GetSystemAddressFromGuid(*peer));
	}

	virtual int GetPingVariance() override
	{
		auto peer = GetPeer();

		if (!peer)
		{
			return 0;
		}

		return abs(GetHost()->GetLowestPing(GetHost()->GetSystemAddressFromGuid(*peer)) - GetHost()->GetAveragePing(GetHost()->GetSystemAddressFromGuid(*peer)));
	}

	virtual net::PeerAddress GetAddress() override
	{
		auto peer = GetPeer();

		if (!peer)
		{
			return *net::PeerAddress::FromString("127.0.0.1", 30120, net::PeerAddress::LookupType::NoResolution);
		}

		auto rsa = GetHost()->GetSystemAddressFromGuid(*peer);

		return net::PeerAddress{
						(sockaddr*)&rsa.address,
						static_cast<socklen_t>(rsa.GetIPVersion() == 4 ?
							sizeof(sockaddr_in) :
							sizeof(sockaddr_in6))
		};
	}

	virtual void OnSendConnectOK() override
	{
		auto peer = GetPeer();

		if (!peer)
		{
			return;
		}

		auto host = GetHost();

#ifdef _DEBUG
		host->SetTimeoutTime(86400 * 1000, host->GetSystemAddressFromGuid(*peer));
#endif
	}

private:
	int m_handle;
	GameServerNetImplRakNet* m_host;
};

class GameServerNetImplRakNet : public GameServerNetBase
{
public:
	GameServerNetImplRakNet(GameServer* server)
		: m_server(server), m_basePeerId(0), m_peer(nullptr)
	{

	}

	virtual void Process() override
	{
		if (!m_peer)
		{
			return;
		}

		SLNet::Packet* packet = m_peer->Receive();

		while (packet)
		{
			switch (packet->data[0])
			{
			case ID_USER_PACKET_ENUM + 1:
			{
				auto peerId = m_peerHandles.right.find(packet->guid)->get_left();

				m_server->ProcessPacket(new NetPeerImplRakNet(this, peerId), packet->data + 1, packet->length - 1);
				break;
			}
			case ID_NEW_INCOMING_CONNECTION:
				m_peerHandles.left.insert({ ++m_basePeerId, packet->guid });
				break;
			case ID_CONNECTION_LOST:
			case ID_DISCONNECTION_NOTIFICATION:
				m_peerHandles.right.erase(packet->guid);
				break;
			}

			m_peer->DeallocatePacket(packet);
			packet = m_peer->Receive();
		}
	}

	virtual void Select(const std::vector<uintptr_t>& addFds, int timeout) override
	{
		std::vector<uint64_t> fds;

		if (m_peer)
		{
			DataStructures::List<SLNet::RakNetSocket2*> sockets;
			m_peer->GetSockets(sockets);

			for (int i = 0; i < sockets.Size(); i++)
			{
				if (sockets[i]->IsBerkleySocket())
				{
					fds.push_back(static_cast<uint64_t>(static_cast<SLNet::RNS2_Berkley*>(sockets[i])->GetSocket()));
				}
			}
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
		return new NetPeerImplRakNet(this, id);
	}

	virtual void ResetPeer(int id) override
	{
		m_peer->CloseConnection(m_peerHandles.left.find(id)->get_right(), true);
	}

	virtual void SendPacket(int peer, int channel, const net::Buffer & buffer, NetPacketType type) override
	{
		SLNet::BitStream bs;
		bs.Write((uint8_t)(ID_USER_PACKET_ENUM + 1));
		bs.Write(reinterpret_cast<const char*>(buffer.GetData().data()), buffer.GetCurOffset());
		
		m_peer->Send(&bs, HIGH_PRIORITY, (type == NetPacketType_Reliable || type == NetPacketType_ReliableReplayed) ? RELIABLE_ORDERED : UNRELIABLE, channel, m_peerHandles.left.find(peer)->get_right(), false);
	}

	virtual void SendOutOfBand(const net::PeerAddress& to, const std::string_view& oob_, bool prefix) override
	{
		auto rsa = SLNet::SystemAddress{
			to.GetHost().c_str(),
			uint16_t(to.GetPort())
		};

		DataStructures::List<SLNet::RakNetSocket2*> sockets;
		m_peer->GetSockets(sockets);

		std::string oob = (prefix ? "\xFF\xFF\xFF\xFF" : "") + std::string{ oob_ };

		for (int i = 0; i < sockets.Size(); i++)
		{
			SLNet::RNS2_SendParameters rsp;
			rsp.systemAddress = rsa;
			rsp.data = const_cast<char*>(oob.data());
			rsp.length = oob.size();

			sockets[i]->Send(&rsp, __FILE__, __LINE__);
		}
	}

	virtual void CreateUdpHost(const net::PeerAddress& address) override
	{
		m_peer = SLNet::RakPeerInterface::GetInstance();

		SLNet::SocketDescriptor socketInfo[2];
		socketInfo[0] = SLNet::SocketDescriptor{ uint16_t(address.GetPort()), address.GetHost().c_str() };
		socketInfo[1] = SLNet::SocketDescriptor{ uint16_t(address.GetPort()), address.GetHost().c_str() };
		socketInfo[1].socketFamily = AF_INET6;

		if (m_peer->Startup(MAX_CLIENTS + 10, socketInfo, sizeof(socketInfo) / sizeof(socketInfo[0])) == SLNet::SOCKET_PORT_ALREADY_IN_USE)
		{
			m_peer->Startup(MAX_CLIENTS + 10, socketInfo, 1);
		}

		m_peer->SetMaximumIncomingConnections(MAX_CLIENTS + 10);

		ms_instance = this;

		m_peer->SetIncomingDatagramEventHandler([](SLNet::RNS2RecvStruct* rs)
		{
			for (const auto& interceptor : ms_instance->m_interceptors)
			{
				if (interceptor(
					reinterpret_cast<uint8_t*>(rs->data),
					rs->bytesRead,
					net::PeerAddress{
						(sockaddr*)&rs->systemAddress.address,
						static_cast<socklen_t>(rs->systemAddress.GetIPVersion() == 4 ?
							sizeof(sockaddr_in) :
							sizeof(sockaddr_in6))
					}
				))
				{
					return false;
				}
			}

			return true;
		});
	}

	virtual void AddRawInterceptor(const std::function<bool(const uint8_t*, size_t, const net::PeerAddress&)>& interceptor) override
	{
		m_interceptors.push_back(interceptor);
	}

	virtual int GetClientVersion() override
	{
		return 4;
	}

private:
	friend class NetPeerImplRakNet;

	GameServer* m_server;

	static GameServerNetImplRakNet* ms_instance;

	SLNet::RakPeerInterface* m_peer;

	int m_basePeerId;

	boost::bimap<int, SLNet::RakNetGUID> m_peerHandles;

	std::vector<std::function<bool(const uint8_t *, size_t, const net::PeerAddress&)>> m_interceptors;
};

GameServerNetImplRakNet* GameServerNetImplRakNet::ms_instance;

const SLNet::RakNetGUID* NetPeerImplRakNet::GetPeer()
{
	auto it = m_host->m_peerHandles.left.find(m_handle);

	if (it == m_host->m_peerHandles.left.end())
	{
		return nullptr;
	}

	return &it->get_right();
}

SLNet::RakPeerInterface* NetPeerImplRakNet::GetHost()
{
	return m_host->m_peer;
}

fwRefContainer<GameServerNetBase> CreateGSNet_RakNet(fx::GameServer* server)
{
	return new GameServerNetImplRakNet(server);
}
}
