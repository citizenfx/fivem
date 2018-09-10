/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "HttpServer.h"
#include "HttpServerImpl.h"
#include "TLSServer.h"

#include <ctime>
#include <iomanip>
#include <memory>
#include <sstream>

#include <boost/algorithm/string.hpp>

namespace net
{
void HttpServer::RegisterHandler(fwRefContainer<HttpHandler> handler)
{
	m_handlers.push_front(handler);
}

void HttpServer::AttachToServer(fwRefContainer<TcpServer> server)
{
	server->SetConnectionCallback([=](auto conn)
	{
		OnConnection(conn);
	});
}

HttpRequest::HttpRequest(int httpVersionMajor, int httpVersionMinor, const std::string& requestMethod, const std::string& path, const HeaderMap& headerList, const std::string& remoteAddress)
	: m_httpVersionMajor(httpVersionMajor), m_httpVersionMinor(httpVersionMinor), m_requestMethod(requestMethod), m_path(path), m_headerList(headerList), m_remoteAddress(remoteAddress)
{
}

HttpRequest::~HttpRequest()
{
	SetDataHandler(std::function<void(const std::vector<uint8_t>&)>());
}

HttpResponse::HttpResponse(fwRefContainer<HttpRequest> request)
	: m_ended(false), m_statusCode(200), m_sentHeaders(false), m_request(request), m_closeConnection(false)
{

}

std::string HttpResponse::GetHeader(const std::string& name)
{
	auto it = m_headerList.find(name);

	return (it == m_headerList.end()) ? std::string() : it->second;
}

void HttpResponse::SetHeader(const std::string& name, const std::string& value)
{
	m_headerList.erase(name);
	m_headerList.insert({ name, value });
}

void HttpResponse::SetHeader(const std::string& name, const std::vector<std::string>& values)
{
	m_headerList.erase(name);

	for (const auto& value : values)
	{
		m_headerList.insert({ name, value });
	}
}

void HttpResponse::RemoveHeader(const std::string& name)
{
	m_headerList.erase(name);
}

void HttpResponse::WriteHead(int statusCode)
{
	return WriteHead(statusCode, std::string(""));
}

void HttpResponse::WriteHead(int statusCode, const HeaderMap& headers)
{
	return WriteHead(statusCode, std::string(""), headers);
}

void HttpResponse::WriteHead(int statusCode, const std::string& statusMessage)
{
	return WriteHead(statusCode, statusMessage, HeaderMap());
}

void HttpResponse::BeforeWriteHead(const std::string& data)
{
}

void HttpResponse::Write(const std::string& data)
{
	BeforeWriteHead(data);

	if (!m_sentHeaders)
	{
		WriteHead(m_statusCode);
	}

	std::vector<uint8_t> dataBuffer(data.size());
	memcpy(&dataBuffer[0], data.c_str(), data.size());

	WriteOut(dataBuffer);
}

void HttpResponse::End(const std::string& data)
{
	Write(data);
	End();
}


std::string HttpResponse::GetStatusMessage(int statusCode)
{
	static std::map<int, std::string> httpStatuses;

	if (httpStatuses.empty())
	{
		httpStatuses[100] = "Continue";
		httpStatuses[101] = "Switching Protocols";
		httpStatuses[200] = "OK";
		httpStatuses[201] = "Created";
		httpStatuses[202] = "Accepted";
		httpStatuses[203] = "Non-Authoritative Information";
		httpStatuses[204] = "No Content";
		httpStatuses[205] = "Reset Content";
		httpStatuses[206] = "Partial Content";
		httpStatuses[300] = "Multiple Choices";
		httpStatuses[301] = "Moved Permanently";
		httpStatuses[302] = "Found";
		httpStatuses[303] = "See Other";
		httpStatuses[304] = "Not Modified";
		httpStatuses[305] = "Use Proxy";
		httpStatuses[307] = "Temporary Redirect";
		httpStatuses[400] = "Bad Request";
		httpStatuses[401] = "Unauthorized";
		httpStatuses[402] = "Payment Required";
		httpStatuses[403] = "Forbidden";
		httpStatuses[404] = "Not Found";
		httpStatuses[405] = "Method Not Allowed";
		httpStatuses[406] = "Not Acceptable";
		httpStatuses[407] = "Proxy Authentication Required";
		httpStatuses[408] = "Request Time-out";
		httpStatuses[409] = "Conflict";
		httpStatuses[410] = "Gone";
		httpStatuses[411] = "Length Required";
		httpStatuses[412] = "Precondition Failed";
		httpStatuses[413] = "Request Entity Too Large";
		httpStatuses[414] = "Request-URI Too Large";
		httpStatuses[415] = "Unsupported Media Type";
		httpStatuses[416] = "Requested Range not Satisfiable";
		httpStatuses[417] = "Expectation Failed";
		httpStatuses[422] = "Unprocessable Entity";
		httpStatuses[429] = "Too Many Requests";
		httpStatuses[500] = "Internal Server Error";
		httpStatuses[501] = "Not Implemented";
		httpStatuses[502] = "Bad Gateway";
		httpStatuses[503] = "Service Unavailable";
		httpStatuses[504] = "Gateway Time-out";
		httpStatuses[505] = "HTTP Version not Supported";

	}

	return httpStatuses[statusCode];
}
}

#include "MultiplexTcpServer.h"
#include "TcpServerManager.h"

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
			request->SetDataHandler([=] (const std::vector<uint8_t>& data)
			{
				std::string dataStr(data.begin(), data.end());
				response->End(dataStr);
			});
		}

		return true;
	}
};

/*
static InitFunction initFunction([] ()
{
	static fwRefContainer<net::TcpServerManager> tcpStack = new net::TcpServerManager();
	static fwRefContainer<net::TcpServer> tcpServer = tcpStack->CreateServer(net::PeerAddress::FromString("localhost:8081").get());

	//static fwRefContainer<net::TLSServer> tlsServer = new net::TLSServer(tcpServer);

	static fwRefContainer<net::HttpServer> impl = new net::HttpServerImpl();
	impl->AttachToServer(tcpServer);

	static fwRefContainer<net::HttpHandler> rc = new LovelyHttpHandler();
	impl->RegisterHandler(rc);

	impl->AddRef();
	tcpServer->AddRef();
	tcpStack->AddRef();

	//std::this_thread::sleep_for(std::chrono::seconds(3600));
});
*/
