#include <StdInc.h>

#include <CoreConsole.h>

#include <GameServer.h>
#include <HttpClient.h>

#include <ResourceEventComponent.h>
#include <ResourceManager.h>

#include <ServerInstanceBase.h>
#include <ServerInstanceBaseRef.h>

#include <ServerLicensingComponent.h>

#include <TcpListenManager.h>
#include <ReverseTcpServer.h>

#include <json.hpp>

static InitFunction initFunction([]()
{
	static auto httpClient = new HttpClient();

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		using namespace std::chrono_literals;

		static bool setNucleus = false;
		static bool setNucleusSuccess = false;
		static std::chrono::milliseconds setNucleusTimeout;

		instance->GetComponent<fx::GameServer>()->OnTick.Connect([instance]()
		{
			if (!setNucleusSuccess && (!setNucleus || (msec() > setNucleusTimeout)))
			{
				auto var = instance->GetComponent<console::Context>()->GetVariableManager()->FindEntryRaw("sv_licenseKeyToken");

				if (var && !var->GetValue().empty())
				{
					auto licensingComponent = instance->GetComponent<ServerLicensingComponent>();
					auto nucleusToken = licensingComponent->GetNucleusToken();

					if (!nucleusToken.empty())
					{
						auto tlm = instance->GetComponent<fx::TcpListenManager>();

						auto jsonData = nlohmann::json::object({
							{ "token", nucleusToken },
							{ "port", fmt::sprintf("%d", tlm->GetPrimaryPort()) },
							{ "ipOverride", instance->GetComponent<fx::GameServer>()->GetIpOverrideVar()->GetValue() },
						});

						trace("^3%suthenticating with Nucleus...^7\n", setNucleus ? "Rea" : "A");

						static auto authDelay = 15s;

						setNucleusTimeout = msec() + authDelay;

						httpClient->DoPostRequest("https://cfx.re/api/register/?v=2", jsonData.dump(), [instance, tlm](bool success, const char* data, size_t length)
						{
							if (!success)
							{
								if (authDelay < 15min)
								{
									authDelay *= 2;
								}

								setNucleusTimeout = msec() + authDelay;

								trace("^1Authenticating with Nucleus failed! That's possibly bad.^7\n");
							}
							else
							{
								auto jsonData = nlohmann::json::parse(std::string(data, length));

								trace("^1        fff                          \n^1  cccc ff   xx  xx     rr rr    eee  \n^1cc     ffff   xx       rrr  r ee   e \n^1cc     ff     xx   ... rr     eeeee  \n^1 ccccc ff   xx  xx ... rr      eeeee \n                                     ^7\n");
								trace("^2Authenticated with cfx.re Nucleus: ^7https://%s/\n", jsonData.value("host", ""));

								fwRefContainer<net::ReverseTcpServer> rts = new net::ReverseTcpServer();
								rts->Listen("users.cfx.re:30130", jsonData.value("rpToken", ""));

								tlm->AddExternalServer(rts);

								instance->GetComponent<fx::ResourceManager>()
									->GetComponent<fx::ResourceEventManagerComponent>()
									->QueueEvent2(
										"_cfx_internal:nucleusConnected",
										{},
										fmt::sprintf("https://%s/", jsonData.value("host", ""))
									);

								static auto webVar = instance->AddVariable<std::string>("web_baseUrl", ConVar_None, jsonData.value("host", ""));

								setNucleusSuccess = true;
							}
						});
					}

					setNucleus = true;
				}
			}
		});
	}, INT32_MAX);
});
