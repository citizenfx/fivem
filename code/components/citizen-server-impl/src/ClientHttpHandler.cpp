#include "StdInc.h"
#include <ServerInstanceBase.h>
#include <HttpServerManager.h>
#include <ClientHttpHandler.h>

#include <json.hpp>

using json = nlohmann::json;

namespace fx
{
	auto ClientMethodRegistry::GetHandler(const std::string& method) -> std::optional<THandler>
	{
		auto it = m_methods.find(method);

		return (it == m_methods.end()) ? std::optional<THandler>() : it->second;
	}

	void ClientMethodRegistry::AddHandler(const std::string& method, const THandler& handler)
	{
		m_methods.insert({ method, handler });
	}

	static std::map<std::string, std::string> ParsePOSTString(const std::string& postDataString)
	{
		std::map<std::string, std::string> postMap;

		// split the string by the usual post map characters
		int curPos = 0;

		while (true)
		{
			int endPos = postDataString.find_first_of('&', curPos);

			int equalsPos = postDataString.find_first_of('=', curPos);

			std::string key;
			std::string value;

			UrlDecode(postDataString.substr(curPos, equalsPos - curPos), key);
			UrlDecode(postDataString.substr(equalsPos + 1, endPos - equalsPos - 1), value);

			postMap[key] = value;

			// save and continue
			curPos = endPos;

			if (curPos == std::string::npos)
			{
				break;
			}

			curPos++;
		}

		return postMap;
	}

	static auto GetClientEndpointHandler(fx::ServerInstanceBase* instance)
	{
		return [=](const fwRefContainer<net::HttpRequest>& request, const fwRefContainer<net::HttpResponse>& response)
		{
			if (request->GetRequestMethod() != "POST")
			{
				response->WriteHead(405);
				response->End("/client is POST only");
				return;
			}

			request->SetDataHandler([=](const std::vector<uint8_t>& postData)
			{
				auto endError = [=](const std::string& error)
				{
					response->End(json::object({ {"error", error} }).dump());
				};

				auto postMap = ParsePOSTString(std::string(postData.begin(), postData.end()));

				auto method = postMap.find("method");

				if (method == postMap.end())
				{
					endError("missing method");
					return;
				}

				auto handler = instance->GetComponent<ClientMethodRegistry>()->GetHandler(method->second);

				if (!handler)
				{
					endError("invalid method");
					return;
				}

				json data = (*handler)(postMap, request);

				response->End(data.dump());
			});
		};
	}
}

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->SetComponent(new fx::ClientMethodRegistry());

		instance->GetComponent<fx::HttpServerManager>()->AddEndpoint("/client", fx::GetClientEndpointHandler(instance));
	});
});