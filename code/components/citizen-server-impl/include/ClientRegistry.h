#pragma once

#include <shared_mutex>

#include <Client.h>
#include <ComponentHolder.h>

#include <tbb/concurrent_unordered_map.h>

namespace tbb
{
	namespace interface5
	{
		template<>
		inline size_t tbb_hasher(const net::PeerAddress& addr)
		{
			return std::hash<std::string_view>()(std::string_view{ (const char*)addr.GetSocketAddress(), (size_t)addr.GetSocketAddressLength() });
		}
	}
}

namespace fx
{
	class ServerInstanceBase;

	class ClientRegistry : public fwRefCountable, public IAttached<ServerInstanceBase>
	{
	public:
		ClientRegistry();

		// invoked upon receiving the `connect` ENet packet
		void HandleConnectingClient(const std::shared_ptr<Client>& client);

		// invoked upon receiving the `connect` ENet packet, after sending `connectOK`
		void HandleConnectedClient(const std::shared_ptr<Client>& client);

		std::shared_ptr<Client> MakeClient(const std::string& guid);

		inline void RemoveClient(const std::shared_ptr<Client>& client)
		{
			m_clientsByPeer[client->GetPeer()].reset();
			m_clientsByNetId[client->GetNetId()].reset();
			m_clientsByConnectionToken[client->GetConnectionToken()].reset();

			if (client->GetSlotId() >= 0 && client->GetSlotId() < m_clientsBySlotId.size())
			{
				m_clientsBySlotId[client->GetSlotId()].reset();
			}

			{
				std::unique_lock<std::shared_mutex> lock(m_clientsMutex);
				m_clients[client->GetGuid()] = nullptr;
			}

			// unassign slot ID
			client->SetSlotId(-1);
		}

		inline std::shared_ptr<Client> GetClientByGuid(const std::string& guid)
		{
			std::shared_lock<std::shared_mutex> lock(m_clientsMutex);

			auto ptr = std::shared_ptr<Client>();
			auto it = m_clients.find(guid);

			if (it != m_clients.end())
			{
				ptr = it->second;
			}

			return ptr;
		}

		inline std::shared_ptr<Client> GetClientByPeer(int peer)
		{
			auto ptr = std::shared_ptr<Client>();
			auto it = m_clientsByPeer.find(peer);

			if (it != m_clientsByPeer.end())
			{
				if (!it->second.expired())
				{
					ptr = it->second.lock();
				}
			}

			return ptr;
		}

		inline std::shared_ptr<Client> GetClientByEndPoint(const net::PeerAddress& address)
		{
			auto ptr = std::shared_ptr<Client>();
			auto it = m_clientsByEndPoint.find(address);

			if (it != m_clientsByEndPoint.end())
			{
				if (!it->second.expired())
				{
					ptr = it->second.lock();
				}
			}

			return ptr;
		}

		inline std::shared_ptr<Client> GetClientByTcpEndPoint(const std::string& address)
		{
			auto ptr = std::shared_ptr<Client>();
			auto it = m_clientsByTcpEndPoint.find(address);

			if (it != m_clientsByTcpEndPoint.end())
			{
				if (!it->second.expired())
				{
					ptr = it->second.lock();
				}
			}

			return ptr;
		}

		inline std::shared_ptr<Client> GetClientByNetID(uint32_t netId)
		{
			auto ptr = std::shared_ptr<Client>();
			auto it = m_clientsByNetId.find(netId);

			if (it != m_clientsByNetId.end())
			{
				if (!it->second.expired())
				{
					ptr = it->second.lock();
				}
			}

			return ptr;
		}

		inline std::shared_ptr<Client> GetClientBySlotID(uint32_t slotId)
		{
			assert(slotId < m_clientsBySlotId.size());

			auto weakPtr = m_clientsBySlotId[slotId];
			return weakPtr.lock();
		}

		inline std::shared_ptr<Client> GetClientByConnectionToken(const std::string& token)
		{
			auto ptr = std::shared_ptr<Client>();
			auto it = m_clientsByConnectionToken.find(token);

			if (it != m_clientsByConnectionToken.end())
			{
				if (!it->second.expired())
				{
					ptr = it->second.lock();
				}
			}

			return ptr;
		}

		template<typename TFn>
		inline void ForAllClients(TFn&& cb)
		{
			m_clientsMutex.lock_shared();

			for (auto& client : m_clients)
			{
				auto cl = client.second;

				m_clientsMutex.unlock_shared();

				if (cl)
				{
					cb(cl);
				}

				m_clientsMutex.lock_shared();
			}

			m_clientsMutex.unlock_shared();
		}

		std::shared_ptr<Client> GetHost();

		void SetHost(const std::shared_ptr<Client>& client);

		virtual void AttachToObject(ServerInstanceBase* instance) override;

		fwEvent<Client*> OnClientCreated;

		fwEvent<Client*> OnConnectedClient;

	private:
		uint16_t m_hostNetId;

		tbb::concurrent_unordered_map<std::string, std::shared_ptr<Client>> m_clients;

		// aliases for fast lookup
		tbb::concurrent_unordered_map<uint32_t, std::weak_ptr<Client>> m_clientsByNetId;
		tbb::concurrent_unordered_map<net::PeerAddress, std::weak_ptr<Client>> m_clientsByEndPoint;
		tbb::concurrent_unordered_map<std::string, std::weak_ptr<Client>> m_clientsByTcpEndPoint;
		tbb::concurrent_unordered_map<std::string, std::weak_ptr<Client>> m_clientsByConnectionToken;
		tbb::concurrent_unordered_map<int, std::weak_ptr<Client>> m_clientsByPeer;

		// pending C++20 std::atomic overloads, again
		std::shared_mutex m_clientsMutex;

		std::vector<std::weak_ptr<Client>> m_clientsBySlotId;

		std::atomic<uint16_t> m_curNetId;

		ServerInstanceBase* m_instance;
	};
}

DECLARE_INSTANCE_TYPE(fx::ClientRegistry);
