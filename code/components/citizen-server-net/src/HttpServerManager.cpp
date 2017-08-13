#include "StdInc.h"

#include <HttpServerManager.h>

#include <TcpListenManager.h>
#include <HttpServerImpl.h>
#include <TLSServer.h>

namespace fx
{
	HttpServerManager::HttpServerManager()
	{
		m_httpHandler = new Handler();
		m_httpHandler->handler = [=] (fwRefContainer<net::HttpRequest> request, fwRefContainer<net::HttpResponse> response)
		{
			{
				std::unique_lock<std::mutex> lock(m_handlersMutex);

				for (auto& prefixEntry : m_handlers)
				{
					if (_strnicmp(request->GetPath().c_str(), prefixEntry.first.c_str(), prefixEntry.first.length()) == 0)
					{
						lock.unlock();

						prefixEntry.second(request, response);
						return true;
					}
				}
			}

			response->SetStatusCode(404);
			response->End(va("Route %s not found.", request->GetPath().c_str()));

			return true;
		};

		m_httpServer = new net::HttpServerImpl();
		m_httpServer->RegisterHandler(m_httpHandler);
	}

	HttpServerManager::~HttpServerManager()
	{

	}

	void HttpServerManager::AddEndpoint(const std::string& prefix, const TEndpointHandler& handler)
	{
		std::unique_lock<std::mutex> lock(m_handlersMutex);

		m_handlers.insert({ prefix, handler });
	}

	void HttpServerManager::RemoveEndpoint(const std::string& prefix)
	{
		std::unique_lock<std::mutex> lock(m_handlersMutex);

		m_handlers.erase(prefix);
	}

	void HttpServerManager::AttachToObject(ServerInstanceBase* instance)
	{
		fwRefContainer<fx::TcpListenManager> listenManager = instance->GetComponent<fx::TcpListenManager>();

		listenManager->OnInitializeMultiplexServer.Connect([=](fwRefContainer<net::MultiplexTcpServer> server)
		{
			m_httpServer->AttachToServer(server->CreateServer([](const std::vector<uint8_t>& bytes)
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

			/*fwRefContainer<net::TLSServer> tlsServer = new net::TLSServer(server->CreateServer([](const std::vector<uint8_t>& bytes)
			{
			if (bytes.size() >= 6)
			{
			return (bytes[0] == 0x16 && bytes[5] == 1) ? net::MultiplexPatternMatchResult::Match : net::MultiplexPatternMatchResult::NoMatch;
			}

			return net::MultiplexPatternMatchResult::InsufficientData;
			}), "citizen/ros/ros.crt", "citizen/ros/ros.key");

			impl->AttachToServer(tlsServer);*/
		});
	}

	bool HttpServerManager::Handler::HandleRequest(fwRefContainer<net::HttpRequest> request, fwRefContainer<net::HttpResponse> response)
	{
		return handler(request, response);
	}
}

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->SetComponent(new fx::HttpServerManager());
	}, -100);
});
