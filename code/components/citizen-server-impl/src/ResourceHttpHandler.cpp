#include "StdInc.h"

#include <HttpServerManager.h>

#include <ServerInstanceBase.h>
#include <ServerInstanceBaseRef.h>

#include <KeyedRateLimiter.h>

#include <ResourceCallbackComponent.h>

#include <ResourceManager.h>

#include <ScriptEngine.h>

#include <optional>

#include <json.hpp>
#include <cfx_version.h>

#include <boost/algorithm/string.hpp>

#include <GameServerComms.h>

#include <MonoThreadAttachment.h>
#include <SharedFunction.h>

#include <TcpListenManager.h>

// HTTP handler
static auto GetHttpHandler(fx::Resource* resource)
{
	return [=](const fwRefContainer<net::HttpRequest>& request, fwRefContainer<net::HttpResponse> response)
	{

	};
}

class ResourceHttpComponent : public fwRefCountable, public fx::IAttached<fx::Resource>
{
private:
	struct RequestWrap
	{
		std::map<std::string, std::string> headers;
		std::string method;
		std::string address;
		std::string path;

		fx::ResourceCallbackComponent::CallbackRef setDataHandler;
		fx::ResourceCallbackComponent::CallbackRef setCancelHandler;

		MSGPACK_DEFINE_MAP(headers, method, address, path, setDataHandler, setCancelHandler);
	};

	struct ResponseWrap
	{
		fx::ResourceCallbackComponent::CallbackRef write;
		fx::ResourceCallbackComponent::CallbackRef writeHead;
		fx::ResourceCallbackComponent::CallbackRef send;

		MSGPACK_DEFINE_MAP(write, writeHead, send);
	};

public:
	virtual void AttachToObject(fx::Resource* object) override;

	void HandleRequest(const fwRefContainer<net::HttpRequest>& request, fwRefContainer<net::HttpResponse> response)
	{
		auto limiter = m_resource->GetManager()->GetComponent<fx::ServerInstanceBaseRef>()->Get()->GetComponent<fx::PeerAddressRateLimiterStore>()->GetRateLimiter("http_" + m_resource->GetName(), fx::RateLimiterDefaults{ 10.0, 25.0 });
		auto address = request->GetRemotePeer();

		if (!fx::IsProxyAddress(address) && !limiter->Consume(address))
		{
			response->SetStatusCode(429);
			response->SetHeader("Content-Type", "text/plain; charset=utf-8");
			response->End("Rate limit exceeded.");

			return;
		}

		// get the local path for the request
		auto rl = m_endpointPrefix.length();

		if (!boost::algorithm::ends_with(m_endpointPrefix, "/"))
		{
			rl++;
		}

		auto path = std::string{ request->GetPath().c_str() };

		auto localPath = (path.length() >= rl) ? path.substr(rl) : "";

		// pass to the registered handler for the resource
		if (!m_handlerRef)
		{
			response->SetStatusCode(404);
			response->SetHeader("Content-Type", "text/plain; charset=utf-8");
			response->End("Not found.");

			return;
		}

		gscomms_execute_callback_on_main_thread([this, localPath, request, response]
		{
			auto cbComponent = m_resource->GetManager()->GetComponent<fx::ResourceCallbackComponent>();

			RequestWrap requestWrap;

			ResponseWrap responseWrap;

			std::map<std::string, std::string> headers;

			for (auto& pair : request->GetHeaders())
			{
				headers.insert({ std::string{ pair.first.c_str() }, std::string{ pair.second.c_str() } });
			}

			requestWrap.headers = headers;
			requestWrap.method = std::string{ request->GetRequestMethod().c_str() };
			requestWrap.address = request->GetRemoteAddress();
			requestWrap.path = "/" + localPath;

			requestWrap.setCancelHandler = cbComponent->CreateCallback([=](const msgpack::unpacked& unpacked)
			{
				auto args = unpacked.get().as<std::vector<msgpack::object>>();

				auto callback = args[0];

				if (callback.type == msgpack::type::EXT)
				{
					if (callback.via.ext.type() == 10 || callback.via.ext.type() == 11)
					{
						fx::FunctionRef functionRef{ std::string{callback.via.ext.data(), callback.via.ext.size} };

						request->SetCancelHandler(make_shared_function([this, functionRef = std::move(functionRef)]() mutable
						{
							gscomms_execute_callback_on_main_thread(make_shared_function([this, functionRef = std::move(functionRef)]()
							{
								m_resource->GetManager()->CallReference<void>(functionRef.GetRef());
							}));
						}));
					}
				}
			});

			requestWrap.setDataHandler = cbComponent->CreateCallback([=](const msgpack::unpacked& unpacked)
			{
				auto args = unpacked.get().as<std::vector<msgpack::object>>();

				auto callback = args[0];
				bool isBinary = false;

				if (args.size() > 1)
				{
					isBinary = (args[1].as<std::string>() == "binary");
				}

				if (callback.type == msgpack::type::EXT)
				{
					if (callback.via.ext.type() == 10 || callback.via.ext.type() == 11)
					{
						fx::FunctionRef functionRef{ std::string{callback.via.ext.data(), callback.via.ext.size} };

						request->SetDataHandler(make_shared_function([this, functionRef = std::move(functionRef), isBinary](const std::vector<uint8_t>& bodyArray) mutable
						{
							gscomms_execute_callback_on_main_thread(make_shared_function([this, functionRef = std::move(functionRef), isBinary, bodyArray]()
							{
								if (isBinary)
								{
									m_resource->GetManager()->CallReference<void>(functionRef.GetRef(), bodyArray);
								}
								else
								{
									m_resource->GetManager()->CallReference<void>(functionRef.GetRef(), std::string(bodyArray.begin(), bodyArray.end()));
								}
							}));
						}));
					}
				}
			});

			responseWrap.write = cbComponent->CreateCallback([=](const msgpack::unpacked& unpacked)
			{
				response->Write(unpacked.get().as<std::vector<msgpack::object>>()[0].as<std::string>());
			});

			responseWrap.writeHead = cbComponent->CreateCallback([=](const msgpack::unpacked& unpacked)
			{
				auto state = unpacked.get().as<std::vector<msgpack::object>>();

				if (state.size() == 1)
				{
					response->SetStatusCode(state[0].as<int>());
				}
				else
				{
					net::HeaderMap headers;

					for (auto& pair : state[1].as<std::map<std::string, msgpack::object>>())
					{
						if (pair.second.type == msgpack::type::ARRAY)
						{
							response->SetHeader(net::HeaderString{ pair.first.c_str() }, pair.second.as<std::vector<std::string>>());
						}
						else
						{
							response->SetHeader(net::HeaderString{ pair.first.c_str() }, pair.second.as<std::string>());
						}
					}

					response->SetStatusCode(state[0].as<int>());
				}

				if (request->GetHttpVersion() != std::pair<int, int>{ 1, 0 })
				{
					response->WriteHead(response->GetStatusCode());
				}
			});

			responseWrap.send = cbComponent->CreateCallback([=](const msgpack::unpacked& unpacked)
			{
				auto state = unpacked.get().as<std::vector<msgpack::object>>();

				if (state.empty())
				{
					response->End();
				}
				else
				{
					response->End(state[0].as<std::string>());
				}
			});

			//MonoEnsureThreadAttached();

			m_resource->GetManager()->CallReference<void>(*m_handlerRef, requestWrap, responseWrap);
		});
	}

