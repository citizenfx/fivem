/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
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
};
}
