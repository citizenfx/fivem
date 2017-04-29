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
#include <deque>
#include <iomanip>
#include <memory>
#include <sstream>

#include <boost/algorithm/string.hpp>

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
		ReadStateBody,
		ReadStateChunked
	};

	struct HttpConnectionData
	{
		HttpConnectionReadState readState;

		std::deque<uint8_t> readBuffer;

		std::vector<uint8_t> requestData;

		size_t lastLength;

		phr_header headers[50];

		phr_chunked_decoder decoder;

		fwRefContainer<HttpRequest> request;

		fwRefContainer<HttpResponse> response;

		int contentLength;

		HttpConnectionData()
			: readState(ReadStateRequest), lastLength(0), contentLength(0)
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
		if (readQueue.size() > (1024 * 1024 * 5))
		{
			stream->Close();
			return;
		}

		// actually copy
		std::copy(data.begin(), data.end(), readQueue.begin() + origSize);

		// process request data until there's no need anymore
		bool continueProcessing = true;

		while (continueProcessing)
		{
			// depending on the state, perform an action
			if (localConnectionData->readState == ReadStateRequest)
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
											   &path, &pathLength, &minorVersion, localConnectionData->headers, &numHeaders, localConnectionData->lastLength);

				if (result > 0)
				{
					// prepare data for a request instance
					std::string requestMethodStr(requestMethod, requestMethodLength);
					std::string pathStr(path, pathLength);

					HeaderMap headerList;

					for (int i = 0; i < numHeaders; i++)
					{
						auto& header = localConnectionData->headers[i];

						headerList.insert(std::make_pair(std::string(header.name, header.name_len), std::string(header.value, header.value_len)));
					}

					// remove the original bytes from the queue
					readQueue.erase(readQueue.begin(), readQueue.begin() + result);
					localConnectionData->lastLength = 0;

					// store the request in a request instance
					fwRefContainer<HttpRequest> request = new HttpRequest(1, minorVersion, requestMethodStr, pathStr, headerList);
					fwRefContainer<HttpResponse> response = new HttpResponse(stream, request);

					for (auto& handler : m_handlers)
					{
						if (handler->HandleRequest(request, response) || response->HasEnded())
						{
							break;
						}
					}

					continueProcessing = (readQueue.size() > 0);

					if (!response->HasEnded())
					{
						// check to see if we'll have to read user data
						static std::string contentLengthKey = "content-length";
						static std::string contentLengthDefault = "0";

						auto& contentLengthStr = request->GetHeader(contentLengthKey, contentLengthDefault);
						int contentLength = atoi(contentLengthStr.c_str());

						if (contentLength > 0)
						{
							localConnectionData->request = request;
							localConnectionData->response = response;
							localConnectionData->contentLength = contentLength;

							localConnectionData->readState = ReadStateBody;
						}
						else
						{
							static std::string transferEncodingKey = "transfer-encoding";
							static std::string transferEncodingDefault = "";
							static std::string transferEncodingComparison = "chunked";

							if (request->GetHeader(transferEncodingKey, transferEncodingDefault) == transferEncodingComparison)
							{
								localConnectionData->request = request;
								localConnectionData->response = response;
								localConnectionData->contentLength = -1;

								localConnectionData->lastLength = 0;

								localConnectionData->readState = ReadStateChunked;

								memset(&localConnectionData->decoder, 0, sizeof(localConnectionData->decoder));
								localConnectionData->decoder.consume_trailer = true;
							}
						}
					}
				}
				else if (result == -1)
				{
					// should probably send 'bad request'?
					stream->Close();
					return;
				}
				else if (result == -2)
				{
					localConnectionData->lastLength = requestData.size();

					continueProcessing = false;
				}
			}
			else if (connectionData->readState == ReadStateBody)
			{
				int contentLength = localConnectionData->contentLength;

				if (readQueue.size() >= contentLength)
				{
					// copy the deque into a vector for data purposes, again
					std::vector<uint8_t> requestData(readQueue.begin(), readQueue.begin() + contentLength);

					// remove the original bytes from the queue
					readQueue.erase(readQueue.begin(), readQueue.begin() + contentLength);

					// call the data handler
					auto& dataHandler = localConnectionData->request->GetDataHandler();

					if (dataHandler)
					{
						dataHandler(requestData);

						localConnectionData->request->SetDataHandler(std::function<void(const std::vector<uint8_t>&)>());
					}

					// clean up the req/res
					localConnectionData->request = nullptr;
					localConnectionData->response = nullptr;

					localConnectionData->readState = ReadStateRequest;

					continueProcessing = (readQueue.size() > 0);
				}
				else
				{
					continueProcessing = false;
				}
			}
			else if (connectionData->readState == ReadStateChunked)
			{
				// append the remnant of the read queue to the vector
				auto& requestData = localConnectionData->requestData;

				size_t addedSize = readQueue.size() - requestData.size();
				size_t oldSize = requestData.size();
				requestData.resize(readQueue.size());

				// copy the appendant
				std::copy(readQueue.begin() + oldSize, readQueue.end(), requestData.begin() + localConnectionData->lastLength);

				// decode stuff
				size_t requestSize = addedSize;

				int result = phr_decode_chunked(&localConnectionData->decoder, reinterpret_cast<char*>(&requestData[localConnectionData->lastLength]), &requestSize);

				if (result == -2)
				{
					localConnectionData->lastLength += requestSize;

					continueProcessing = false;
				}
				else if (result == -1)
				{
					stream->Close();
					return;
				}
				else
				{
					// call the data handler
					auto& dataHandler = localConnectionData->request->GetDataHandler();

					if (dataHandler)
					{
						requestData.resize(localConnectionData->lastLength);

						dataHandler(requestData);

						localConnectionData->request->SetDataHandler(std::function<void(const std::vector<uint8_t>&)>());
					}

					// clean up the req/res
					localConnectionData->request = nullptr;
					localConnectionData->response = nullptr;

					localConnectionData->requestData.clear();

					localConnectionData->readState = ReadStateRequest;

					continueProcessing = (readQueue.size() > 0);
				}
			}
		}
	});
}

