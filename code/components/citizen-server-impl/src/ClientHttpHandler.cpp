#include "StdInc.h"
#include <ServerInstanceBase.h>
#include <HttpServerManager.h>
#include <ClientHttpHandler.h>

#include <CoreConsole.h>
#include <pplx/threadpool.h>

#include <json.hpp>

using json = nlohmann::json;

static std::shared_ptr<ConVar<bool>> g_threadedHttpVar;

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

	void ClientMethodRegistry::AddAfterFilter(const std::string& method, const TFilter& handler)
	{
		auto it = m_methods.find(method);

		assert(it != m_methods.end());

		auto lastValue = it->second;

		it->second = [=](const std::map<std::string, std::string>& a1, const fwRefContainer<net::HttpRequest>& a2, const TCallback& a3)
		{
			lastValue(a1, a2, [=](const json& data)
			{
				handler(data, a1, a2, a3);
			});
		};
	}

	std::map<std::string, std::string> ParsePOSTString(const std::string_view& postDataString)
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

			UrlDecode(std::string(postDataString.substr(curPos, equalsPos - curPos)), key);
			UrlDecode(std::string(postDataString.substr(equalsPos + 1, endPos - equalsPos - 1)), value);

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

				auto runTask = [=]()
				{
					(*handler)(postMap, request, [response](const json& data)
					{
						if (data.is_null())
						{
							response->End();
							return;
						}

						response->Write(data.dump() + "\r\n");
					});
				};

				if (g_threadedHttpVar->GetValue())
				{
					crossplat::threadpool::shared_instance().service().post(runTask);
				}
				else
				{
					runTask();
				}
			});
		};
	}
}

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		g_threadedHttpVar = instance->AddVariable<bool>("sv_threadedClientHttp", ConVar_None, true);

		instance->SetComponent(new fx::ClientMethodRegistry());

		instance->GetComponent<fx::HttpServerManager>()->AddEndpoint("/client", fx::GetClientEndpointHandler(instance));
	});
});
