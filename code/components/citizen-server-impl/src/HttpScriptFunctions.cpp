#include "StdInc.h"

#include <ResourceManager.h>
#include <ResourceEventComponent.h>

#include <ScriptEngine.h>

#include <cpr/cpr.h>

#include <fxScripting.h>

#include <json.hpp>

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
				auto unpacked = json::parse(std::string(context.GetArgument<const char*>(0), context.GetArgument<size_t>(1)));

				auto method = unpacked.value<std::string>("method", "GET");
				auto headerArray = unpacked.value<std::map<std::string, json>>("headers", {});

				std::map<std::string, std::string, cpr::CaseInsensitiveCompare> headerMap;
				for (const auto& header : headerArray)
				{
					headerMap.insert({ header.first, header.second.get<std::string>() });
				}

				auto url = cpr::Url{ unpacked.value<std::string>("url", "") };
				auto body = cpr::Body{ unpacked.value<std::string>("data", "") };
				auto headers = cpr::Header{
					headerMap
				};

				int token = reqToken.fetch_add(1);

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
				};

				if (method == "GET")
				{
					cpr::GetCallback(cb, url, body, headers);
				}
				else if (method == "POST")
				{
					cpr::PostCallback(cb, url, body, headers);
				}
				else if (method == "HEAD")
				{
					cpr::HeadCallback(cb, url, body, headers);
				}
				else if (method == "OPTIONS")
				{
					cpr::OptionsCallback(cb, url, body, headers);
				}
				else if (method == "PUT")
				{
					cpr::PutCallback(cb, url, body, headers);
				}
				else if (method == "DELETE")
				{
					cpr::DeleteCallback(cb, url, body, headers);
				}
				else if (method == "PATCH")
				{
					cpr::PatchCallback(cb, url, body, headers);
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
