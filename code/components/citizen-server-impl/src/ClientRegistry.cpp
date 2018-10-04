#include <StdInc.h>
#include <ClientRegistry.h>

#include <ServerInstanceBase.h>
#include <ServerEventComponent.h>

#include <msgpack.hpp>

extern std::shared_ptr<ConVar<bool>> g_oneSyncVar;

namespace fx
{
	ClientRegistry::ClientRegistry()
		: m_hostNetId(-1), m_curNetId(1), m_clientsBySlotId(MAX_CLIENTS)
	{

	}

	std::shared_ptr<Client> ClientRegistry::MakeClient(const std::string& guid)
	{
		auto client = std::make_shared<Client>(guid);
		m_clients[guid] = client;

		std::weak_ptr<Client> weakClient(client);

		client->OnAssignNetId.Connect([=]()
		{
			m_clientsByNetId[weakClient.lock()->GetNetId()] = weakClient;
		});

		client->OnAssignPeer.Connect([=]()
		{
			m_clientsByPeer[weakClient.lock()->GetPeer()] = weakClient;

			if (!g_oneSyncVar->GetValue())
			{
				return;
			}

			for (int slot = m_clientsBySlotId.size() - 1; slot >= 0; slot--)
			{
				// 31 is a special case
				if (slot == 31)
				{
					continue;
				}

				if (m_clientsBySlotId[slot].expired())
				{
					client->SetSlotId(slot);

					m_clientsBySlotId[slot] = weakClient;

					break;
				}
			}
		});

		client->OnAssignTcpEndPoint.Connect([=]()
		{
			m_clientsByTcpEndPoint[weakClient.lock()->GetTcpEndPoint()] = weakClient;
		});

		client->OnAssignConnectionToken.Connect([=]()
		{
			m_clientsByConnectionToken[weakClient.lock()->GetConnectionToken()] = weakClient;
		});

		OnClientCreated(client.get());

		return client;
	}

	void ClientRegistry::HandleConnectingClient(const std::shared_ptr<Client>& client)
	{
		client->SetNetId(m_curNetId.fetch_add(1));
	}

	void ClientRegistry::HandleConnectedClient(const std::shared_ptr<Client>& client)
	{
		// for name handling, send player state
		fwRefContainer<ServerEventComponent> events = m_instance->GetComponent<ServerEventComponent>();

		// send every player information about the joining client
		events->TriggerClientEvent("onPlayerJoining", std::optional<std::string_view>(), client->GetNetId(), client->GetName(), client->GetSlotId());

		// send the JOINING CLIENT information about EVERY OTHER CLIENT
		std::string target = fmt::sprintf("%d", client->GetNetId());

		ForAllClients([&](const std::shared_ptr<fx::Client>& otherClient)
		{
			events->TriggerClientEvent("onPlayerJoining", target, otherClient->GetNetId(), otherClient->GetName(), otherClient->GetSlotId());
		});

		// trigger connection handlers
		OnConnectedClient(client.get());
	}

	std::shared_ptr<fx::Client> ClientRegistry::GetHost()
	{
		if (m_hostNetId == -1)
		{
			return nullptr;
		}

		return GetClientByNetID(m_hostNetId);
	}

	void ClientRegistry::SetHost(const std::shared_ptr<Client>& client)
	{
		if (!client)
		{
			m_hostNetId = -1;
		}
		else
		{
			if (!g_oneSyncVar->GetValue())
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
