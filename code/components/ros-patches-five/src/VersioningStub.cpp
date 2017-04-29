/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <HttpServerImpl.h>
#include <ros/LoopbackTcpServer.h>
#include <ros/EndpointMapper.h>

#include <fstream>

class VersioningHandler : public net::HttpHandler
{
private:
	std::map<std::string, std::string> m_data;

public:
	VersioningHandler()
	{
		auto loadData = [&] (const std::string& filename)
		{
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;

			std::ifstream file(MakeRelativeCitPath(L"citizen/ros/" + converter.from_bytes(filename)), std::ios::in | std::ios::binary);

			m_data[filename] = std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
		};

		loadData("versioning.xml");
		loadData("launcher_online_config.xml");

		// SCUI assets
		loadData("splash.png");
		loadData("logotype.ttf");
		loadData("fontawesome-webfont.ttf");
		loadData("font-awesome.css");
		loadData("roboto.ttf");
	}

	bool HandleRequest(fwRefContainer<net::HttpRequest> request, fwRefContainer<net::HttpResponse> response) override
	{
		trace("versioning: %s\n", request->GetPath().c_str());

		if (request->GetPath().find("/prod/gtav/") == 0)
		{
			response->SetStatusCode(200);

			if (request->GetPath().find(".css") != std::string::npos)
			{
				response->SetHeader("Content-Type", "text/css; charset=utf-8");
			}
			else
			{
				response->SetHeader("Content-Type", "text/xml; charset=utf-8");
			}

			response->End(m_data[request->GetPath().substr(11)]);
		}
		else
		{
			response->SetStatusCode(404);

			response->End("nope.");
		}

		return true;
	}
};

static InitFunction initFunction([] ()
{
	// create the handler
	VersioningHandler* handler = new VersioningHandler();
	handler->AddRef();

	LoopbackTcpServerManager* tcpServerManager = Instance<LoopbackTcpServerManager>::Get();

	net::HttpServer* httpServer = new net::HttpServerImpl();
	httpServer->AddRef();
	httpServer->RegisterHandler(handler);

	// create the backend server
	fwRefContainer<LoopbackTcpServer> insecureServer = tcpServerManager->RegisterTcpServer("patches.rockstargames.com");
	insecureServer->AddRef();
	insecureServer->SetPort(80);

	// attach the HTTP server
	httpServer->AttachToServer(insecureServer);

	// also register the versioning handler for ROS SCUI
	EndpointMapper* endpointMapper = Instance<EndpointMapper>::Get();

	endpointMapper->AddPrefix("/prod/gtav/", handler);
});