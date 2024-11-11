
#include "StdInc.h"
#include <ServerIdentityProvider.h>

#include <json.hpp>
#include "ServerLicensingComponent.h"

#include <ServerInstanceBase.h>
#include <ServerInstanceBaseRef.h>
#include <ResourceManager.h>
#include <HttpClient.h>
#include <StdInc.h>

#include <CoreConsole.h>

#include <GameServer.h>
#include <TcpListenManager.h>
#include <Error.h>

#include "boost/filesystem.hpp"
#include <boost/algorithm/string.hpp>

#include <regex>

static bool licenseChecked = false;
static HttpClient* httpClient;

static InitFunction httpinitFunction([]()
{
	httpClient = new HttpClient();

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		if (!licenseChecked)
		{
			console::Printf("Server Auth", "Checking license...\n");
			auto var = instance->GetComponent<console::Context>()->GetVariableManager()->FindEntryRaw("sv_licenseKey");
			if (!var->GetValue().empty())
			{
				auto jsonData = nlohmann::json::object({ { "license", var->GetValue() } });
				auto tlm = instance->GetComponent<fx::TcpListenManager>();
				HttpRequestOptions opts;
				opts.ipv4 = true;
				httpClient->DoPostRequest(fmt::sprintf(LICENSING_EP "server/register.php?work=register"), jsonData.dump(), opts, [instance, tlm](bool success, const char* data, size_t length)
				{
					if (!success)
					{
						FatalError("A connection with the VMP server could not be established!");
					}
					else
					{
						auto jsonData = nlohmann::json::parse(std::string(data, length));
						if (jsonData.value("status", 0) == 1)
						{
							console::Printf("Server Auth", "Server license key authentication succeeded!\n");
							auto consoleCtx = instance->GetComponent<console::Context>();
							{
								se::ScopedPrincipal principalScope(se::Principal{ "system.console" });
								consoleCtx->ExecuteSingleCommandDirect(ProgramArguments{ "sets", "sv_sessionId", jsonData.value("session_id", "0") });
								consoleCtx->ExecuteSingleCommandDirect(ProgramArguments{ "set", "sv_secret", jsonData.value("secret", "0") });
							}
							auto gameServer = instance->GetComponent<fx::GameServer>();
							gameServer->ForceHeartbeat();
							console::Printf("Server Auth", "Session Id : %s\n", jsonData.value("session_id", "0"));
							licenseChecked = true;
						}
						else
						{
							FatalError("%s", jsonData.value("message", "Authentication failed"));
						}
					}
				});
			}
			else
			{
				FatalError("Please set sv_licenseKey in server.cfg!");
			}
		}
	},
	1);
});