#include "StdInc.h"

#include <ResourceManager.h>
#include <ResourceEventComponent.h>

#include <ScriptEngine.h>

#include <cpr/cpr.h>

#include <fxScripting.h>

#include <json.hpp>
#include <utf8.h>

#include <tbb/concurrent_vector.h>

#include <HttpClient.h>

using json = nlohmann::json;

static HttpClient* httpClient;
static std::atomic<int> reqToken;

template<bool IsJson>
void PerformHttpRequestInternal(fx::ScriptContext& context)
{
	fx::OMPtr<IScriptRuntime> runtime;

	if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
	{
		fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());
		fwRefContainer<fx::ResourceEventComponent> evComponent = resource->GetComponent<fx::ResourceEventComponent>();

		if (resource)
		{
			bool followLocation = false;
			std::map<std::string, std::string> headerMap;
			std::string method = "GET";
			std::string url;
			std::string data;

			if constexpr (IsJson)
			{
				// parse passed argument data
				auto inString = std::string_view{ context.CheckArgument<const char*>(0), context.GetArgument<size_t>(1) };

				if (!utf8::is_valid(inString.begin(), inString.end()))
				{
					context.SetResult(-1);
					return;
				}

				auto unpacked = json::parse(inString);

				// get method
				method = unpacked.value<std::string>("method", "GET");

				// get headers from the passed data
				if (unpacked["headers"].is_object())
				{
					auto headerArray = unpacked.value<std::map<std::string, json>>("headers", {});

					for (const auto& header : headerArray)
					{
						headerMap.insert({ header.first, header.second.get<std::string>() });
					}
				}

				url = unpacked.value<std::string>("url", "");
				data = unpacked.value<std::string>("data", "");

				followLocation = unpacked.value<bool>("followLocation", true);
			}
			else
			{
				auto unpacked = msgpack::unpack(context.CheckArgument<const char*>(0), context.GetArgument<size_t>(1));
				const auto& obj = unpacked.get();

				if (obj.type != msgpack::type::MAP)
				{
					throw std::runtime_error("invalid msgpack type");
				}

				// convert to map
				auto map = obj.as<std::map<std::string, msgpack::object>>();

				// get passed arguments
				map["method"].convert_if_not_nil(method);

				if (map["headers"].type == msgpack::type::MAP) // Lua may make it array
				{
					map["headers"].convert(headerMap);
				}

				map["url"].convert_if_not_nil(url);
				map["data"].convert_if_not_nil(data);
				map["followLocation"].convert_if_not_nil(followLocation);
			}

			auto responseHeaders = std::make_shared<HttpHeaderList>();
			auto responseCode = std::make_shared<int>();

			using namespace std::chrono_literals;

			HttpRequestOptions options;
			options.headers = headerMap;
			options.responseHeaders = responseHeaders;
			options.responseCode = responseCode;
			options.timeoutNoResponse = 5000ms;
			options.followLocation = followLocation;
			options.addErrorBody = true;

			// create token
			int token = reqToken.fetch_add(1);

			// run a HTTP request
			httpClient->DoMethodRequest(method, url, data, options, [evComponent, token, responseCode, responseHeaders](bool success, const char* data, size_t length)
			{
				if (!success)
				{
					evComponent->QueueEvent2("__cfx_internal:httpResponse", {}, token, *responseCode, msgpack::type::nil_t{}, std::map<std::string, std::string>(), std::string{ data, length });
				}
				else
				{
					msgpack::zone mz;
					auto responseHeaderData = std::map<std::string, msgpack::object>();
					for (auto& [key, value] : *responseHeaders)
					{
						if (auto it = responseHeaderData.find(key); it != responseHeaderData.end())
						{
							// inefficient, but trivial
							std::vector<std::string> newVal;

							if (it->second.type != msgpack::type::ARRAY)
							{
								newVal = {
									it->second.as<std::string>(),
									value
								};
							}
							else
							{
								newVal = it->second.as<std::vector<std::string>>();
								newVal.push_back(value);
							}

							responseHeaderData.erase(it);
							responseHeaderData.emplace(key, msgpack::object{ newVal, mz });
						}
						else
						{
							responseHeaderData.emplace(key, msgpack::object{ value, mz });
						}
					}

					evComponent->QueueEvent2("__cfx_internal:httpResponse", {}, token, *responseCode, std::string{ data, length }, responseHeaderData, msgpack::type::nil_t{});
				}
			});

			context.SetResult(token);
			return;
		}
	}

	context.SetResult(-1);
}

static InitFunction initFunction([]()
{
	httpClient = new HttpClient(L"FXServer/PerformHttpRequest");

	fx::ScriptEngine::RegisterNativeHandler("PERFORM_HTTP_REQUEST_INTERNAL", PerformHttpRequestInternal<true>);
	fx::ScriptEngine::RegisterNativeHandler("PERFORM_HTTP_REQUEST_INTERNAL_EX", PerformHttpRequestInternal<false>);
});
