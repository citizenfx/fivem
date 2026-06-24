#include "StdInc.h"

#include <ServerInstanceBase.h>
#include <HttpServerManager.h>
#include <ResourceManager.h>
#include <StateBagComponent.h>
#include <ClientRegistry.h>
#include <state/ServerGameStatePublic.h>

#include <CoreConsole.h>
#include <botan/base64.h>

#include <json.hpp>

using json = nlohmann::json;

namespace
{
std::shared_ptr<ConVar<std::string>> g_onesyncBasicAuthUser;
std::shared_ptr<ConVar<std::string>> g_onesyncBasicAuthPassword;
}

static std::string MakeBasicAuthHash(const std::string& user, const std::string& password)
{
	if (user.empty() && password.empty())
		return {};

	std::string authString = user + ":" + password;
	return "Basic " + Botan::base64_encode(reinterpret_cast<const uint8_t*>(authString.data()), authString.size());
}

static bool IsAuthorizedRequest(const fwRefContainer<net::HttpRequest>& request)
{
	if (!g_onesyncBasicAuthUser)
		return false;

	const std::string authHash = MakeBasicAuthHash(g_onesyncBasicAuthUser->GetValue(), g_onesyncBasicAuthPassword->GetValue());

	if (authHash.empty())
		return false;

	const auto auth = request->GetHeader("Authorization", "");
	return !auth.empty() && auth == authHash;
}

static std::string GetBagIdFromPath(std::string_view prefix, const char* path)
{
	constexpr std::string_view suffix = "/stateBag.json";
	std::string_view pathView(path);

	if (pathView.size() <= prefix.size() + suffix.size())
		return {};
	if (pathView.substr(0, prefix.size()) != prefix)
		return {};
	auto inner = pathView.substr(prefix.size());
	if (inner.size() < suffix.size())
		return {};
	if (inner.substr(inner.size() - suffix.size()) != suffix)
		return {};
	return std::string(inner.substr(0, inner.size() - suffix.size()));
}

