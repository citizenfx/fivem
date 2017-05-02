#include <StdInc.h>
#include <ClientRegistry.h>

#include <ServerInstanceBase.h>

namespace fx
{
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
}

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->SetComponent(new fx::ClientRegistry());
	}, -1000);
});