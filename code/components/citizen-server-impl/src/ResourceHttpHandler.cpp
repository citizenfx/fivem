#include "StdInc.h"

#include <HttpServerManager.h>

#include <ServerInstanceBase.h>
#include <ServerInstanceBaseRef.h>

#include <ResourceCallbackComponent.h>

#include <ResourceManager.h>

#include <ScriptEngine.h>

#include <optional>

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

		MSGPACK_DEFINE_MAP(headers, method, address, path, setDataHandler);
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

			requestWrap.setDataHandler = cbComponent->CreateCallback([=](const msgpack::unpacked& unpacked)
			{
				auto callback = unpacked.get().as<std::vector<msgpack::object>>()[0];

				if (callback.type == msgpack::type::EXT)
				{
					if (callback.via.ext.type() == 10 || callback.via.ext.type() == 11)
					{
						fx::FunctionRef functionRef{ std::string{callback.via.ext.data(), callback.via.ext.size} };

						request->SetDataHandler(make_shared_function([this, functionRef = std::move(functionRef)](const std::vector<uint8_t>& bodyArray)
						{
							m_resource->GetManager()->CallReference<void>(functionRef.GetRef(), std::string(bodyArray.begin(), bodyArray.end()));
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
});
