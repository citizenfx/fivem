/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <HttpServerImpl.h>
#include <TLSServer.h>
#include <ros/LoopbackTcpServer.h>
#include <ros/EndpointMapper.h>

#include <fstream>

class VersioningHandler : public net::HttpHandler
{
private:
	std::map<std::string, std::string> m_data;
	std::string m_url;

public:
	VersioningHandler(const std::string& url, std::initializer_list<std::string> files)
		: m_url(url)
	{
		auto loadData = [&] (const std::string& filename)
		{
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;

			std::vector<char> outBlob;

			FILE* f = _pfopen(MakeRelativeCitPath(L"citizen/ros/" + converter.from_bytes(filename)).c_str(), _P("rb"));
			assert(f);

			fseek(f, 0, SEEK_END);
			outBlob.resize(ftell(f));

			fseek(f, 0, SEEK_SET);
			fread(outBlob.data(), 1, outBlob.size(), f);

			fclose(f);

			m_data[filename] = std::string(outBlob.data(), outBlob.size());
		};

		for (auto& file : files)
		{
			loadData(file);
		}
	}

	bool HandleRequest(fwRefContainer<net::HttpRequest> request, fwRefContainer<net::HttpResponse> response) override
	{
		if (request->GetPath().find(m_url.c_str()) == 0)
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

			response->End(m_data[std::string{ request->GetPath().c_str() }.substr(m_url.length())]);
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
	VersioningHandler* publicHandler = new VersioningHandler("/public/", { "title_metadata.json", "launcher_online_config.xml", "recognised_titles.json", "legacy_titles.json" });
	publicHandler->AddRef();

	LoopbackTcpServerManager* tcpServerManager = Instance<LoopbackTcpServerManager>::Get();

	// create public handler
	net::HttpServer* httpServer2 = new net::HttpServerImpl();
	httpServer2->AddRef();
	httpServer2->RegisterHandler(publicHandler);

	// create the TLS backend server
	fwRefContainer<LoopbackTcpServer> secureServer = tcpServerManager->RegisterTcpServer("gamedownloads-rockstargames-com.akamaized.net");
	secureServer->AddRef();
	secureServer->SetPort(443);

	// create the TLS wrapper for the TLS backend
	net::TLSServer* tlsWrapper = new net::TLSServer(secureServer, "citizen/ros/ros.crt", "citizen/ros/ros.key");
	tlsWrapper->AddRef();

	httpServer2->AttachToServer(tlsWrapper);

	// also register the versioning handler for ROS SCUI
	EndpointMapper* endpointMapper = Instance<EndpointMapper>::Get();
		endpointMapper->AddPrefix("/public/", publicHandler);
});