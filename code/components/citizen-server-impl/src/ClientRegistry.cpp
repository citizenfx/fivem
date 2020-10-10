#include <StdInc.h>
#include <ClientRegistry.h>

#include <GameServer.h>

#include <ServerInstanceBase.h>
#include <ServerEventComponent.h>

#include <msgpack.hpp>

#include <ResourceManagerImpl.h>
#include <ResourceEventComponent.h>

namespace fx
{
	extern bool IsOneSync();

	ClientRegistry::ClientRegistry()
		: m_hostNetId(-1), m_curNetId(1), m_instance(nullptr)
	{
		if (fx::IsBigMode())
		{
			m_clientsBySlotId.resize(MAX_CLIENTS);
		}
		else
		{
			m_clientsBySlotId.resize(129);
		}
	}

	fx::ClientSharedPtr ClientRegistry::MakeClient(const std::string& guid)
	{
		fx::ClientSharedPtr client = fx::ClientSharedPtr::Construct(guid);
		fx::ClientWeakPtr weakClient(client);

		m_clients.emplace(guid, client);
		client->OnAssignNetId.Connect([this, weakClient]()
		{
			m_clientsByNetId[weakClient.lock()->GetNetId()] = weakClient;
		});

		client->OnAssignPeer.Connect([this, weakClient]()
		{
			auto client = weakClient.lock();

			if (!client)
			{
				return;
			}

			m_clientsByPeer[client->GetPeer()] = weakClient;

			if (!IsOneSync())
			{
				return;
			}

			// reconnecting clients will assign a peer again, but should *not* be assigned a new slot ID
			if (client->GetSlotId() != -1)
			{
				return;
			}

			std::lock_guard clientGuard(m_clientSlotMutex);
			for (int slot = m_clientsBySlotId.size() - 1; slot >= 0; slot--)
			{
				// 31 is a special case
				if (slot == 31)
				{
					continue;
				}

				if (!m_clientsBySlotId[slot])
				{
					client->SetSlotId(slot);
					m_clientsBySlotId[slot] = weakClient;
					break;
				}
			}
		});

		client->OnAssignTcpEndPoint.Connect([this, weakClient]()
		{
			m_clientsByTcpEndPoint[weakClient.lock()->GetTcpEndPoint()] = weakClient;
		});

		client->OnAssignConnectionToken.Connect([this, weakClient]()
		{
			m_clientsByConnectionToken[weakClient.lock()->GetConnectionToken()] = weakClient;
		});

		OnClientCreated(client);

		return client;
	}

	void ClientRegistry::HandleConnectingClient(const fx::ClientSharedPtr& client)
	{
		client->SetNetId(m_curNetId.fetch_add(1));
	}

	void ClientRegistry::HandleConnectedClient(const fx::ClientSharedPtr& client, uint32_t oldNetID)
	{
		auto eventManager = m_instance->GetComponent<fx::ResourceManager>()->GetComponent<fx::ResourceEventManagerComponent>();


		/*NETEV playerJoining SERVER
		/#*
		 * A server-side event that is triggered when a player has a finally-assigned NetID.
		 *
		 * @param source - The player's NetID (a number in Lua/JS), **not a real argument, use [FromSource] or source**.
		 * @param oldID - The original TempID for the connecting player, as specified during playerConnecting.
		 #/
		declare function playerJoining(source: string, oldID: string): void;
		*/
		eventManager->TriggerEvent2("playerJoining", { fmt::sprintf("net:%d", client->GetNetId()) }, fmt::sprintf("%d", oldNetID));

		if (!fx::IsBigMode())
		{
			// for name handling, send player state
			fwRefContainer<ServerEventComponent> events = m_instance->GetComponent<ServerEventComponent>();

			// send every player information about the joining client
			events->TriggerClientEventReplayed("onPlayerJoining", std::optional<std::string_view>(), client->GetNetId(), client->GetName(), client->GetSlotId());

			// send the JOINING CLIENT information about EVERY OTHER CLIENT
			std::string target = fmt::sprintf("%d", client->GetNetId());

			ForAllClients([&](const fx::ClientSharedPtr& otherClient)
			{
				events->TriggerClientEventReplayed("onPlayerJoining", target, otherClient->GetNetId(), otherClient->GetName(), otherClient->GetSlotId());
			});
		}
		else
		{
			fwRefContainer<ServerEventComponent> events = m_instance->GetComponent<ServerEventComponent>();

			std::string target = fmt::sprintf("%d", client->GetNetId());

			events->TriggerClientEventReplayed("onPlayerJoining", target, client->GetNetId(), client->GetName(), 128);
		}

		// trigger connection handlers
		OnConnectedClient(client.get());
	}

	fx::ClientSharedPtr ClientRegistry::GetHost()
	{
		if (m_hostNetId == 0xFFFF)
		{
			return fx::ClientSharedPtr{};
		}

		return GetClientByNetID(m_hostNetId);
	}

	void ClientRegistry::SetHost(const fx::ClientSharedPtr& client)
	{
		if (!client)
		{
			m_hostNetId = -1;
		}
		else
		{
			if (!IsOneSync())
			{
				m_hostNetId = client->GetNetId();
			}
		}
	}

	void ClientRegistry::AttachToObject(ServerInstanceBase* instance)
	{
		m_instance = instance;
	}
}

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->SetComponent(new fx::ClientRegistry());
	}, -1000);
});
