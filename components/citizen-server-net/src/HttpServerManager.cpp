#include "StdInc.h"
#include <ServerInstanceBase.h>
#include <TcpListenManager.h>

#include <HttpServer.h>
#include <HttpServerImpl.h>

#include <TLSServer.h>

class LovelyHttpHandler : public net::HttpHandler
{
public:
	virtual bool HandleRequest(fwRefContainer<net::HttpRequest> request, fwRefContainer<net::HttpResponse> response) override
	{
		response->SetHeader(std::string("Content-Type"), std::string("text/plain"));

		if (request->GetRequestMethod() == "GET")
		{
			response->End(std::string(request->GetPath()));
		}
		else if (request->GetRequestMethod() == "POST")
		{
			request->SetDataHandler([=](const std::vector<uint8_t>& data)
			{
				std::string dataStr(data.begin(), data.end());
				response->End(dataStr);
			});
		}

		return true;
	}
};

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		fx::TcpListenManager* listenManager = Instance<fx::TcpListenManager>::Get(instance->GetInstanceRegistry());

		listenManager->OnInitializeMultiplexServer.Connect([=](fwRefContainer<net::MultiplexTcpServer> server)
		{
			fwRefContainer<net::HttpServer> impl = new net::HttpServerImpl();
			impl->AttachToServer(server->CreateServer([](const std::vector<uint8_t>& bytes)
			{
				if (bytes.size() > 10)
				{
					auto firstR = std::find(bytes.begin(), bytes.end(), '\r');

					if (firstR != bytes.end())
					{
						auto firstN = firstR + 1;

						if (firstN != bytes.end())
						{
							if (*firstN == '\n')
							{
								std::string match(firstR - 8, firstR);

								if (match.find("HTTP/") == 0)
								{
									return net::MultiplexPatternMatchResult::Match;
								}
							}

							return net::MultiplexPatternMatchResult::NoMatch;
						}
					}
				}

				return net::MultiplexPatternMatchResult::InsufficientData;
			}));

			fwRefContainer<net::TLSServer> tlsServer = new net::TLSServer(server->CreateServer([](const std::vector<uint8_t>& bytes)
			{
				if (bytes.size() >= 6)
				{
					return (bytes[0] == 0x16 && bytes[5] == 1) ? net::MultiplexPatternMatchResult::Match : net::MultiplexPatternMatchResult::NoMatch;
				}

				return net::MultiplexPatternMatchResult::InsufficientData;
			}), "citizen/ros/ros.crt", "citizen/ros/ros.key");
			tlsServer->AddRef();

			impl->AttachToServer(tlsServer);

			impl->AddRef();

			static fwRefContainer<net::HttpHandler> rc = new LovelyHttpHandler();
			impl->RegisterHandler(rc);
		});
	});
});