static json DecodeStoredData(const std::map<std::string, std::string>& storedData)
{
	json result = json::object();
	for (const auto& [key, value] : storedData)
	{
		json parsed = json::parse(value, nullptr, false);
		result[key] = parsed.is_discarded() ? json(value) : std::move(parsed);
	}
	return result;
}

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		g_onesyncBasicAuthUser = instance->AddVariable<std::string>("sv_onesyncBasicAuthUser", ConVar_None, "");
		g_onesyncBasicAuthPassword = instance->AddVariable<std::string>("sv_onesyncBasicAuthPassword", ConVar_None, "");

		auto httpManager = instance->GetComponent<fx::HttpServerManager>();

		httpManager->AddEndpoint("/onesync/statebags.json",
			[instance](const fwRefContainer<net::HttpRequest>& request, fwRefContainer<net::HttpResponse> response)
			{
				if (!IsAuthorizedRequest(request))
				{
					response->SetStatusCode(403);
					response->End("Forbidden.");
					return;
				}

				auto stateBags = instance->GetComponent<fx::ResourceManager>()->GetComponent<fx::StateBagComponent>();

				auto globalDetails = stateBags->GetStateBagDetails("global");
				json globalStateJson = globalDetails.has_value()
					? DecodeStoredData(globalDetails->storedData)
					: json::object();

				json targetsJson = json::array();
				for (int targetId : stateBags->GetRegisteredTargets())
				{
					targetsJson.push_back({
						{ "id", targetId },
						{ "bagCount", stateBags->GetStateBagCountForTarget(targetId) }
					});
				}

				json prefixesJson = json::array();
				for (const auto& [prefix, useParentTargets] : stateBags->GetPreCreatePrefixes())
				{
					prefixesJson.push_back({
						{ "prefix", prefix },
						{ "useParentTargets", useParentTargets }
					});
				}

				json result = {
					{ "totalBags", stateBags->GetStateBagCount() },
					{ "activeBags", stateBags->GetActiveStateBagCount() },
					{ "expiredBags", stateBags->GetExpiredStateBagCount() },
					{ "targetCount", stateBags->GetTargetCount() },
					{ "preCreatedBags", stateBags->GetPreCreatedStateBagCount() },
					{ "erasureListCount", stateBags->GetErasureListCount() },
					{ "preCreatePrefixCount", stateBags->GetPreCreatePrefixCount() },
					{ "globalState", std::move(globalStateJson) },
					{ "targets", std::move(targetsJson) },
					{ "prefixes", std::move(prefixesJson) }
				};

				response->SetHeader("Content-Type", "application/json; charset=utf-8");
				response->End(result.dump());
			});

		httpManager->AddEndpoint("/onesync/entities.json",
			[instance](const fwRefContainer<net::HttpRequest>& request, fwRefContainer<net::HttpResponse> response)
			{
				if (!IsAuthorizedRequest(request))
				{
					response->SetStatusCode(403);
					response->End("Forbidden.");
					return;
				}

				auto gameState = instance->GetComponent<fx::ServerGameStatePublic>();

				json entitiesJson = json::array();
				for (const auto& e : gameState->GetEntityDebugInfoList())
				{
					json entityJson = {
						{ "id", e.id },
						{ "netId", e.netId },
						{ "routingBucket", e.routingBucket },
						{ "ownerClientID", e.ownerClientID },
						{ "firstOwningClientID", e.firstOwningClientID },
						{ "firstOwnerDropped", e.firstOwnerDropped },
						{ "lastOwningClientID", e.lastOwningClientID },
						{ "type", e.type },
						{ "uniquer", e.uniquer },
						{ "scriptHash", e.scriptHash },
						{ "ownedByScript", e.ownedByScript },
						{ "ownedByServerScript", e.ownedByServerScript },
						{ "shouldServerKeepEntity", e.shouldServerKeepEntity },
						{ "relevant", e.relevant },
						{ "createdAt", e.createdAt },
						{ "frameIndex", e.frameIndex },
						{ "lastFrameIndex", e.lastFrameIndex },
						{ "timestamp", e.timestamp },
						{ "creationToken", e.creationToken },
						{ "lastMigratedAt", e.lastMigratedAt }
					};

					if (e.hasStateBag)
					{
						entityJson["stateBag"] = e.stateBagName;
					}

					if (e.hasPosition)
					{
						entityJson["position"] = { e.position[0], e.position[1], e.position[2] };
					}

					entitiesJson.push_back(std::move(entityJson));
				}

				response->SetHeader("Content-Type", "application/json; charset=utf-8");
				response->End(entitiesJson.dump());
			});

		httpManager->AddEndpoint("/onesync/entities/",
			[instance](const fwRefContainer<net::HttpRequest>& request, fwRefContainer<net::HttpResponse> response)
			{
				if (!IsAuthorizedRequest(request))
				{
					response->SetStatusCode(403);
					response->End("Forbidden.");
					return;
				}

				auto entityId = GetBagIdFromPath("/onesync/entities/", request->GetPath().c_str());
				if (entityId.empty())
				{
					response->SetStatusCode(400);
					response->SetHeader("Content-Type", "application/json; charset=utf-8");
					response->End(json{ { "error", "Invalid path. Expected /onesync/entities/{netId}/stateBag.json" } }.dump());
					return;
				}

				auto stateBags = instance->GetComponent<fx::ResourceManager>()->GetComponent<fx::StateBagComponent>();
				auto details = stateBags->GetStateBagDetails("entity:" + entityId);

				if (!details.has_value())
				{
					response->SetStatusCode(404);
					response->SetHeader("Content-Type", "application/json; charset=utf-8");
					response->End(json{ { "error", "StateBag not found." } }.dump());
					return;
				}

				json result = DecodeStoredData(details->storedData);

				response->SetHeader("Content-Type", "application/json; charset=utf-8");
				response->End(result.dump());
			});

		httpManager->AddEndpoint("/onesync/clients.json",
			[instance](const fwRefContainer<net::HttpRequest>& request, fwRefContainer<net::HttpResponse> response)
			{
				if (!IsAuthorizedRequest(request))
				{
					response->SetStatusCode(403);
					response->End("Forbidden.");
					return;
				}

				auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

				json clientsJson = json::array();
				clientRegistry->ForAllClients([&clientsJson](const fx::ClientSharedPtr& client)
				{
					if (!client->HasConnected())
					{
						return;
					}

					clientsJson.push_back({ { "id", client->GetNetId() } });
				});

				response->SetHeader("Content-Type", "application/json; charset=utf-8");
				response->End(clientsJson.dump());
			});

		httpManager->AddEndpoint("/onesync/clients/",
			[instance](const fwRefContainer<net::HttpRequest>& request, fwRefContainer<net::HttpResponse> response)
			{
				if (!IsAuthorizedRequest(request))
				{
					response->SetStatusCode(403);
					response->End("Forbidden.");
					return;
				}

				auto clientId = GetBagIdFromPath("/onesync/clients/", request->GetPath().c_str());
				if (clientId.empty())
				{
					response->SetStatusCode(400);
					response->SetHeader("Content-Type", "application/json; charset=utf-8");
					response->End(json{ { "error", "Invalid path. Expected /onesync/clients/{netId}/stateBag.json" } }.dump());
					return;
				}

				auto stateBags = instance->GetComponent<fx::ResourceManager>()->GetComponent<fx::StateBagComponent>();
				auto details = stateBags->GetStateBagDetails("player:" + clientId);

				if (!details.has_value())
				{
					response->SetStatusCode(404);
					response->SetHeader("Content-Type", "application/json; charset=utf-8");
					response->End(json{ { "error", "StateBag not found." } }.dump());
					return;
				}

				json result = DecodeStoredData(details->storedData);

				response->SetHeader("Content-Type", "application/json; charset=utf-8");
				response->End(result.dump());
			});
	});
});
