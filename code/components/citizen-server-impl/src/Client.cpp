#include "StdInc.h"
#include <Client.h>
#include <GameServer.h>

namespace fx
{
	Client::Client(const std::string& guid)
		: m_guid(guid), m_netId(0xFFFF), m_netBase(-1), m_lastSeen(0), m_hasRouted(false)
	{

	}

	void Client::SetPeer(ENetPeer* peer)
	{
		m_peer.reset(peer);
		m_peerAddress = GetPeerAddress(peer->address);

		OnAssignPeer();
	}

	void Client::SetNetBase(uint32_t netBase)
	{
		m_netBase = netBase;
	}

	void Client::SetNetId(uint32_t netId)
	{
		m_netId = netId;

		OnAssignNetId();
	}

	void Client::SetTcpEndPoint(const std::string& value)
	{
		m_tcpEndPoint = value;

		OnAssignTcpEndPoint();
	}

	void Client::Touch()
	{
		m_lastSeen = msec();
	}

	bool Client::IsDead()
	{
		// if we've not connected yet, we can't be dead
		if (m_netId >= 0xFFFF)
		{
			auto canBeDead = GetData("canBeDead");

			if (!canBeDead.has_value() || !std::any_cast<bool>(canBeDead))
			{
				return (msec() - m_lastSeen) > CLIENT_VERY_DEAD_TIMEOUT;
			}
		}

		return (msec() - m_lastSeen) > CLIENT_DEAD_TIMEOUT;
	}

	void Client::SetData(const std::string& key, const std::any& data)
	{
		m_userData[key] = data;
	}

	const std::any& Client::GetData(const std::string& key)
	{
		auto it = m_userData.find(key);

		if (it == m_userData.end())
		{
			static const std::any emptyAny;
			return emptyAny;
		}

		return it->second;
	}

	void Client::SendPacket(int channel, const net::Buffer& buffer, ENetPacketFlag flags /* = (ENetPacketFlag)0 */)
	{
		if (m_peer)
		{
			auto packet = enet_packet_create(buffer.GetBuffer(), buffer.GetLength(), flags);
			enet_peer_send(m_peer.get(), channel, packet);
		}
	}
}
