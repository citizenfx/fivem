#include "StdInc.h"
#include <ClientHttpHandler.h>

#include <ClientRegistry.h>
#include <GameServer.h>

#include <UvTcpServer.h>
#include <TcpServerManager.h>

#include <ServerInstanceBase.h>

template<typename T>
class UvHandleContainer
{
public:
	UvHandleContainer()
	{
		m_handle = std::make_unique<T>();
	}

	~UvHandleContainer()
	{
		if (m_handle)
		{
			UvClose(std::move(m_handle));
		}
	}

	inline T* get()
	{
		return m_handle.get();
	}

private:
	std::unique_ptr<T> m_handle;
};

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();
		auto gameServer = instance->GetComponent<fx::GameServer>();

		instance->GetComponent<fx::ClientMethodRegistry>()->AddHandler("getDeferState", [=](const std::map<std::string, std::string>& postMap, const fwRefContainer<net::HttpRequest>& request, const std::function<void(const json&)>& cb)
		{
			cb(json::object({ { "error", "This server does not support legacy deferrals." } }));
			return;
		});
	}, 5000);
});
