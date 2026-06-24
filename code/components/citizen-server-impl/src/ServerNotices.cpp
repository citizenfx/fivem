#include <StdInc.h>

#include <HttpClient.h>

#include <ServerInstanceBase.h>

#include <NoticeLogicProcessor.h>

// 100KiB cap, conditional notices should fit more than comfortably under this limit
#define MAX_NOTICE_FILESIZE 102400

static void DownloadAndProcessNotices(fx::ServerInstanceBase* server, HttpClient* httpClient)
{
	HttpRequestOptions options;
	options.maxFilesize = MAX_NOTICE_FILESIZE;
	httpClient->DoGetRequest("https://gss.cfx-services.net/v1/public/promotions-targeting", options, [server, httpClient](bool success, const char* data, size_t length)
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

static InitFunction initFunction([]()
{
	static auto httpClient = new HttpClient();

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		DownloadAndProcessNotices(instance, httpClient);
	}, INT32_MAX);
});
