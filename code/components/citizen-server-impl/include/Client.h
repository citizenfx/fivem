#pragma once

#include <NetAddress.h>
#include <NetBuffer.h>

#include <enet/enet.h>

namespace {
	using namespace std::literals::chrono_literals;

	constexpr const auto CLIENT_DEAD_TIMEOUT = 15s;
}

namespace fx
{
	struct enet_peer_deleter
	{
		inline void operator()(ENetPeer* data)
		{
			enet_peer_reset(data);
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

		inline const net::PeerAddress& GetAddress()
		{
			return m_peerAddress;
		}

		inline ENetPeer* GetPeer()
		{
			return (m_peer) ? m_peer.get() : nullptr;
		}

		inline const std::string& GetName()
		{
			return m_name;
		}

		inline void SetName(const std::string& name)
		{
			m_name = name;
		}

		inline const std::string& GetConnectionToken()
		{
			return m_connectionToken;
		}

		inline void SetConnectionToken(const std::string& value)
		{
			m_connectionToken = value;
		}

		inline std::chrono::milliseconds GetLastSeen()
		{
			return m_lastSeen;
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

		// the client's nickname
		std::string m_name;

		// the client's ENet peer
		std::unique_ptr<ENetPeer, enet_peer_deleter> m_peer;
	};
}
