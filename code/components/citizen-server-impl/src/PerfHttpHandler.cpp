#include "StdInc.h"

#include <ServerInstanceBase.h>
#include <HttpServerManager.h>

#include <ServerPerfComponent.h>

#include <TcpListenManager.h>
#include <KeyedRateLimiter.h>

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
		instance->GetComponent<fx::HttpServerManager>()->AddEndpoint("/perf/", [instance](const fwRefContainer<net::HttpRequest>& request, fwRefContainer<net::HttpResponse> response)
		{
			static auto limiter = instance->GetComponent<fx::PeerAddressRateLimiterStore>()->GetRateLimiter("http_perf", fx::RateLimiterDefaults{ 2.0, 5.0 });
			auto address = request->GetRemotePeer();

			bool cooldown = false;

			if (!fx::IsProxyAddress(address) && !limiter->Consume(address, 1.0, &cooldown))
			{
				if (cooldown)
				{
					response->CloseSocket();
					return;
				}

				response->SetStatusCode(429);
				response->End("Rate limit exceeded.");
				return;
			}

			auto promRegistry = instance->GetComponent<fx::ServerPerfComponent>()->GetRegistry();
			auto metrics = promRegistry->Collect();

			auto serializer = std::unique_ptr<prometheus::Serializer>{ new prometheus::TextSerializer() };
			auto serialized = serializer->Serialize(metrics);

			response->SetHeader("Content-Type", "text/plain");
			response->End(std::move(serialized));
		});
	}, 1500);
});
