#include "StdInc.h"
#include <Client.h>

inline static uint64_t msec()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
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
}