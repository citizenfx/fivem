#include <StdInc.h>
#include <ClientRegistry.h>

#include <ServerInstanceBase.h>
#include <ServerEventComponent.h>

#include <msgpack.hpp>

namespace fx
{
	ClientRegistry::ClientRegistry()
		: m_hostNetId(-1), m_curNetId(1)
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
		});

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
		events->TriggerClientEvent("onPlayerJoining", std::optional<std::string_view>(), client->GetNetId(), client->GetName());

		// send the JOINING CLIENT information about EVERY OTHER CLIENT
		std::string target = fmt::sprintf("net:%d", client->GetNetId());

		ForAllClients([&](const std::shared_ptr<fx::Client>& otherClient)
		{
			events->TriggerClientEvent("onPlayerJoining", target, otherClient->GetNetId(), otherClient->GetName());
		});
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
			m_hostNetId = client->GetNetId();
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
