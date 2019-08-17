#include <StdInc.h>

#include <CoreConsole.h>

#include <GameServer.h>
#include <HttpClient.h>

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
		static bool setNucleus = false;

		instance->GetComponent<fx::GameServer>()->OnTick.Connect([instance]()
		{
			if (!setNucleus)
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
							{ "port", fmt::sprintf("%d", tlm->GetPrimaryPort()) }
						});

						trace("^3Authenticating with Nucleus...^7\n");

						httpClient->DoPostRequest("https://cfx.re/api/register/?v=2", jsonData.dump(), [tlm](bool success, const char* data, size_t length)
						{
							if (!success)
							{
								trace("^1Authenticating with Nucleus failed! That's bad.^7\n");
							}
							else
							{
								auto jsonData = nlohmann::json::parse(std::string(data, length));

								trace("^1        fff                          \n  cccc ff   xx  xx     rr rr    eee  \ncc     ffff   xx       rrr  r ee   e \ncc     ff     xx   ... rr     eeeee  \n ccccc ff   xx  xx ... rr      eeeee \n                                     ^7\n");
								trace("^2Authenticated with cfx.re Nucleus: ^7https://%s/\n", jsonData.value("host", ""));

								fwRefContainer<net::ReverseTcpServer> rts = new net::ReverseTcpServer();
								rts->Listen("cfx.re:30130", jsonData.value("rpToken", ""));

								tlm->AddExternalServer(rts);
							}
						});
					}

					setNucleus = true;
				}
			}
		});
	}, INT32_MAX);
});
