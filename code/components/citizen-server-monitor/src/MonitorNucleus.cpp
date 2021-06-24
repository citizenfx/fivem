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

#include <boost/random/random_device.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

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
			std::string extToken;

			{
				char tokenData[64] = { 0 };
				fwPlatformString tokenPath = MakeRelativeCitPath("server-monitor-token.key");

				if (auto f = _pfopen(tokenPath.c_str(), _P("rb")))
				{
					fgets(tokenData, std::size(tokenData), f);
					fclose(f);
				}

				if (!tokenData[0])
				{
					std::string token = boost::uuids::to_string(boost::uuids::basic_random_generator<boost::random_device>()());

					if (auto f = _pfopen(tokenPath.c_str(), _P("wb")))
					{
						fmt::fprintf(f, "%s", token);
						fclose(f);
					}

					extToken = token;
				}
				else
				{
					extToken = tokenData;
				}
			}

			static ConVar<std::string> serverProfile("serverProfile", ConVar_None, "default");
			extToken += "_" + serverProfile.GetValue();

			if (!nucleusToken.empty())
			{
				auto tlm = instance->GetComponent<fx::TcpListenManager>();

				auto jsonData = nlohmann::json::object({
					{ "token", nucleusToken },
					{ "tokenEx", extToken },
					{ "port", fmt::sprintf("%d", tlm->GetPrimaryPort()) },
				});

				static auto authDelay = 15s;

				setNucleusTimeout = msec() + authDelay;

				HttpRequestOptions opts;
				opts.ipv4 = true;

				httpClient->DoPostRequest("https://cfx.re/api/register/?v=2", jsonData.dump(), opts, [instance, tlm](bool success, const char* data, size_t length)
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
