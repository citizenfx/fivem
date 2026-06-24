#include <StdInc.h>

#include <CoreConsole.h>

#include <HttpClient.h>

#include <ResourceEventComponent.h>
#include <ResourceManager.h>

#include <MonitorInstance.h>
#include <TcpListenManager.h>

#include <json.hpp>

#include <boost/random/random_device.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <chrono>

extern fwEvent<fx::MonitorInstance*> OnMonitorTick;

inline auto msec()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
}

static InitFunction initFunction([]()
{
	static auto httpClient = new HttpClient();

	using namespace std::chrono_literals;

	static bool registerSent = false;
	static bool registerSuccess = false;
	static std::chrono::milliseconds registerTimeout{};
	static auto registerDelay = 15s;

	OnMonitorTick.Connect([](fx::MonitorInstance* instance)
	{
		if (registerSuccess)
		{
			return;
		}

		if (registerSent && msec() < registerTimeout)
		{
			return;
		}

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

		auto tlm = instance->GetComponent<fx::TcpListenManager>();

		auto reqJson = nlohmann::json::object({
			{ "token", "anonymous" },
			{ "tokenEx", extToken },
			{ "port", fmt::sprintf("%d", tlm->GetPrimaryPort()) },
		});

		registerTimeout = msec() + registerDelay;
		registerSent = true;

		HttpRequestOptions opts;
		opts.ipv4 = true;

		httpClient->DoPostRequest("https://cfx.re/api/register/?v=2", reqJson.dump(), opts, [instance](bool success, const char* data, size_t length)
		{
			auto backoff = [&]()
			{
				if (registerDelay < 15min)
				{
					registerDelay *= 2;
				}
				registerTimeout = msec() + registerDelay;
			};

			if (!success)
			{
				backoff();
				return;
			}

			std::string host;
			try
			{
				auto resp = nlohmann::json::parse(std::string(data, length));
				host = resp.value("host", "");
			}
			catch (const std::exception&)
			{
				backoff();
				return;
			}

			if (host.empty())
			{
				backoff();
				return;
			}

			auto url = fmt::sprintf("https://%s/", host);

			instance->GetComponent<fx::ResourceManager>()
				->GetComponent<fx::ResourceEventManagerComponent>()
				->QueueEvent2(
					"_cfx_internal:nucleusConnected",
					{},
					url
				);

			static auto webVar = instance->AddVariable<std::string>("web_baseUrl", ConVar_None, host);

			registerSuccess = true;
		});
	});
});
