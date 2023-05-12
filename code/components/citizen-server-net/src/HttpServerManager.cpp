#include "StdInc.h"

#include <HttpServerManager.h>

#include <TcpListenManager.h>
#include <HttpServerImpl.h>
#include <TLSServer.h>

#include <boost/algorithm/string.hpp>

fwEvent<fwRefContainer<net::MultiplexTcpServer>> OnCreateTlsMultiplex;

namespace fx
{
	HttpServerManager::HttpServerManager()
	{
		m_httpHandler = new Handler();
		m_httpHandler->handler = [this] (fwRefContainer<net::HttpRequest> request, fwRefContainer<net::HttpResponse> response)
		{
			{
				std::shared_lock lock(m_handlersMutex);

				for (auto& [ prefix, handler ] : m_handlers)
				{
					bool matches = false;

					if (prefix != "/")
					{
						matches = _strnicmp(request->GetPath().c_str(), prefix.c_str(), prefix.length()) == 0;

						if (!matches)
						{
							if (boost::algorithm::ends_with(prefix, "/"))
							{
								net::HeaderStringView prefixView{ prefix.c_str(), prefix.size() };
								matches = request->GetPath() == prefixView.substr(0, prefixView.length() - 1);
							}
						}
					}
					else
					{
						matches = request->GetPath().compare(prefix.c_str()) == 0;
					}

					if (matches)
					{
						lock.unlock();

						handler(request, response);
						return true;
					}
				}
			}

			response->SetStatusCode(404);
			response->End(fmt::sprintf("Route %s not found.", std::string_view{ request->GetPath().c_str() }));

			return true;
		};

		m_httpServer = new net::HttpServerImpl();
		m_httpServer->RegisterHandler(m_httpHandler);

		m_http2Server = new net::Http2ServerImpl();
		m_http2Server->RegisterHandler(m_httpHandler);
	}

	HttpServerManager::~HttpServerManager()
	{

	}

	void HttpServerManager::AddEndpoint(const std::string& prefix, const TEndpointHandler& handler)
	{
		std::unique_lock lock(m_handlersMutex);

		m_handlers.insert({ prefix, handler });
	}

	void HttpServerManager::RemoveEndpoint(const std::string& prefix)
	{
		std::unique_lock lock(m_handlersMutex);

		m_handlers.erase(prefix);
	}

	void HttpServerManager::AttachToObject(ServerInstanceBase* instance)
	{
		fwRefContainer<fx::TcpListenManager> listenManager = instance->GetComponent<fx::TcpListenManager>();

		listenManager->OnInitializeMultiplexServer.Connect([=](fwRefContainer<net::MultiplexTcpServer> server)
		{
			auto httpPatternMatcher = [](const std::vector<uint8_t>& bytes)
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

					// length safety check
					if (bytes.size() > 4096)
					{
						// 4096-byte+ but no HTTP/? that's awkward
						return net::MultiplexPatternMatchResult::NoMatch;
					}

					// try to avoid classifying non-HTTP traffic as maybe-HTTP
					auto spacePos = std::find(bytes.begin(), bytes.end(), ' ');

					// method shouldn't be more than 64 bytes
					if (std::distance(bytes.begin(), spacePos) > 64)
					{
						return net::MultiplexPatternMatchResult::NoMatch;
					}

					// should start with a letter
					if (bytes[0] < 'A' || bytes[0] > 'Z')
					{
						return net::MultiplexPatternMatchResult::NoMatch;
					}

					for (auto it = bytes.begin(); it != spacePos; it++)
					{
						// HTTP methods *usually* should contain A-Z or - only
						if ((*it < 'A' || *it > 'Z') && *it != '-')
						{
							return net::MultiplexPatternMatchResult::NoMatch;
						}
					}
				}

				return net::MultiplexPatternMatchResult::InsufficientData;
			};

			m_httpServer->AttachToServer(server->CreateServer(httpPatternMatcher));

			fwRefContainer<net::TLSServer> tlsServer = new net::TLSServer(server->CreateServer([](const std::vector<uint8_t>& bytes)
			{
				if (bytes.size() >= 6)
				{
					return (bytes[0] == 0x16 && bytes[5] == 1) ? net::MultiplexPatternMatchResult::Match : net::MultiplexPatternMatchResult::NoMatch;
				}

				return net::MultiplexPatternMatchResult::InsufficientData;
			}), "server-tls.crt", "server-tls.key", true);

			tlsServer->AddRef();

			tlsServer->SetProtocolList({ "h2", "http/1.1" });

			m_http2Server->AttachToServer(tlsServer->GetProtocolServer("h2"));
			m_httpServer->AttachToServer(tlsServer->GetProtocolServer("http/1.1"));

			// create a TLS multiplex for the default protocol
			fwRefContainer<net::MultiplexTcpServer> tlsMultiplex = new net::MultiplexTcpServer();
			m_httpServer->AttachToServer(tlsMultiplex->CreateServer(httpPatternMatcher));

			OnCreateTlsMultiplex(tlsMultiplex);

			tlsMultiplex->AttachToServer(tlsServer);
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