HttpRequest::HttpRequest(int httpVersionMajor, int httpVersionMinor, const std::string& requestMethod, const std::string& path, const HeaderMap& headerList)
	: m_httpVersionMajor(httpVersionMajor), m_httpVersionMinor(httpVersionMinor), m_requestMethod(requestMethod), m_path(path), m_headerList(headerList)
{
}

HttpRequest::~HttpRequest()
{
	SetDataHandler(std::function<void(const std::vector<uint8_t>&)>());
}

HttpResponse::HttpResponse(fwRefContainer<TcpServerStream> clientStream, fwRefContainer<HttpRequest> request)
	: m_clientStream(clientStream), m_ended(false), m_statusCode(200), m_sentHeaders(false), m_request(request), m_closeConnection(false)
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

void HttpResponse::WriteHead(int statusCode, const HeaderMap& headers)
{
	return HttpResponse::WriteHead(statusCode, std::string(""), headers);
}

void HttpResponse::WriteHead(int statusCode, const std::string& statusMessage)
{
	return HttpResponse::WriteHead(statusCode, statusMessage, HeaderMap());
}

void HttpResponse::WriteHead(int statusCode, const std::string& statusMessage, const HeaderMap& headers)
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

	auto requestConnection = m_request->GetHeader(std::string("connection"), std::string("close"));

	if (_stricmp(requestConnection.c_str(), "keep-alive") != 0)
	{
		outData << "Connection: close\r\n";

		m_closeConnection = true;
	}
	else
	{
		outData << "Connection: keep-alive\r\n";
	}

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
	SetHeader(std::string("Content-Length"), std::to_string(data.size()));

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
	if (m_closeConnection)
	{
		m_clientStream->Close();
	}
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
