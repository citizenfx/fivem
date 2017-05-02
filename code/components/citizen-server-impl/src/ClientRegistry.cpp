#include <StdInc.h>
#include <ClientRegistry.h>

#include <ServerInstanceBase.h>

namespace fx
{
	std::shared_ptr<Client> ClientRegistry::MakeClient(const std::string& guid)
	{
		auto client = std::make_shared<Client>(guid);
		m_clients[guid] = client;

		client->OnAssignNetId.Connect([=]()
		{
			m_clientsByNetId[client->GetNetId()] = client;
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