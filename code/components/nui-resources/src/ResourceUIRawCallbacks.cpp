#include <StdInc.h>
#include <ResourceUI.h>

#include <fxScripting.h>
#include <ScriptEngine.h>

#include <ResourceManager.h>
#include <ResourceCallbackComponent.h>

#include <boost/algorithm/string.hpp>

struct Header
{
	std::string key;
	std::string value;

	MSGPACK_DEFINE_MAP(key, value);
};

struct RequestWrap
{
	std::map<std::string, std::string> headers;
	std::vector<Header> rawHeaders;
	std::string method;
	std::string body;
	std::string path;

	MSGPACK_DEFINE_MAP(headers, rawHeaders, method, path, body);
};

static ResUICallback MakeUICallback(fx::Resource* resource, const std::string& type, const std::string& ref)
{
	return [resource, type, ref](const std::string& path, const std::string& query, const std::multimap<std::string, std::string>& headers, const std::string& postData, ResUIResultCallback cb)
	{
		RequestWrap req;
		req.method = (postData.empty()) ? "GET" : "POST";
		req.body = postData;

		std::map<std::string, msgpack::object> headerMap;
		for (auto& header : headers)
		{
			req.headers.insert(header);
			req.rawHeaders.push_back({ header.first, header.second });
		}

		req.path = path.substr(type.length());

		if (!query.empty())
		{
			req.path += "?" + query;
		}

		auto cbComponent = resource->GetManager()->GetComponent<fx::ResourceCallbackComponent>();

		auto resCb = cbComponent->CreateCallback([cb](const msgpack::v1::unpacked& unpacked)
		{
			auto args = unpacked.get().as<std::vector<msgpack::object>>();

			if (args.size() == 1)
			{
				auto outObj = args[0].as<std::map<std::string, msgpack::object>>();

				std::multimap<std::string, std::string> headers;
				int statusCode = 200;
				
				if (auto hit = outObj.find("headers"); hit != outObj.end())
				{
					if (hit->second.type != msgpack::type::ARRAY)
					{
						for (auto& pair : hit->second.as<std::map<std::string, msgpack::object>>())
						{
							if (pair.second.type == msgpack::type::ARRAY)
							{
								for (const auto& value : pair.second.as<std::vector<std::string>>())
								{
									headers.emplace(boost::algorithm::to_lower_copy(pair.first), value);
								}
							}
							else
							{
								headers.emplace(boost::algorithm::to_lower_copy(pair.first), pair.second.as<std::string>());
							}
						}
					}
				}

				if (auto sit = outObj.find("status"); sit != outObj.end())
				{
					statusCode = sit->second.as<int>();
				}

				std::string body;

				if (auto bit = outObj.find("body"); bit != outObj.end())
				{
					body = bit->second.as<std::string>();
				}

				cb(statusCode, headers, body);
			}
		});

		resource->GetManager()->CallReference<void>(ref, req, resCb);
	};
}

static InitFunction initFunction([] ()
{
	fx::ScriptEngine::RegisterNativeHandler("REGISTER_RAW_NUI_CALLBACK", [] (fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			if (resource)
			{
				fwRefContainer<ResourceUI> resourceUI = resource->GetComponent<ResourceUI>();

				if (resourceUI.GetRef())
				{
					std::string type = context.CheckArgument<const char*>(0);
					std::string ref = context.CheckArgument<const char*>(1);

					resourceUI->AddCallback(type, MakeUICallback(resource, type, ref));
				}
			}
		}
	});
});
