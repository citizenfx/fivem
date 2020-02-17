#include <StdInc.h>

#include <CoreConsole.h>

#include <HttpClient.h>

#include <ResourceEventComponent.h>
#include <ResourceManager.h>

#include <ServerInstanceBase.h>
#include <ServerInstanceBaseRef.h>

#include <TcpListenManager.h>
#include <ReverseTcpServer.h>

#include <MonitorInstance.h>

#include <json.hpp>

extern fwEvent<fx::MonitorInstance*> OnMonitorTick;

inline auto msec()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
}

static InitFunction initFunction([]()
{
	static auto httpClient = new HttpClient();

	using namespace std::chrono_literals;

	static bool setNucleus = false;
	static bool setNucleusSuccess = false;
	static std::chrono::milliseconds setNucleusTimeout;

	OnMonitorTick.Connect([](fx::MonitorInstance* instance)
	{
		if (!setNucleusSuccess && (!setNucleus || (msec() > setNucleusTimeout)))
		{
			std::string nucleusToken = "anonymous";

			if (!nucleusToken.empty())
			{
				auto tlm = instance->GetComponent<fx::TcpListenManager>();

				auto jsonData = nlohmann::json::object({
					{ "token", nucleusToken },
					{ "port", fmt::sprintf("%d", tlm->GetPrimaryPort()) },
				});

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
					}
					else
					{
						auto jsonData = nlohmann::json::parse(std::string(data, length));

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
	});
});
