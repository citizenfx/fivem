#include "StdInc.h"

#include <ServerInstanceBase.h>
#include <HttpServerManager.h>

#include <ServerPerfComponent.h>

#include <prometheus/serializer.h>
#include <prometheus/text_serializer.h>

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->SetComponent(new fx::ServerPerfComponent());
	}, -1000);

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->GetComponent<fx::HttpServerManager>()->AddEndpoint("/perf/", [instance](const fwRefContainer<net::HttpRequest>& request, const fwRefContainer<net::HttpResponse>& response)
		{
			auto promRegistry = instance->GetComponent<fx::ServerPerfComponent>()->GetRegistry();
			auto metrics = promRegistry->Collect();

			auto serializer = std::unique_ptr<prometheus::Serializer>{ new prometheus::TextSerializer() };
			auto serialized = serializer->Serialize(metrics);

			response->SetHeader("Content-Type", "text/plain");
			response->End(std::move(serialized));
		});
	}, 1500);
});
