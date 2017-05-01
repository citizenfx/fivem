#pragma once

#include <ServerInstanceBase.h>

#include <HttpServer.h>

namespace fx
{
	class HttpServerManager : public fwRefCountable, public IAttached<ServerInstanceBase>
	{
	public:
		HttpServerManager();

		virtual ~HttpServerManager();

	public:
		using TEndpointHandler = std::function<void(const fwRefContainer<net::HttpRequest>& request, fwRefContainer<net::HttpResponse> response)>;

		void AddEndpoint(const std::string& prefix, const TEndpointHandler& handler);

		virtual void AttachToObject(ServerInstanceBase* instance);

	private:
		struct Handler : public net::HttpHandler
		{
			std::function<bool(fwRefContainer<net::HttpRequest>, fwRefContainer<net::HttpResponse>)> handler;

			virtual bool HandleRequest(fwRefContainer<net::HttpRequest> request, fwRefContainer<net::HttpResponse> response) override;
		};

	private:
		fwRefContainer<net::HttpServer> m_httpServer;

		fwRefContainer<Handler> m_httpHandler;

		std::map<std::string, TEndpointHandler> m_handlers;
	};
}

DECLARE_INSTANCE_TYPE(fx::HttpServerManager);