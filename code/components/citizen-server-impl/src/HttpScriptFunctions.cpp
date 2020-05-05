#include "StdInc.h"

#include <ResourceManager.h>
#include <ResourceEventComponent.h>

#include <ScriptEngine.h>

#include <cpr/cpr.h>

#include <fxScripting.h>

#include <json.hpp>

#include <tbb/concurrent_vector.h>

#include <HttpClient.h>

using json = nlohmann::json;

static InitFunction initFunction([]()
{
	static HttpClient* httpClient = new HttpClient(L"FXServer/PerformHttpRequest");

	fx::ScriptEngine::RegisterNativeHandler("PERFORM_HTTP_REQUEST_INTERNAL", [](fx::ScriptContext& context)
	{
		static std::atomic<int> reqToken;

		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());
			fwRefContainer<fx::ResourceEventComponent> evComponent = resource->GetComponent<fx::ResourceEventComponent>();

			if (resource)
			{
				// parse passed argument data
				auto unpacked = json::parse(std::string(context.CheckArgument<const char*>(0), context.GetArgument<size_t>(1)));

				// get method
				auto method = unpacked.value<std::string>("method", "GET");

				// get headers from the passed data
				std::map<std::string, std::string> headerMap;

				if (unpacked["headers"].is_object())
				{
					auto headerArray = unpacked.value<std::map<std::string, json>>("headers", {});

					for (const auto& header : headerArray)
					{
						headerMap.insert({ header.first, header.second.get<std::string>() });
					}
				}

				auto url = unpacked.value<std::string>("url", "");
				auto data = unpacked.value<std::string>("data", "");

				auto responseHeaders = std::make_shared<HttpHeaderList>();
				auto responseCode = std::make_shared<int>();

				using namespace std::chrono_literals;

				HttpRequestOptions options;
				options.headers = headerMap;
				options.responseHeaders = responseHeaders;
				options.responseCode = responseCode;
				options.timeoutNoResponse = 5000ms;

				// create token
				int token = reqToken.fetch_add(1);

				// run a HTTP request
				httpClient->DoMethodRequest(method, url, data, options, [evComponent, token, responseCode, responseHeaders](bool success, const char* data, size_t length)
				{
					if (!success)
					{
						evComponent->QueueEvent2("__cfx_internal:httpResponse", {}, token, *responseCode, msgpack::type::nil_t{}, std::map<std::string, std::string>());
					}
					else
					{
						evComponent->QueueEvent2("__cfx_internal:httpResponse", {}, token, *responseCode, std::string{ data, length }, *responseHeaders);
					}
				});

				context.SetResult(token);
				return;
			}
		}

		context.SetResult(-1);
	});
});
