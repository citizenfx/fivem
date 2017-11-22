/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
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
	virtual void OnConnection(fwRefContainer<TcpServerStream> stream) override;

public:
	HttpServerImpl();

	virtual ~HttpServerImpl() override;
};

class
#ifdef COMPILING_NET_HTTP_SERVER
	DLL_EXPORT
#endif
Http2ServerImpl: public HttpServer
{
private:
	virtual void OnConnection(fwRefContainer<TcpServerStream> stream) override;

public:
	Http2ServerImpl();

	virtual ~Http2ServerImpl() override;
};
}
