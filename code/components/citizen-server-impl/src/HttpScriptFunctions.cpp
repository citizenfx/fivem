#include "StdInc.h"

#include <ResourceManager.h>
#include <ResourceEventComponent.h>

#include <ScriptEngine.h>

#include <cpr/cpr.h>

#include <fxScripting.h>

#include <json.hpp>

#include <tbb/concurrent_vector.h>

using json = nlohmann::json;

static InitFunction initFunction([]()
{
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
				std::map<std::string, std::string, cpr::CaseInsensitiveCompare> headerMap;

				if (unpacked["headers"].is_object())
				{
					auto headerArray = unpacked.value<std::map<std::string, json>>("headers", {});

					for (const auto& header : headerArray)
					{
						headerMap.insert({ header.first, header.second.get<std::string>() });
					}
				}

				// get URL/body, make CPR structures
				auto url = cpr::Url{ unpacked.value<std::string>("url", "") };
				auto body = cpr::Body{ unpacked.value<std::string>("data", "") };
				auto headers = cpr::Header{
					headerMap
				};

				// create token
				int token = reqToken.fetch_add(1);

				// callback to enqueue events
				auto future = std::make_shared<std::unique_ptr<std::future<void>>>();

				static tbb::concurrent_vector<decltype(future)> futureCleanup;

				auto cb = [=](cpr::Response r)
				{
					if (r.error)
					{
						evComponent->QueueEvent2("__cfx_internal:httpResponse", {}, token, 0, msgpack::type::nil{}, std::map<std::string, std::string>());
					}
					else
					{
						evComponent->QueueEvent2("__cfx_internal:httpResponse", {}, token, r.status_code, r.text, r.header);
					}

					futureCleanup.push_back(future);
				};

				// remove completed futures
				// merely by the virtue of being in this list they're guaranteed-completed, so can be safely removed without blocking
				futureCleanup.clear();

				// invoke cpr::*Callback
				if (method == "GET")
				{
					*future = std::make_unique<std::future<void>>(cpr::GetCallback(cb, url, body, headers, cpr::VerifySsl{ false }));
				}
				else if (method == "POST")
				{
					*future = std::make_unique<std::future<void>>(cpr::PostCallback(cb, url, body, headers, cpr::VerifySsl{ false }));
				}
				else if (method == "HEAD")
				{
					*future = std::make_unique<std::future<void>>(cpr::HeadCallback(cb, url, body, headers, cpr::VerifySsl{ false }));
				}
				else if (method == "OPTIONS")
				{
					*future = std::make_unique<std::future<void>>(cpr::OptionsCallback(cb, url, body, headers, cpr::VerifySsl{ false }));
				}
				else if (method == "PUT")
				{
					*future = std::make_unique<std::future<void>>(cpr::PutCallback(cb, url, body, headers, cpr::VerifySsl{ false }));
				}
				else if (method == "DELETE")
				{
					*future = std::make_unique<std::future<void>>(cpr::DeleteCallback(cb, url, body, headers, cpr::VerifySsl{ false }));
				}
				else if (method == "PATCH")
				{
					*future = std::make_unique<std::future<void>>(cpr::PatchCallback(cb, url, body, headers, cpr::VerifySsl{ false }));
				}
				else
				{
					token = -1;
				}

				context.SetResult(token);
				return;
			}
		}

		context.SetResult(-1);
	});
});
