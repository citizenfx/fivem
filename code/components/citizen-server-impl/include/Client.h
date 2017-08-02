#pragma once

#include <NetAddress.h>
#include <NetBuffer.h>

#include <enet/enet.h>

#include <ComponentHolder.h>

#include <tbb/concurrent_unordered_map.h>

#include <any>

namespace {
	using namespace std::literals::chrono_literals;

	constexpr const auto CLIENT_DEAD_TIMEOUT = 10s;
}

#ifdef COMPILING_CITIZEN_SERVER_IMPL
#define SERVER_IMPL_EXPORT DLL_EXPORT
#else
#define SERVER_IMPL_EXPORT DLL_IMPORT
#endif

namespace fx
{
	struct enet_peer_deleter
	{
		inline void operator()(ENetPeer* data)
		{
			enet_peer_reset(data);
		}
	};

	class SERVER_IMPL_EXPORT Client : public ComponentHolderImpl<Client>
	{
	public:
		Client(const std::string& guid);

		void SetPeer(ENetPeer* peer);

		void SetNetId(uint32_t netId);

		void SetNetBase(uint32_t netBase);

		// updates the last-seen timer
		void Touch();

		bool IsDead();

		inline uint32_t GetNetId()
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

		inline const std::string& GetTcpEndPoint()
		{
			return m_tcpEndPoint;
		}

		void SetTcpEndPoint(const std::string& value);

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

		inline const std::vector<std::string>& GetIdentifiers()
		{
			return m_identifiers;
		}

		inline bool HasRouted()
		{
			return m_hasRouted;
		}

		inline void SetHasRouted()
		{
			m_hasRouted = true;
		}

		inline void AddIdentifier(const std::string& identifier)
		{
			m_identifiers.emplace_back(identifier);
		}

		const std::any& GetData(const std::string& key);

		void SetData(const std::string& key, const std::any& data);

		void SendPacket(int channel, const net::Buffer& buffer, ENetPacketFlag flags = (ENetPacketFlag)0);

		fwEvent<> OnAssignNetId;
		fwEvent<> OnAssignPeer;
		fwEvent<> OnAssignTcpEndPoint;

	private:
		// a temporary token for tying HTTP connections to UDP connections
		std::string m_connectionToken;

		// the client's UDP peer address
		net::PeerAddress m_peerAddress;

		// when the client was last seen
		std::chrono::milliseconds m_lastSeen;

		// the client's primary GUID
		std::string m_guid;

		// the client's identifiers
		std::vector<std::string> m_identifiers;

		// the client's netid
		uint32_t m_netId;

		// the client's netbase
		uint32_t m_netBase;

		// the client's nickname
		std::string m_name;

		// the client's remote endpoint used for HTTP
		std::string m_tcpEndPoint;

		// the client's ENet peer
		std::unique_ptr<ENetPeer, enet_peer_deleter> m_peer;

		// whether the client has sent a routing msg once
		bool m_hasRouted;

		// an arbitrary set of data
		tbb::concurrent_unordered_map<std::string, std::any> m_userData;
	};
}
