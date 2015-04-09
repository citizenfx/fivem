/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "HttpServer.h"
#include "HttpServerImpl.h"

#include <ctime>
#include <deque>
#include <iomanip>
#include <memory>
#include <sstream>

#include <picohttpparser.h>

namespace net
{
HttpServerImpl::HttpServerImpl()
{

}

HttpServerImpl::~HttpServerImpl()
{

}

void HttpServerImpl::AttachToServer(fwRefContainer<TcpServer> server)
{
	server->SetConnectionCallback(std::bind(&HttpServerImpl::OnConnection, this, std::placeholders::_1));
}

void HttpServerImpl::RegisterHandler(fwRefContainer<HttpHandler> handler)
{
	m_handlers.push_front(handler);
}

void HttpServerImpl::OnConnection(fwRefContainer<TcpServerStream> stream)
{
	enum HttpConnectionReadState
	{
		ReadStateRequest,
		ReadStateBody
	};

	struct HttpConnectionData
	{
		HttpConnectionReadState readState;

		std::deque<uint8_t> readBuffer;

		size_t lastLength;

		phr_header headers[50];

		HttpConnectionData()
			: readState(ReadStateRequest), lastLength(0)
		{

		}
	};

	std::shared_ptr<HttpConnectionData> connectionData = std::make_shared<HttpConnectionData>();

	stream->SetReadCallback([=] (const std::vector<uint8_t>& data)
	{
		// keep a reference to the connection data locally
		std::shared_ptr<HttpConnectionData> localConnectionData = connectionData;

		// place bytes in the read buffer
		auto& readQueue = connectionData->readBuffer;

		size_t origSize = readQueue.size();
		readQueue.resize(origSize + data.size());

		// close the stream if the length is too big
		if (readQueue.size() > (1024 * 1024))
		{
			stream->Close();
			return;
		}

		// actually copy
		std::copy(data.begin(), data.end(), readQueue.begin() + origSize);

		// depending on the state, perform an action
		if (connectionData->readState == ReadStateRequest)
		{
			// copy the deque into a vector for data purposes
			std::vector<uint8_t> requestData(readQueue.begin(), readQueue.end());

			// define output variables
			const char* requestMethod;
			size_t requestMethodLength;

			const char* path;
			size_t pathLength;

			int minorVersion;
			size_t numHeaders = 50;

			int result = phr_parse_request(reinterpret_cast<const char*>(&requestData[0]), requestData.size(), &requestMethod, &requestMethodLength,
										   &path, &pathLength, &minorVersion, connectionData->headers, &numHeaders, connectionData->lastLength);

			if (result > 0)
			{
				// prepare data for a request instance
				std::string requestMethodStr(requestMethod, requestMethodLength);
				std::string pathStr(path, pathLength);

				std::map<std::string, std::string> headerList;
				
				for (int i = 0; i < numHeaders; i++)
				{
					auto& header = connectionData->headers[i];

					headerList.insert(std::make_pair(std::string(header.name, header.name_len), std::string(header.value, header.value_len)));
				}

				// remove the original bytes from the queue
				readQueue.erase(readQueue.begin(), readQueue.begin() + result);

				// store the request in a request instance
				fwRefContainer<HttpRequest> request = new HttpRequest(1, minorVersion, requestMethodStr, pathStr, headerList);
				fwRefContainer<HttpResponse> response = new HttpResponse(stream);
				
				for (auto& handler : m_handlers)
				{
					if (handler->HandleRequest(request, response) || response->HasEnded())
					{
						break;
					}
				}
			}
			else if (result == -1)
			{
				// should probably send 'bad request'?
				stream->Close();
				return;
			}

			localConnectionData->lastLength = requestData.size();
		}
	});
}

HttpRequest::HttpRequest(int httpVersionMajor, int httpVersionMinor, const std::string& requestMethod, const std::string& path, const std::map<std::string, std::string>& headerList)
	: m_httpVersionMajor(httpVersionMajor), m_httpVersionMinor(httpVersionMinor), m_requestMethod(requestMethod), m_path(path), m_headerList(headerList)
{

}

HttpResponse::HttpResponse(fwRefContainer<TcpServerStream> clientStream)
	: m_clientStream(clientStream), m_ended(false), m_statusCode(200), m_sentHeaders(false)
{

}

std::string HttpResponse::GetHeader(const std::string& name)
{
	auto it = m_headerList.find(name);

	return (it == m_headerList.end()) ? std::string() : it->second;
}

void HttpResponse::SetHeader(const std::string& name, const std::string& value)
{
	m_headerList[name] = value;
}

void HttpResponse::RemoveHeader(const std::string& name)
{
	m_headerList.erase(name);
}

void HttpResponse::WriteHead(int statusCode)
{
	return HttpResponse::WriteHead(statusCode, std::string(""));
}

void HttpResponse::WriteHead(int statusCode, const std::map<std::string, std::string>& headers)
{
	return HttpResponse::WriteHead(statusCode, std::string(""), headers);
}

void HttpResponse::WriteHead(int statusCode, const std::string& statusMessage)
{
	return HttpResponse::WriteHead(statusCode, statusMessage, std::map<std::string, std::string>());
}

void HttpResponse::WriteHead(int statusCode, const std::string& statusMessage, const std::map<std::string, std::string>& headers)
{
	if (m_sentHeaders)
	{
		return;
	}

	std::ostringstream outData;
	outData.imbue(std::locale());

	outData << "HTTP/1.0 " << std::to_string(statusCode) << " " << (statusMessage.empty() ? GetStatusMessage(statusCode) : statusMessage) << "\r\n";

	auto& usedHeaders = (headers.size() == 0) ? m_headerList : headers;
	
	if (usedHeaders.find("date") == usedHeaders.end())
	{
		std::time_t timeVal;
		std::time(&timeVal);

		std::tm time = *std::gmtime(&timeVal);
		outData << "Date: " << std::put_time(&time, "%a, %d %b %Y %H:%M:%S %Z") << "\r\n";
	}

	outData << "Connection: close\r\n";

	for (auto& header : usedHeaders)
	{
		outData << header.first << ": " << header.second << "\r\n";
	}

	outData << "\r\n";

	std::string outStr = outData.str();

	std::vector<uint8_t> dataBuffer(outStr.size());
	memcpy(&dataBuffer[0], outStr.c_str(), outStr.size());

	m_clientStream->Write(dataBuffer);

	m_sentHeaders = true;
}

void HttpResponse::Write(const std::string& data)
{
	if (!m_sentHeaders)
	{
		WriteHead(m_statusCode);
	}

	std::vector<uint8_t> dataBuffer(data.size());
	memcpy(&dataBuffer[0], data.c_str(), data.size());

	m_clientStream->Write(dataBuffer);
}

void HttpResponse::End(const std::string& data)
{
	Write(data);
	End();
}

void HttpResponse::End()
{
	// TODO: handle keep-alive
	m_clientStream->Close();
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
		response->End(std::string(request->GetPath()));

		return true;
	}
};

static InitFunction initFunction([] ()
{
	fwRefContainer<net::TcpServerManager> tcpStack = new net::TcpServerManager();
	fwRefContainer<net::TcpServer> tcpServer = tcpStack->CreateServer(net::PeerAddress::FromString("localhost:8081").get());

	fwRefContainer<net::HttpServer> impl = new net::HttpServerImpl();
	impl->AttachToServer(tcpServer);

	fwRefContainer<net::HttpHandler> rc = new LovelyHttpHandler();
	impl->RegisterHandler(rc);

	std::this_thread::sleep_for(std::chrono::seconds(3600));
});