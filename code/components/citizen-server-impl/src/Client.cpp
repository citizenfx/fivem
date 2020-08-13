#include "StdInc.h"
#include <Client.h>
#include <GameServer.h>

namespace fx
{
	Client::Client(const std::string& guid)
		: m_guid(guid), m_netId(0xFFFF), m_netBase(-1), m_lastSeen(0), m_hasRouted(false), m_slotId(-1)
	{

	}

	void Client::SetPeer(int peer, const net::PeerAddress& peerAddress)
	{
		m_peer.reset(new int(peer));
		m_peerAddress = peerAddress;

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

		UpdateCachedPrincipalValues();
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
			static const std::any& emptyAny = *new std::any();
			return emptyAny;
		}

		return it->second;
	}

	void Client::SendPacket(int channel, const net::Buffer& buffer, NetPacketType type /* = (ENetPacketFlag)0 */)
	{
		if (m_peer)
		{
			gscomms_send_packet(this, *m_peer, channel, buffer, type);
		}
	}
}
