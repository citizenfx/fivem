#include "StdInc.h"
#include <ServerInstanceBase.h>
#include <HttpServerManager.h>
#include <ClientHttpHandler.h>

#include <CoreConsole.h>
#include <thread_pool.hpp>

#include <json.hpp>
#include <rapidjson/writer.h>

#include <FormData.h>

using json = nlohmann::json;

static std::shared_ptr<ConVar<bool>> g_threadedHttpVar;
static std::shared_ptr<ConVar<int>> g_maxClientEndpointRequestSize;

namespace fx
{
	auto ClientMethodRegistry::GetHandler(const std::string& method) -> std::optional<std::variant<THandler<TCallback>, THandler<TCallbackFast>>>
	{
		auto it = m_methods.find(method);

		if (it != m_methods.end())
		{
			return it->second;
		}

		return {};
	}

	static auto GetClientEndpointHandler(fx::ServerInstanceBase* instance)
	{
		return [=](const fwRefContainer<net::HttpRequest>& request, fwRefContainer<net::HttpResponse> response)
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
					response->End(json::object({ {"error", error} }).dump(-1, ' ', false, json::error_handler_t::replace));
				};

				if (postData.size() > g_maxClientEndpointRequestSize->GetValue())
				{
					endError("POST data too big");
					return;
				}

				std::string_view postDataStringView {reinterpret_cast<const char*>(postData.data()), postData.size()};
				auto postMap = net::DecodeFormData(postDataStringView);

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
					if (handler->index() == 0)
					{
						(std::get<0>(*handler))(postMap, request, [response](const json& data)
						{
							if (data.is_null())
							{
								response->End();
								return;
							}

							response->Write(data.dump(-1, ' ', false, json::error_handler_t::replace) + "\r\n");
						});
					}
					else if (handler->index() == 1)
					{
						(std::get<1>(*handler))(postMap, request, [response](const rapidjson::Document& data)
						{
							if (data.IsNull())
							{
								response->End();
								return;
							}

							rapidjson::StringBuffer sb;
							rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

							if (!data.Accept(writer))
							{
								response->End();
								return;
							}

							sb.Put('\r');
							sb.Put('\n');

							// for TCP write timeout bits, write this in chunks
							constexpr size_t kChunkSize = 16384;

							for (size_t i = 0; i < sb.GetLength(); i += kChunkSize)
							{
								response->Write(std::string{ sb.GetString() + i, std::min(kChunkSize, sb.GetLength() - i) });
							}
						});
					}
				};

				if (g_threadedHttpVar->GetValue())
				{
					static tp::ThreadPool tg;
					tg.post(runTask);
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
		g_maxClientEndpointRequestSize = instance->AddVariable<int>("sv_maxClientEndpointRequestSize", ConVar_None, 1024 * 100);

		instance->SetComponent(new fx::ClientMethodRegistry());

		instance->GetComponent<fx::HttpServerManager>()->AddEndpoint("/client", fx::GetClientEndpointHandler(instance));
	});
});
