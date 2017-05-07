#pragma once

#include <NetAddress.h>
#include <NetBuffer.h>

#include <enet/enet.h>

namespace fx
{
	// TODO(fxserver): consolidate
	template<void* Fn>
	struct enet_deleter2
	{
		template<typename T>
		void operator()(T* data)
		{
			((void(*)(T*))Fn)(data);
		}
	};

	class Client
	{
	public:
		Client(const std::string& guid);

		void SetPeer(ENetPeer* peer);

		void SetNetId(uint16_t netId);

		void SetNetBase(uint32_t netBase);

		// updates the last-seen timer
		void Touch();

		bool IsDead();

		inline uint16_t GetNetId()
		{
			return m_netId;
		}

		inline uint32_t GetNetBase()
		{
			return m_netBase;
		}

		inline const std::string& GetGuid()
		{
			return m_guid;
		}

		inline ENetPeer* GetPeer()
		{
			return (m_peer) ? m_peer.get() : nullptr;
		}

		void SendPacket(int channel, const net::Buffer& buffer, ENetPacketFlag flags = (ENetPacketFlag)0);

		fwEvent<> OnAssignNetId;
		fwEvent<> OnAssignPeer;

	private:
		// a temporary token for tying HTTP connections to UDP connections
		std::string m_connectionToken;

		// the client's UDP peer address
		net::PeerAddress m_peerAddress;

		// when the client was last seen
		std::chrono::milliseconds m_lastSeen;

		// the client's primary GUID
		std::string m_guid;

		// the client's netid
		uint16_t m_netId;

		// the client's netbase
		uint32_t m_netBase;

		// the client's ENet peer
		std::unique_ptr<ENetPeer, enet_deleter2<&enet_peer_reset>> m_peer;
	};
}