#include "StdInc.h"
#include <ServerInstanceBase.h>
#include <HttpServerManager.h>
#include <ClientHttpHandler.h>

#include <CoreConsole.h>
#include <thread_pool.hpp>

#include <json.hpp>
#include <rapidjson/writer.h>

#include <FormData.h>

#include <zlib.h>

using json = nlohmann::json;

static std::shared_ptr<ConVar<bool>> g_threadedHttpVar;
static std::shared_ptr<ConVar<int>> g_maxClientEndpointRequestSize;

namespace
{
bool ClientAcceptsGzip(const fwRefContainer<net::HttpRequest>& request)
{
	const auto header = request->GetHeader("accept-encoding");

	for (size_t i = 0; i + 4 <= header.size(); ++i)
	{
		if ((header[i] | 0x20) == 'g' && (header[i + 1] | 0x20) == 'z' &&
			(header[i + 2] | 0x20) == 'i' && (header[i + 3] | 0x20) == 'p')
		{
			return true;
		}
	}

	return false;
}

std::optional<std::string> GzipCompress(std::string_view input)
{
	z_stream zs{};

	// MAX_WBITS + 16 for the gzip wrapper format
	if (deflateInit2(&zs, Z_BEST_SPEED, Z_DEFLATED, MAX_WBITS + 16, 8, Z_DEFAULT_STRATEGY) != Z_OK)
	{
		return std::nullopt;
	}

	std::string output;
	output.resize(deflateBound(&zs, static_cast<uLong>(input.size())));

	zs.next_in = reinterpret_cast<z_const Bytef*>(const_cast<char*>(input.data()));
	zs.avail_in = static_cast<uInt>(input.size());
	zs.next_out = reinterpret_cast<Bytef*>(output.data());
	zs.avail_out = static_cast<uInt>(output.size());

	const int rc = deflate(&zs, Z_FINISH);
	if (rc != Z_STREAM_END)
	{
		deflateEnd(&zs);
		return std::nullopt;
	}

	output.resize(zs.total_out);
	deflateEnd(&zs);
	return output;
}
}

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
						(std::get<1>(*handler))(postMap, request, [request, response](const rapidjson::Document& data)
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

							// Compress with gzip when client supports it
							constexpr size_t kGzipMinSize = 1024;

							std::string compressedHolder;
							std::string_view bodyView{ sb.GetString(), sb.GetSize() };

							if (bodyView.size() >= kGzipMinSize && ClientAcceptsGzip(request))
							{
								if (auto compressed = GzipCompress(bodyView))
								{
									compressedHolder = std::move(*compressed);
									bodyView = compressedHolder;
									response->SetHeader(net::HeaderString{ "content-encoding" },
										net::HeaderString{ "gzip" });
								}
							}

							// for TCP write timeout bits, write this in chunks
							constexpr size_t kChunkSize = 16384;

							for (size_t i = 0; i < bodyView.size(); i += kChunkSize)
							{
								response->Write(std::string{ bodyView.data() + i,
									std::min(kChunkSize, bodyView.size() - i) });
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
