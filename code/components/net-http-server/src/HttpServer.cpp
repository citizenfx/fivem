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

	WriteOut(data);
}

void HttpResponse::Write(std::string&& data)
{
	BeforeWriteHead(data);

	if (!m_sentHeaders)
	{
		WriteHead(m_statusCode);
	}

	WriteOut(std::move(data));
}

void HttpResponse::WriteOut(const std::string& data)
{
	std::vector<uint8_t> dataBuf(data.size());
	memcpy(dataBuf.data(), data.data(), dataBuf.size());

	WriteOut(std::move(dataBuf));
}

void HttpResponse::WriteOut(std::vector<uint8_t>&& data)
{
	WriteOut(static_cast<const std::vector<uint8_t>&>(data));
}

void HttpResponse::WriteOut(std::string&& data)
{
	WriteOut(static_cast<const std::string&>(data));
}

void HttpResponse::End(const std::string& data)
{
	Write(data);
	End();
}

void HttpResponse::End(std::string&& data)
{
	Write(std::move(data));
	End();
}

static std::map<int, std::string_view> httpStatuses =
{
	{ 100, "Continue" },
	{ 101, "Switching Protocols" },
	{ 200, "OK" },
	{ 201, "Created" },
	{ 202, "Accepted" },
	{ 203, "Non-Authoritative Information" },
	{ 204, "No Content" },
	{ 205, "Reset Content" },
	{ 206, "Partial Content" },
	{ 300, "Multiple Choices" },
	{ 301, "Moved Permanently" },
	{ 302, "Found" },
	{ 303, "See Other" },
	{ 304, "Not Modified" },
	{ 305, "Use Proxy" },
	{ 307, "Temporary Redirect" },
	{ 400, "Bad Request" },
	{ 401, "Unauthorized" },
	{ 402, "Payment Required" },
	{ 403, "Forbidden" },
	{ 404, "Not Found" },
	{ 405, "Method Not Allowed" },
	{ 406, "Not Acceptable" },
	{ 407, "Proxy Authentication Required" },
	{ 408, "Request Time-out" },
	{ 409, "Conflict" },
	{ 410, "Gone" },
	{ 411, "Length Required" },
	{ 412, "Precondition Failed" },
	{ 413, "Request Entity Too Large" },
	{ 414, "Request-URI Too Large" },
	{ 415, "Unsupported Media Type" },
	{ 416, "Requested Range not Satisfiable" },
	{ 417, "Expectation Failed" },
	{ 422, "Unprocessable Entity" },
	{ 429, "Too Many Requests" },
	{ 500, "Internal Server Error" },
	{ 501, "Not Implemented" },
	{ 502, "Bad Gateway" },
	{ 503, "Service Unavailable" },
	{ 504, "Gateway Time-out" },
	{ 505, "HTTP Version not Supported" },
};

std::string_view HttpResponse::GetStatusMessage(int statusCode)
{
	return httpStatuses[statusCode];
}
}
