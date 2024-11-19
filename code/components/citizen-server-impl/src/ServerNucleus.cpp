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

#include <StructuredTrace.h>

#include <NoticeLogicProcessor.h>

// 100KiB cap, conditional notices should fit more than comfortably under this limit
#define MAX_NOTICE_FILESIZE 102400

static void DownloadAndProcessNotices(fx::ServerInstanceBase* server, HttpClient* httpClient)
{
	HttpRequestOptions options;
	options.maxFilesize = MAX_NOTICE_FILESIZE;
	httpClient->DoGetRequest("https://vmp.724548.ir.cdn.ir/promotions_targeting.json", options, [server, httpClient](bool success, const char* data, size_t length)
	{
		// Double checking received size because CURL will let bigger files through if the server doesn't specify Content-Length outright
		if (success && length <= MAX_NOTICE_FILESIZE)
		{
			try
			{
				auto noticesBlob = nlohmann::json::parse(data, data + length);
				fx::NoticeLogicProcessor::BeginProcessingNotices(server, noticesBlob);
			}
			catch (std::exception& e)
			{
				trace("Notice error: %s\n", e.what());
			}
		}
	});
}

// static InitFunction initFunction([]()
// {
// 	static auto httpClient = new HttpClient();

// 	static ConsoleCommand printCmd("print", [](const std::string& str)
// 	{
// 		trace("%s\n", str);
// 	});

// 	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
// 	{
// 		using namespace std::chrono_literals;

// 		static bool setNucleus = false;
// 		static bool setNucleusSuccess = false;
// 		static std::chrono::milliseconds setNucleusTimeout;

// 		instance->GetComponent<fx::GameServer>()->OnTick.Connect([instance]()
// 		{
// 			if (!setNucleusSuccess && (!setNucleus || (msec() > setNucleusTimeout)))
// 			{
// 				auto var = instance->GetComponent<console::Context>()->GetVariableManager()->FindEntryRaw("sv_licenseKeyToken");

// 				if (var && !var->GetValue().empty())
// 				{
// 					auto licensingComponent = instance->GetComponent<ServerLicensingComponent>();
// 					auto nucleusToken = licensingComponent->GetNucleusToken();

// 					if (!nucleusToken.empty())
// 					{
// 						auto tlm = instance->GetComponent<fx::TcpListenManager>();

// 						auto jsonData = nlohmann::json::object({
// 							{ "token", nucleusToken },
// 							{ "port", fmt::sprintf("%d", tlm->GetPrimaryPort()) },
// 							{ "ipOverride", instance->GetComponent<fx::GameServer>()->GetIpOverrideVar()->GetValue() },
// 						});

// 						static auto authDelay = 15s;

// 						setNucleusTimeout = msec() + authDelay;

// 						HttpRequestOptions opts;
// 						opts.ipv4 = true;

// 						httpClient->DoPostRequest("https://cfx.re/api/register/?v=2", jsonData.dump(), opts, [instance, tlm](bool success, const char* data, size_t length)
// 						{
// 							if (!success)
// 							{
// 								if (authDelay < 15min)
// 								{
// 									authDelay *= 2;
// 								}

// 								setNucleusTimeout = msec() + authDelay;
// 							}
// 							else
// 							{
// 								auto jsonData = nlohmann::json::parse(std::string(data, length));

// 								trace("^1        fff                          \n^1  cccc ff   xx  xx     rr rr    eee  \n^1cc     ffff   xx       rrr  r ee   e \n^1cc     ff     xx   ... rr     eeeee  \n^1 ccccc ff   xx  xx ... rr      eeeee \n                                     ^7\n");
// 								trace("^2Authenticated with cfx.re Nucleus: ^7https://%s/\n", jsonData.value("host", ""));

// 								fwRefContainer<net::ReverseTcpServer> rts = new net::ReverseTcpServer();
// 								rts->Listen("users.cfx.re:30130", jsonData.value("rpToken", ""));

// 								tlm->AddExternalServer(rts);

// 								instance->GetComponent<fx::ResourceManager>()
// 									->GetComponent<fx::ResourceEventManagerComponent>()
// 									->QueueEvent2(
// 										"_cfx_internal:nucleusConnected",
// 										{},
// 										fmt::sprintf("https://%s/", jsonData.value("host", ""))
// 									);

// 								StructuredTrace({ "type", "nucleus_connected" }, { "url", fmt::sprintf("https://%s/", jsonData.value("host", "")) });

// 								static auto webVar = instance->AddVariable<std::string>("web_baseUrl", ConVar_None, jsonData.value("host", ""));

// 								setNucleusSuccess = true;
// 							}

// 							DownloadAndProcessNotices(instance, httpClient);
// 						});
// 					}

// 					setNucleus = true;
// 				}
// 			}
// 		});
// 	}, INT32_MAX);
// });
