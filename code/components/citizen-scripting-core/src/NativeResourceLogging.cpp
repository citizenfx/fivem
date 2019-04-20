#include <StdInc.h>

#if __has_include("NativeHandlerLogging.h") && defined(ENABLE_NATIVE_HANDLER_LOGGING)
#include <HttpClient.h>
#include <NativeHandlerLogging.h>

#include <Resource.h>
#include <ResourceManager.h>

#include <unordered_map>
#include <unordered_set>

#include <json.hpp>

static std::unordered_map<std::string, std::unordered_set<uint64_t>> g_nativeLogs;

using json = nlohmann::json;

static void AsyncUploadNativeLog()
{
	FILE* f = _wfopen(MakeRelativeCitPath(L"cache/native_stats.json").c_str(), L"rb");

	if (f)
	{
		fseek(f, 0, SEEK_END);

		size_t size = ftell(f);

		fseek(f, 0, SEEK_SET);

		std::vector<char> data(size);
		fread(data.data(), 1, data.size(), f);

		fclose(f);

		_wunlink(MakeRelativeCitPath(L"cache/native_stats.json").c_str());

		HttpRequestOptions options;
		options.headers["content-type"] = "application/json; charset=utf-8";

		Instance<HttpClient>::Get()->DoPostRequest("http://native-collector-live.fivem.net/upload", std::string(data.data(), data.size()), options, [](bool success, const char*, size_t)
		{

		});
	}
}

static void SerializeNativeLog()
{
	auto jsonMap = json::object();

	for (const auto& dataPair : g_nativeLogs)
	{
		const auto& [resourceName, set] = dataPair;

		auto arr = json::array();

		for (const auto& native : set)
		{
			arr.push_back(json(fmt::sprintf("%016llx", native)));
		}

		jsonMap[resourceName] = arr;
	}

	FILE* f = _wfopen(MakeRelativeCitPath(L"cache/native_stats.json").c_str(), L"wb");

	if (f)
	{
		auto str = jsonMap.dump();

		fwrite(str.c_str(), 1, str.size(), f);
		fclose(f);
	}
}

static InitFunction initFunction([]()
{
	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
	{
		resource->OnActivate.Connect([resource]()
		{
			NativeHandlerLogging::PushLog(std::make_shared<concurrency::concurrent_unordered_set<uint64_t>>());
		}, -99999999);

		resource->OnDeactivate.Connect([resource]()
		{
			auto nativeLog = NativeHandlerLogging::PopLog();

			if (nativeLog)
			{
				auto& outLog = g_nativeLogs[resource->GetName()];

				for (const auto& entry : *nativeLog)
				{
					outLog.insert(entry);
				}
			}
		}, 99999999);
	});

	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* resourceManager)
	{
		AsyncUploadNativeLog();

		resourceManager->OnTick.Connect([]()
		{
			static uint32_t lastTick;

			if ((GetTickCount() - lastTick) > 30000)
			{
				SerializeNativeLog();

				lastTick = GetTickCount();
			}
		});
	});
});
#endif
