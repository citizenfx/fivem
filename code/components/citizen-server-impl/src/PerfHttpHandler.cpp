#include "StdInc.h"

#include <ServerInstanceBase.h>
#include <HttpServerManager.h>

#include <ServerPerfComponent.h>

#include <TcpListenManager.h>
#include <KeyedRateLimiter.h>

#include <prometheus/serializer.h>
#include <prometheus/text_serializer.h>
#include <skyr/v1/url_search_parameters.hpp>

#include <botan/base64.h>

namespace
{
std::shared_ptr<ConVar<std::string>> g_prometheusBasicAuthUser;
std::shared_ptr<ConVar<std::string>> g_prometheusBasicAuthPassword;
std::string g_prometheusAuthHash {};

void GenerateAuthHash(const std::string& user, const std::string& password)
{
	if (user.empty() && password.empty())
	{
		g_prometheusAuthHash.clear();
		return;
	}

	std::string authString = user + ":" + password;
	g_prometheusAuthHash = "Basic " + Botan::base64_encode(reinterpret_cast<const uint8_t*>(authString.data()), authString.size());
}

void OnPrometheusBasicAuthUserChanged(internal::ConsoleVariableEntry<std::string>* variableEntry)
{
	std::string user = variableEntry->GetValue();
	std::string password = g_prometheusBasicAuthPassword->GetValue();

	GenerateAuthHash(user, password);
}

void OnPrometheusBasicAuthPasswordChanged(internal::ConsoleVariableEntry<std::string>* variableEntry)
{
	std::string user = g_prometheusBasicAuthUser->GetValue();
	std::string password = variableEntry->GetValue();

	GenerateAuthHash(user, password);
}
}

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->SetComponent(new fx::ServerPerfComponent());
	}, -1000);

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		g_prometheusBasicAuthUser = instance->AddVariable<std::string>("sv_prometheusBasicAuthUser", ConVar_None, "", OnPrometheusBasicAuthUserChanged);
		g_prometheusBasicAuthPassword = instance->AddVariable<std::string>("sv_prometheusBasicAuthPassword", ConVar_None, "", OnPrometheusBasicAuthPasswordChanged);
		GenerateAuthHash(g_prometheusBasicAuthUser->GetValue(), g_prometheusBasicAuthPassword->GetValue());

		instance->GetComponent<fx::HttpServerManager>()->AddEndpoint("/perf", [instance](const fwRefContainer<net::HttpRequest>& request, fwRefContainer<net::HttpResponse> response)
		{
			bool needsAuthorization = !g_prometheusAuthHash.empty();
			bool authorizedRequest = false;

			if (needsAuthorization)
			{
				if (const auto authorization = request->GetHeader("Authorization", ""); !authorization.empty() && authorization == g_prometheusAuthHash)
				{
					authorizedRequest = true;
				}

				if (!authorizedRequest)
				{
					response->SetStatusCode(401);
					response->End("Unauthorized.");
					return;
				}
			}

			static auto limiter = instance->GetComponent<fx::PeerAddressRateLimiterStore>()->GetRateLimiter("http_perf", fx::RateLimiterDefaults{ 2.0, 5.0 });
			auto address = request->GetRemotePeer();

			bool cooldown = false;

			if (!fx::IsProxyAddress(address) && !authorizedRequest && !limiter->Consume(address, 1.0, &cooldown))
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

			const prometheus::TextSerializer serializer {};
			std::string serialized = serializer.Serialize(metrics);

			response->SetHeader("Content-Type", "text/plain");
			response->End(std::move(serialized));
		});
	}, 1500);
});
