#include <StdInc.h>
#include <ClientRegistry.h>

#include <ServerInstanceBase.h>

namespace fx
{
	ClientRegistry::ClientRegistry()
		: m_hostNetId(-1)
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
		m_hostNetId = client->GetNetId();
	}
}

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->SetComponent(new fx::ClientRegistry());
	}, -1000);
});