	inline void SetHandlerRef(const std::string& ref)
	{
		m_handlerRef = ref;
	}

private:
	fx::Resource* m_resource;

	std::string m_endpointPrefix;

	std::optional<std::string> m_handlerRef;
};

void ResourceHttpComponent::AttachToObject(fx::Resource* object)
{
	m_resource = object;

	object->OnStart.Connect([this]()
	{
		// get the server from the resource
		fx::ServerInstanceBase* server = m_resource->GetManager()->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's HTTP manager
		fwRefContainer<fx::HttpServerManager> httpManager = server->GetComponent<fx::HttpServerManager>();

		// #TODOMONITOR: *really* make helper
		auto monitorVar = m_resource->GetManager()->GetComponent<fx::ServerInstanceBaseRef>()->Get()->GetComponent<console::Context>()->GetVariableManager()->FindEntryRaw("monitorMode");
		m_endpointPrefix = fmt::sprintf("/%s/", m_resource->GetName());

		if (monitorVar && monitorVar->GetValue() != "0" && m_resource->GetName() == "monitor")
		{
			m_endpointPrefix = "";
		}

		// add an endpoint
		httpManager->AddEndpoint(
			m_endpointPrefix,
			std::bind(&ResourceHttpComponent::HandleRequest, this, std::placeholders::_1, std::placeholders::_2));
	}, 9999);

	object->OnStop.Connect([this]()
	{
		// get the server from the resource
		fx::ServerInstanceBase* server = m_resource->GetManager()->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's HTTP manager
		fwRefContainer<fx::HttpServerManager> httpManager = server->GetComponent<fx::HttpServerManager>();

		// remove an endpoint
		if (!m_endpointPrefix.empty())
		{
			httpManager->RemoveEndpoint(m_endpointPrefix);
		}
	}, -9999);
}

DECLARE_INSTANCE_TYPE(ResourceHttpComponent);

static InitFunction initFunction([]()
{
	// on resource start, register a resource-specific HTTP handler
	// and unregister on stop
	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
	{
		resource->SetComponent(new ResourceHttpComponent());
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_HTTP_HANDLER", [](fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			if (resource)
			{
				resource->GetComponent<ResourceHttpComponent>()->SetHandlerRef(context.GetArgument<const char*>(0));
			}
		}
	});

	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->GetComponent<fx::HttpServerManager>()->AddEndpoint("/", [=](const fwRefContainer<net::HttpRequest>& request, fwRefContainer<net::HttpResponse> response)
		{
			auto webVar = instance->GetComponent<console::Context>()->GetVariableManager()->FindEntryRaw("web_baseUrl");

			if (webVar)
			{
				auto wvv = webVar->GetValue();
				std::string_view wvvv{
					wvv
				};

				auto endPos = wvvv.find(".users.cfx.re");

				if (endPos != std::string::npos)
				{
					auto startPos = wvvv.rfind("-", endPos);

					if (startPos != std::string::npos)
					{
						auto webUrl = fmt::sprintf("https://cfx.re/join/%s", wvvv.substr(startPos + 1, endPos - (startPos + 1)));

						response->SetStatusCode(302);
						response->SetHeader("Location", webUrl);

						response->End("Redirecting...");
						return;
					}
				}
			}

			auto data = nlohmann::json::object(
				{
					{ "version", "FXServer-" GIT_DESCRIPTION }
				}
			);

			response->End(data.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
		});
	});
});
