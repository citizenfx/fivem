/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ros/EndpointMapper.h>
#include <ros/LoopbackTcpServer.h>

#include <TcpServerManager.h>
#include <TLSServer.h>

#include <HttpServerImpl.h>

void EndpointMapper::AddPrefix(const std::string& routeOrigin, fwRefContainer<net::HttpHandler> handler)
{
	m_prefixes.insert({ routeOrigin, handler });
}

void EndpointMapper::AddGameService(const std::string& serviceName, const TGameServiceHandler& handler)
{
	m_gameServices.insert({ serviceName, handler });
}

bool EndpointMapper::HandleRequest(fwRefContainer<net::HttpRequest> request, fwRefContainer<net::HttpResponse> response)
{
	trace(__FUNCTION__ ": %s %s\n", request->GetRequestMethod().c_str(), request->GetPath().c_str());

	for (auto& prefixEntry : m_prefixes)
	{
		if (_strnicmp(request->GetPath().c_str(), prefixEntry.first.c_str(), prefixEntry.first.length()) == 0)
		{
			return prefixEntry.second->HandleRequest(request, response);
		}
	}

	response->SetStatusCode(404);
	response->End(va("Route %s not found.", request->GetPath().c_str()));

	return true;
}

boost::optional<TGameServiceHandler> EndpointMapper::GetGameServiceHandler(const std::string& serviceName)
{
	for (auto& handlerEntry : m_gameServices)
	{
		if (_stricmp(serviceName.c_str(), handlerEntry.first.c_str()) == 0)
		{
			return handlerEntry.second;
		}
	}

	return boost::optional<TGameServiceHandler>();
}

static InitFunction initFunction([] ()
{
	// create the endpoint mapper
	EndpointMapper* endpointMapper = new EndpointMapper();
	endpointMapper->AddRef();

	Instance<EndpointMapper>::Set(endpointMapper);

	// register the endpoint mapper on a pair of HTTP servers
	LoopbackTcpServerManager* tcpServerManager = Instance<LoopbackTcpServerManager>::Get();
	
	net::HttpServer* httpServer = new net::HttpServerImpl();
	httpServer->AddRef();
	httpServer->RegisterHandler(endpointMapper);

	// create the HTTP (non-TLS) backend server
	fwRefContainer<LoopbackTcpServer> insecureServer = tcpServerManager->RegisterTcpServer("prod.ros.rockstargames.com");
	insecureServer->AddRef();
	insecureServer->SetPort(80);

	// create the TLS backend server
	fwRefContainer<LoopbackTcpServer> secureServer = tcpServerManager->RegisterTcpServer("prod.ros.rockstargames.com");
	secureServer->AddRef();
	secureServer->SetPort(443);

	// create auth-prod TLS servers
	fwRefContainer<LoopbackTcpServer> insecureServer2 = tcpServerManager->RegisterTcpServer("auth-prod.ros.rockstargames.com");
	insecureServer2->AddRef();
	insecureServer2->SetPort(80);

	// create the TLS backend server
	fwRefContainer<LoopbackTcpServer> secureServer2 = tcpServerManager->RegisterTcpServer("auth-prod.ros.rockstargames.com");
	secureServer2->AddRef();
	secureServer2->SetPort(443);

	// create auth-gta5-prod TLS servers
	fwRefContainer<LoopbackTcpServer> insecureServer3 = tcpServerManager->RegisterTcpServer("auth-gta5-prod.ros.rockstargames.com");
	insecureServer3->AddRef();
	insecureServer3->SetPort(80);

	// create the TLS backend server
	fwRefContainer<LoopbackTcpServer> secureServer3 = tcpServerManager->RegisterTcpServer("auth-gta5-prod.ros.rockstargames.com");
	secureServer3->AddRef();
	secureServer3->SetPort(443);

	// create the local socket server, if enabled
	if (wcsstr(GetCommandLine(), L"ros:legit") != nullptr)
	{
		net::PeerAddress address = net::PeerAddress::FromString("localhost:32891").get();

		fwRefContainer<net::TcpServerFactory> manager = new net::TcpServerManager();
		fwRefContainer<net::TcpServer> tcpServer = manager->CreateServer(address);

		manager->AddRef();
		tcpServer->AddRef();

		httpServer->AttachToServer(tcpServer);
	}

	// create the TLS wrapper for the TLS backend
	net::TLSServer* tlsWrapper = new net::TLSServer(secureServer, "citizen/ros/ros.crt", "citizen/ros/ros.key");
	tlsWrapper->AddRef();

	net::TLSServer* tlsWrapper2 = new net::TLSServer(secureServer2, "citizen/ros/ros.crt", "citizen/ros/ros.key");
	tlsWrapper2->AddRef();

	net::TLSServer* tlsWrapper3 = new net::TLSServer(secureServer3, "citizen/ros/ros.crt", "citizen/ros/ros.key");
	tlsWrapper3->AddRef();

	// attach the endpoint mappers
	httpServer->AttachToServer(tlsWrapper);
	httpServer->AttachToServer(tlsWrapper2);
	httpServer->AttachToServer(tlsWrapper3);
	httpServer->AttachToServer(insecureServer);
	httpServer->AttachToServer(insecureServer2);
	httpServer->AttachToServer(insecureServer3);
}, -500);
