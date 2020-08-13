#pragma once

#include <NetAddress.h>
#include <NetBuffer.h>

#include <GameServerComms.h>

#include <ComponentHolder.h>

#include <EASTL/fixed_list.h>

#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_queue.h>

#include <any>

#include <se/Security.h>

#include <citizen_util/object_pool.h>
#include <citizen_util/shared_reference.h>

#include <shared_mutex>

#define MAX_CLIENTS (1024 + 1) // don't change this past 256 ever, also needs to be synced with client code

namespace {
	using namespace std::literals::chrono_literals;

#ifdef _DEBUG
	constexpr const auto CLIENT_DEAD_TIMEOUT = 86400s;
	constexpr const auto CLIENT_VERY_DEAD_TIMEOUT = 86400s;
#else
	constexpr const auto CLIENT_DEAD_TIMEOUT = 60s;
	constexpr const auto CLIENT_VERY_DEAD_TIMEOUT = 120s;
#endif
}

#ifdef COMPILING_CITIZEN_SERVER_IMPL
#define SERVER_IMPL_EXPORT DLL_EXPORT
#else
#define SERVER_IMPL_EXPORT DLL_IMPORT
#endif

namespace fx
{
	namespace sync
	{
		class ClientSyncDataBase
		{
		public:
			virtual ~ClientSyncDataBase() = default;
		};
	}

	struct gs_peer_deleter
	{
		inline void operator()(int* data)
		{
			gscomms_execute_callback_on_net_thread([=]()
			{
				gscomms_reset_peer(*data);
				delete data;
			});
		}
	};


	class SERVER_IMPL_EXPORT Client : public ComponentHolderImpl<Client>, public se::PrincipalSource
	{
	public:
		Client(const std::string& guid);

		void SetPeer(int peer, const net::PeerAddress& peerAddress);

		void SetNetId(uint32_t netId);

		void SetNetBase(uint32_t netBase);

		// updates the last-seen timer
		void Touch();

		bool IsDead();

		inline uint32_t GetNetId()
		{
			return m_netId;
		}

		inline uint32_t GetSlotId()
		{
			return m_slotId;
		}

		inline void SetSlotId(uint32_t slotId)
		{
			m_slotId = slotId;
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

		inline int GetPeer()
		{
			return (m_peer) ? *m_peer.get() : 0;
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

			OnAssignConnectionToken();
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

			UpdateCachedPrincipalValues();
		}

		inline auto EnterPrincipalScope()
		{
			auto principal = std::make_unique<se::ScopedPrincipal>(this);

			return std::move(principal);
		}

		inline void GetPrincipals(const std::function<bool(const se::Principal&)>& iterator)
		{
			for (auto& principal : m_principals)
			{
				if (iterator(principal))
				{
					break;
				}
			}
		}

		inline std::shared_ptr<sync::ClientSyncDataBase> GetSyncData()
		{
			return m_syncData;
		}

		inline void SetSyncData(const std::shared_ptr<sync::ClientSyncDataBase>& ptr)
		{
			m_syncData = ptr;
		}

		inline void PushReplayPacket(int channel, const net::Buffer& buffer)
		{
			m_replayQueue.push({ buffer, channel });
		}

		inline void ReplayPackets()
		{
			std::tuple<net::Buffer, int> value;

			while (m_replayQueue.try_pop(value))
			{
				const auto&[buffer, channel] = value;

				SendPacket(channel, buffer, NetPacketType_Reliable);
			}
		}

		const std::any& GetData(const std::string& key);

		void SetData(const std::string& key, const std::any& data);

		void SendPacket(int channel, const net::Buffer& buffer, NetPacketType flags = NetPacketType_Unreliable);

		fwEvent<> OnAssignNetId;
		fwEvent<> OnAssignPeer;
		fwEvent<> OnAssignTcpEndPoint;
		fwEvent<> OnAssignConnectionToken;

		fwEvent<> OnCreatePed;

		fwEvent<> OnDrop;

	private:
		inline void UpdateCachedPrincipalValues()
		{
			m_principals = {};

			for (auto& identifier : this->GetIdentifiers())
			{
				m_principals.emplace_back(se::Principal{ fmt::sprintf("identifier.%s", identifier) });
			}

			m_principals.emplace_back(se::Principal{ fmt::sprintf("player.%d", m_netId) });
		}

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

		// the client's slot ID
		uint32_t m_slotId;

		// the client's netbase
		uint32_t m_netBase;

		// the client's nickname
		std::string m_name;

		// the client's remote endpoint used for HTTP
		std::string m_tcpEndPoint;

		// the client's ENet peer
		std::unique_ptr<int, gs_peer_deleter> m_peer;

		// sync data
		std::shared_ptr<sync::ClientSyncDataBase> m_syncData;

		// whether the client has sent a routing msg once
		bool m_hasRouted;

		// packets to resend when a new peer connects using this client
		tbb::concurrent_queue<std::tuple<net::Buffer, int>> m_replayQueue;

		// an arbitrary set of data
		tbb::concurrent_unordered_map<std::string, std::any> m_userData;

		// principal values
		std::list<se::Principal> m_principals;
	};

	inline object_pool<Client, 512 * 1024> clientPool;

	using ClientSharedPtr = shared_reference<Client, &clientPool>;
	using ClientWeakPtr = weak_reference<ClientSharedPtr>;
}
