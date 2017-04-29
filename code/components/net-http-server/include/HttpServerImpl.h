/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "HttpServer.h"

#include <forward_list>

namespace net
{
class 
#ifdef COMPILING_NET_HTTP_SERVER
	DLL_EXPORT
#endif
	HttpServerImpl : public HttpServer
{
private:
	fwRefContainer<TcpServer> m_server;

	std::forward_list<fwRefContainer<HttpHandler>> m_handlers;

private:
	void OnConnection(fwRefContainer<TcpServerStream> stream);

public:
	HttpServerImpl();

	virtual ~HttpServerImpl() override;

	virtual void AttachToServer(fwRefContainer<TcpServer> server) override;

	virtual void RegisterHandler(fwRefContainer<HttpHandler> handler) override;
};
}