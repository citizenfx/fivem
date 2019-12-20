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

#include <MonoThreadAttachment.h>

// HTTP handler
static auto GetHttpHandler(fx::Resource* resource)
{
	return [=](const fwRefContainer<net::HttpRequest>& request, fwRefContainer<net::HttpResponse> response)
	{

	};
}

// blindly copypasted from StackOverflow (to allow std::function to store the funcref types with their move semantics)
// TODO: we use this twice now, time for a shared header?
template<class F>
struct shared_function
{
	std::shared_ptr<F> f;
	shared_function() = default;
	shared_function(F&& f_) : f(std::make_shared<F>(std::move(f_))) {}
	shared_function(shared_function const&) = default;
	shared_function(shared_function&&) = default;
	shared_function& operator=(shared_function const&) = default;
	shared_function& operator=(shared_function&&) = default;

	template<class...As>
	auto operator()(As&&...as) const
	{
		return (*f)(std::forward<As>(as)...);
	}
};

template<class F>
shared_function<std::decay_t<F>> make_shared_function(F&& f)
{
	return { std::forward<F>(f) };
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

		if (!limiter->Consume(*net::PeerAddress::FromString(request->GetRemoteAddress())))
		{
			response->SetStatusCode(429);
			response->SetHeader("Content-Type", "text/plain; charset=utf-8");
			response->End("Rate limit exceeded.");

			return;
		}

		// get the local path for the request
		auto localPath = request->GetPath().substr(m_resource->GetName().length() + 2);

		// pass to the registered handler for the resource
		if (m_handlerRef)
		{
			auto cbComponent = m_resource->GetManager()->GetComponent<fx::ResourceCallbackComponent>();

			RequestWrap requestWrap;

			ResponseWrap responseWrap;

			std::map<std::string, std::string> headers;

			for (auto& pair : request->GetHeaders())
			{
				headers.insert(pair);
			}

			requestWrap.headers = headers;
			requestWrap.method = request->GetRequestMethod();
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

						request->SetCancelHandler(make_shared_function([this, functionRef = std::move(functionRef)]()
						{
							m_resource->GetManager()->CallReference<void>(functionRef.GetRef());
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

						request->SetDataHandler(make_shared_function([this, functionRef = std::move(functionRef), isBinary](const std::vector<uint8_t>& bodyArray)
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
							response->SetHeader(pair.first, pair.second.as<std::vector<std::string>>());
						}
						else
						{
							response->SetHeader(pair.first, pair.second.as<std::string>());
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

			MonoEnsureThreadAttached();
			m_resource->GetManager()->CallReference<void>(*m_handlerRef, requestWrap, responseWrap);
		}
	}

	inline void SetHandlerRef(const std::string& ref)
	{
		m_handlerRef = ref;
	}

private:
	fx::Resource* m_resource;

	std::optional<std::string> m_handlerRef;
};

void ResourceHttpComponent::AttachToObject(fx::Resource* object)
{
	m_resource = object;

	object->OnStart.Connect([this]()
	{
		// workaround
		if (m_resource->GetName() == "_cfx_internal")
		{
			return;
		}

		// get the server from the resource
		fx::ServerInstanceBase* server = m_resource->GetManager()->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's HTTP manager
		fwRefContainer<fx::HttpServerManager> httpManager = server->GetComponent<fx::HttpServerManager>();

		// add an endpoint
		httpManager->AddEndpoint(
			fmt::sprintf("/%s/", m_resource->GetName()),
			std::bind(&ResourceHttpComponent::HandleRequest, this, std::placeholders::_1, std::placeholders::_2));
	}, 9999);

	object->OnStop.Connect([this]()
	{
		// workaround
		if (m_resource->GetName() == "_cfx_internal")
		{
			return;
		}

		// get the server from the resource
		fx::ServerInstanceBase* server = m_resource->GetManager()->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's HTTP manager
		fwRefContainer<fx::HttpServerManager> httpManager = server->GetComponent<fx::HttpServerManager>();

		// remove an endpoint
		httpManager->RemoveEndpoint(fmt::sprintf("/%s/", m_resource->GetName()));
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
		instance->GetComponent<fx::HttpServerManager>()->AddEndpoint("/", [=](const fwRefContainer<net::HttpRequest>& request, const fwRefContainer<net::HttpResponse>& response)
		{
			auto resource = instance->GetComponent<fx::ResourceManager>()->GetResource("webadmin");

			if (resource.GetRef() && resource->GetState() == fx::ResourceState::Started)
			{
				response->SetStatusCode(302);
				response->SetHeader("Location", "/webadmin/");

				response->End("Redirecting...");
				return;
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
