#pragma once

#include <ServerInstanceBase.h>

#include <HttpServer.h>

#ifdef COMPILING_CITIZEN_SERVER_NET
#define CSNET_EXPORT DLL_EXPORT
#else
#define CSNET_EXPORT DLL_IMPORT
#endif

namespace fx
{
	class CSNET_EXPORT HttpServerManager : public fwRefCountable, public IAttached<ServerInstanceBase>
	{
	public:
		HttpServerManager();

		virtual ~HttpServerManager();

	public:
		using TEndpointHandler = std::function<void(const fwRefContainer<net::HttpRequest>& request, fwRefContainer<net::HttpResponse> response)>;

		virtual void AddEndpoint(const std::string& prefix, const TEndpointHandler& handler);

		virtual void RemoveEndpoint(const std::string& prefix);

		virtual void AttachToObject(ServerInstanceBase* instance);

	private:
		struct Handler : public net::HttpHandler
		{
			std::function<bool(fwRefContainer<net::HttpRequest>, fwRefContainer<net::HttpResponse>)> handler;

			virtual bool HandleRequest(fwRefContainer<net::HttpRequest> request, fwRefContainer<net::HttpResponse> response) override;
		};

	private:
		fwRefContainer<net::HttpServer> m_httpServer;

		fwRefContainer<net::HttpServer> m_http2Server;

		fwRefContainer<Handler> m_httpHandler;

		std::map<std::string, TEndpointHandler> m_handlers;

		std::mutex m_handlersMutex;
	};
}

namespace net
{
	class MultiplexTcpServer;
}

extern
#ifndef COMPILING_CITIZEN_SERVER_NET
	DLL_IMPORT
#else
	DLL_EXPORT	
#endif
	fwEvent<fwRefContainer<net::MultiplexTcpServer>> OnCreateTlsMultiplex;

DECLARE_INSTANCE_TYPE(fx::HttpServerManager);
