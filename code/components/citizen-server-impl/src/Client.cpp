#include "StdInc.h"
#include <Client.h>

using namespace std::literals::chrono_literals;

constexpr const auto CLIENT_DEAD_TIMEOUT = 15s;

inline static std::chrono::milliseconds msec()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
}

namespace fx
{
	Client::Client(const std::string& guid)
		: m_guid(guid), m_netId(-1), m_netBase(-1), m_lastSeen(0)
	{

	}

	void Client::SetPeer(ENetPeer* peer)
	{
		m_peer.reset(peer);

		OnAssignPeer();
	}

	void Client::SetNetBase(uint32_t netBase)
	{
		m_netBase = netBase;
	}

	void Client::SetNetId(uint16_t netId)
	{
		if (m_netId == 0xFFFF)
		{
			m_netId = netId;

			OnAssignNetId();
		}
	}

	void Client::Touch()
	{
		m_lastSeen = msec();
	}

	bool Client::IsDead()
	{
		return (msec() - m_lastSeen) > CLIENT_DEAD_TIMEOUT;
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