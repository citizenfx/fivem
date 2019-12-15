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

#include <CfxState.h>
#include <HostSharedData.h>

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

DLL_EXPORT fwEvent<net::TcpServer*> OnConfigureWebSocket;

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

	if (wcsstr(GetCommandLine(), L"ros:legit") == nullptr && wcsstr(GetCommandLine(), L"ros:steam") == nullptr)
	{
		auto domains = {
			"prod.ros.rockstargames.com",
			"auth-prod.ros.rockstargames.com",
			"auth-gta5-prod.ros.rockstargames.com",
			"rgl-prod.ros.rockstargames.com",
			"auth-rgl-prod.ros.rockstargames.com",
			"ps-rgl-prod.ros.rockstargames.com",
			"prs-rgl-prod.ros.rockstargames.com",
			"app-rgl-prod.ros.rockstargames.com",
			"crews-rdr2-prod.ros.rockstargames.com",
			"prs-rdr2-prod.ros.rockstargames.com",
			"ugc-rdr2-prod.ros.rockstargames.com",
			"inbox-rdr2-prod.ros.rockstargames.com",
			"tm-rdr2-prod.ros.rockstargames.com",
			"posse-rdr2-prod.ros.rockstargames.com",
			"feed-rdr2-prod.ros.rockstargames.com",
			"conductor-rdr2-prod.ros.rockstargames.com",
			"challenges-rdr2-prod.ros.rockstargames.com",
			"prod-locator-cloud.rockstargames.com",
			"www.google-analytics.com",
		};

		for (auto& domain : domains)
		{
			// create the HTTP (non-TLS) backend server
			fwRefContainer<LoopbackTcpServer> insecureServer = tcpServerManager->RegisterTcpServer(domain);
			insecureServer->AddRef();
			insecureServer->SetPort(80);

			// create the TLS backend server
			fwRefContainer<LoopbackTcpServer> secureServer = tcpServerManager->RegisterTcpServer(domain);
			secureServer->AddRef();
			secureServer->SetPort(443);

			// create the TLS wrapper for the TLS backend
			net::TLSServer* tlsWrapper = new net::TLSServer(secureServer, "citizen/ros/ros.crt", "citizen/ros/ros.key");
			tlsWrapper->AddRef();

			// attach the endpoint mappers
			httpServer->AttachToServer(tlsWrapper);
			httpServer->AttachToServer(insecureServer);
		}

		{
			static fwRefContainer<LoopbackTcpServer> wsServer = tcpServerManager->RegisterTcpServer("cfx-web-rdr2-prod.ros.rockstargames.com");
			wsServer->AddRef();
			wsServer->SetPort(80);

			static HookFunction hf([]()
			{
				OnConfigureWebSocket(wsServer.GetRef());
			});
		}
	}

	// create the local socket server, if this is the master process
	static HostSharedData<CfxState> initState("CfxInitState");

#ifdef IS_RDR3
	if (initState->IsMasterProcess())
#else
	if (wcsstr(GetCommandLine(), L"ros:legit") != nullptr)
#endif
	{
		net::PeerAddress address = net::PeerAddress::FromString("localhost:32891").get();

		fwRefContainer<net::TcpServerFactory> manager = new net::TcpServerManager();
		fwRefContainer<net::TcpServer> tcpServer = manager->CreateServer(address);

		if (tcpServer.GetRef())
		{
			manager->AddRef();
			tcpServer->AddRef();

			httpServer->AttachToServer(tcpServer);
		}
	}
}, -500);
