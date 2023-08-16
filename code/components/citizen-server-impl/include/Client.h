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

#include <boost/type_index.hpp>

#include <shared_mutex>

constexpr auto MAX_CLIENTS = (2048 + 1);

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
	// here temporarily as mostly used for GetData
	class SERVER_IMPL_EXPORT AnyBase
	{
	public:
		virtual ~AnyBase() = default;

		virtual uint32_t GetType() = 0;
	};

	template<typename T>
	class AnyHolder : public AnyBase
	{
	public:
		explicit AnyHolder(const T& data)
			: m_data(data)
		{

		}

		virtual uint32_t GetType() override
		{
			return HashString(boost::typeindex::type_id<T>().raw_name());
		}

		inline T& GetData()
		{
			return m_data;
		}

		inline const T& GetData() const
		{
			return m_data;
		}

	private:
		T m_data;
	};

	template<typename T>
	auto MakeAny(const T& data)
	{
		return std::make_shared<AnyHolder<std::remove_reference_t<std::remove_const_t<T>>>>(data);
	}

	template<typename T>
	T& AnyCast(const std::shared_ptr<AnyBase>& base)
	{
		if (!base || base->GetType() != HashString(boost::typeindex::type_id<T>().raw_name()))
		{
			throw std::bad_any_cast();
		}

		auto entry = std::static_pointer_cast<AnyHolder<T>>(base);
		return entry->GetData();
	}

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
		inline const std::string& GetTcpEndPoint1()
		{
			return "123";
		}

		void SetTcpEndPoint1(const std::string& value);

		inline const std::string& GetConnectionToken()
		{
			return m_connectionToken;
		}

		inline void SetConnectionToken(const std::string& value)
		{
			m_connectionToken = value;

			OnAssignConnectionToken();
		}

		inline std::chrono::milliseconds GetFirstSeen()
		{
			return m_firstSeen;
		}

		inline std::chrono::milliseconds GetLastSeen()
		{
			return m_lastSeen;
		}

		inline const std::vector<std::string>& GetIdentifiers()
		{
			return m_identifiers;
		}

		inline const std::vector<std::string>& GetTokens()
		{
			return m_tokens;
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

		inline void AddToken(const std::string& token)
		{
			m_tokens.emplace_back(token);
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
			std::shared_lock _(m_syncDataMutex);
			return m_syncData;
		}

		inline void SetSyncData(const std::shared_ptr<sync::ClientSyncDataBase>& ptr)
		{
			std::unique_lock _(m_syncDataMutex);
			m_syncData = ptr;
		}

		// to be called from CreateSyncData
		inline auto AcquireSyncDataCreationLock()
		{
			return std::unique_lock(m_syncDataCreationMutex);
		}

		inline bool IsDropping() const
		{
			return m_dropping;
		}

		inline void SetDropping()
		{
			m_dropping = true;
		}

		inline auto GetNetworkMetricsSendCallback()
		{
			return m_clientNetworkMetricsSendCallback;
		}

		inline void SetNetworkMetricsSendCallback(void (*callback)(Client *thisptr, int channel, const net::Buffer& buffer, NetPacketType flags))
		{
			m_clientNetworkMetricsSendCallback = callback;
		}

		inline auto GetNetworkMetricsRecvCallback()
		{
			return m_clientNetworkMetricsRecvCallback;
		}

		inline void SetNetworkMetricsRecvCallback(void(*callback)(Client *thisptr, uint32_t packetId, net::Buffer& packet))
		{
			m_clientNetworkMetricsRecvCallback = callback;
		}

		int GetPing();

		std::shared_ptr<AnyBase> GetData(const std::string& key);

		void SetDataRaw(const std::string& key, const std::shared_ptr<AnyBase>& data);

		template<typename TAny>
		inline void SetData(const std::string& key, const TAny& data)
		{
			if constexpr (std::is_same_v<TAny, std::nullptr_t>)
			{
				SetDataRaw(key, {});
				return;
			}

			SetDataRaw(key, MakeAny(data));
		}

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

		// when client was first seen (time since Epoch)
		std::chrono::milliseconds m_firstSeen;

		// when the client was last seen (time since Epoch)
		std::chrono::milliseconds m_lastSeen;

		// the client's primary GUID
		std::string m_guid;

		// the client's identifiers
		std::vector<std::string> m_identifiers;

		// the client's tokens
		std::vector<std::string> m_tokens;

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
		std::shared_mutex m_syncDataMutex;
		std::mutex m_syncDataCreationMutex;

		// whether the client has sent a routing msg once
		bool m_hasRouted;

		// an arbitrary set of data
		std::shared_mutex m_userDataMutex;
		std::unordered_map<std::string, std::shared_ptr<AnyBase>> m_userData;

		// principal values
		std::list<se::Principal> m_principals;

		// whether the client is currently being dropped
		volatile bool m_dropping;

		void (*m_clientNetworkMetricsSendCallback)(Client *thisptr, int channel, const net::Buffer& buffer, NetPacketType flags);

		void (*m_clientNetworkMetricsRecvCallback)(Client *thisptr, uint32_t packetId, net::Buffer& packet);
	};

	extern SERVER_IMPL_EXPORT object_pool<Client, 512 * MAX_CLIENTS> clientPool;

	using ClientSharedPtr = shared_reference<Client, &clientPool>;
	using ClientWeakPtr = weak_reference<ClientSharedPtr>;
}
