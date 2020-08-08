/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "HttpServer.h"
#include "HttpServerImpl.h"

#include <picohttpparser.h>

#include <ctime>
#include <deque>
#include <iomanip>
#include <sstream>
#include <string_view>

namespace net
{
class Http1Response : public HttpResponse
{
private:
	fwRefContainer<TcpServerStream> m_clientStream;

	std::shared_ptr<HttpState> m_requestState;

	bool m_chunked;

	bool m_sentWriteHead;

	bool m_disableKeepAlive;

public:
	inline Http1Response(fwRefContainer<TcpServerStream> clientStream, fwRefContainer<HttpRequest> request, const std::shared_ptr<HttpState>& reqState, bool disableKeepAlive)
		: HttpResponse(request), m_requestState(reqState), m_clientStream(clientStream), m_chunked(false), m_sentWriteHead(false), m_disableKeepAlive(disableKeepAlive)
	{
		
	}

	virtual void WriteHead(int statusCode, const std::string& statusMessage, const HeaderMap& headers) override
	{
		if (m_sentHeaders)
		{
			return;
		}

		BeforeWriteHead({});

		std::ostringstream outData;
		outData.imbue(std::locale());

		outData << "HTTP/1.1 " << std::to_string(statusCode) << " " << (statusMessage.empty() ? GetStatusMessage(statusCode) : statusMessage) << "\r\n";

		auto usedHeaders = (headers.size() == 0) ? m_headerList : headers;

		if (usedHeaders.find("date") == usedHeaders.end())
		{
			std::time_t timeVal;
			std::time(&timeVal);

			std::tm time = *std::gmtime(&timeVal);
			outData << "Date: " << std::put_time(&time, "%a, %d %b %Y %H:%M:%S GMT") << "\r\n";
		}

		auto requestConnection = m_request->GetHeader("connection", "keep-alive");

		if (_strnicmp(requestConnection.data(), "keep-alive", requestConnection.size()) != 0 || m_disableKeepAlive)
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
			outData << std::string_view{ header.first.data(), header.first.size() } << ": "
					<< std::string_view{ header.second.data(), header.second.size() } << "\r\n";
		}

		outData << "\r\n";

		std::string outStr = outData.str();

		m_clientStream->Write(std::move(outStr));

		m_sentHeaders = true;
	}

	virtual void BeforeWriteHead(size_t length) override
	{
		// only execute WriteHead filtering once
		if (m_sentWriteHead)
		{
			return;
		}

		m_sentWriteHead = true;

		// HACK: disallow chunking for ROS requests
		if (m_request->GetHttpVersion() == std::make_pair(1, 0) ||
			m_request->GetHeader("host").find("rockstargames.com") != std::string::npos ||
			GetHeader("transfer-encoding").find("identity") != std::string::npos)
		{
			if (m_headerList.find("content-length") == m_headerList.end())
			{
				SetHeader("Content-Length", HeaderString{ std::to_string(length).c_str() });
			}

			// unset transfer-encoding, if present
			m_headerList.erase("transfer-encoding");
		}
		else
		{
			SetHeader("Transfer-Encoding", "chunked");

			// if client code set Content-Length, unset it.
			//
			// setting both Transfer-Encoding: chunked and Content-Length is considered ill-formed.
			m_headerList.erase("content-length");

			m_chunked = true;
		}
	}

	private:
	template<typename TContainer>
	void WriteOutInternal(TContainer data, fu2::unique_function<void(bool)>&& cb = {})
	{
		size_t length = data.size();

		if (m_chunked)
		{
			// we _don't_ want to send a 0-sized chunk
			if (length > 0)
			{
				// assume chunked
				m_clientStream->Write(fmt::sprintf("%x\r\n", length));
				m_clientStream->Write(std::forward<TContainer>(data), std::move(cb));
				m_clientStream->Write("\r\n");
			}
		}
		else
		{
			m_clientStream->Write(std::forward<TContainer>(data), std::move(cb));
		}
	}

	public:
	virtual void WriteOut(const std::vector<uint8_t>& data, fu2::unique_function<void(bool)>&& onComplete = {}) override
	{
		WriteOutInternal<decltype(data)>(data, std::move(onComplete));
	}

	virtual void WriteOut(std::vector<uint8_t>&& data, fu2::unique_function<void(bool)>&& onComplete = {}) override
	{
		WriteOutInternal<decltype(data)>(std::move(data), std::move(onComplete));
	}

	virtual void WriteOut(const std::string& data, fu2::unique_function<void(bool)>&& onComplete = {}) override
	{
		WriteOutInternal<decltype(data)>(data, std::move(onComplete));
	}

	virtual void WriteOut(std::string&& data, fu2::unique_function<void(bool)>&& onComplete = {}) override
	{
		WriteOutInternal<decltype(data)>(std::move(data), std::move(onComplete));
	}

	virtual void WriteOut(std::unique_ptr<char[]> data, size_t length, fu2::unique_function<void(bool)>&& cb = {}) override
	{
		if (m_chunked)
		{
			// we _don't_ want to send a 0-sized chunk
			if (length > 0)
			{
				// assume chunked
				m_clientStream->Write(fmt::sprintf("%x\r\n", length));
				m_clientStream->Write(std::move(data), length, std::move(cb));
				m_clientStream->Write("\r\n");
			}
		}
		else
		{
			m_clientStream->Write(std::move(data), length, std::move(cb));
		}
	}

	virtual void End() override
	{
		if (m_chunked && m_clientStream.GetRef())
		{
			// assume chunked
			m_clientStream->Write("0\r\n\r\n");
		}

		if (m_requestState->blocked)
		{
			m_requestState->blocked = false;

			decltype(m_requestState->ping) ping;

			{
				std::unique_lock<std::mutex> lock(m_requestState->pingLock);
				ping = m_requestState->ping;
			}

			if (ping)
			{
				m_clientStream->ScheduleCallback(std::move(ping), false);
			}
		}

		if (m_closeConnection && m_clientStream.GetRef())
		{
			m_clientStream->Close();
		}
	}

	virtual void CloseSocket() override
	{
		auto s = m_clientStream;

		if (s.GetRef())
		{
			s->Close();
		}
	}
};

HttpServerImpl::HttpServerImpl()
{

}

HttpServerImpl::~HttpServerImpl()
{

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

		phr_header headers[50]{ 0 };

		phr_chunked_decoder decoder{ 0 };

		fwRefContainer<HttpRequest> request;

		fwRefContainer<HttpResponse> response;

		int contentLength;

		int pipelineLength;

		bool invalid;

		HttpConnectionData()
			: readState(ReadStateRequest), lastLength(0), contentLength(0), pipelineLength(0), invalid(false)
		{

		}
	};

	std::shared_ptr<HttpConnectionData> connectionData = std::make_shared<HttpConnectionData>();
	std::shared_ptr<HttpState> reqState = std::make_shared<HttpState>();

	std::function<void(const std::vector<uint8_t>&)> readCallback;
	readCallback = [this, stream, connectionData, reqState](const std::vector<uint8_t>& data)
	{
		// keep a reference to the connection data locally
		std::shared_ptr<HttpConnectionData> localConnectionData = connectionData;

		// if the connection is supposed to be closed, don't try using it
		if (localConnectionData->invalid)
		{
			return;
		}

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
			// second check: if the connection is supposed to be closed, don't try using it
			if (localConnectionData->invalid)
			{
				return;
			}

			// depending on the state, perform an action
			if (localConnectionData->readState == ReadStateRequest)
			{
				if (reqState->blocked)
				{
					break;
				}

				// increment the pipeline length, so we can close every 10 requests
				localConnectionData->pipelineLength++;

				// copy the deque into a vector for data purposes
				std::vector<uint8_t> requestData(readQueue.begin(), readQueue.end());

				// define output variables
				const char* requestMethod;
				size_t requestMethodLength;

				const char* path;
				size_t pathLength;

				int minorVersion;
				size_t numHeaders = 50;

				int result = -2;
				
				if (requestData.size() > 0)
				{
					result = phr_parse_request(reinterpret_cast<const char*>(&requestData[0]), requestData.size(), &requestMethod, &requestMethodLength,
					&path, &pathLength, &minorVersion, localConnectionData->headers, &numHeaders, localConnectionData->lastLength);
				}

				if (result > 0)
				{
					// prepare data for a request instance
					HeaderString requestMethodStr(requestMethod, requestMethodLength);
					HeaderString pathStr(path, pathLength);

					HeaderMap headerList;

					for (int i = 0; i < numHeaders; i++)
					{
						auto& header = localConnectionData->headers[i];

						headerList.insert({ { header.name, header.name_len }, { header.value, header.value_len } });
					}

					// remove the original bytes from the queue
					readQueue.erase(readQueue.begin(), readQueue.begin() + result);
					localConnectionData->lastLength = 0;

					// store the request in a request instance
					fwRefContainer<HttpRequest> request = new HttpRequest(1, minorVersion, requestMethodStr, pathStr, headerList, stream->GetPeerAddress());
					fwRefContainer<HttpResponse> response = new Http1Response(stream, request, reqState, localConnectionData->pipelineLength > 10);

					reqState->blocked = true;

					localConnectionData->request = request;
					localConnectionData->response = response;

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
						static HeaderString contentLengthKey = "content-length";
						static std::string contentLengthDefault = "0";

						auto contentLengthStr = request->GetHeader(contentLengthKey, contentLengthDefault);
						int contentLength = atoi(contentLengthStr.data());

						if (contentLength > 0)
						{
							localConnectionData->contentLength = contentLength;

							localConnectionData->readState = ReadStateBody;
						}
						else
						{
							static HeaderString transferEncodingKey = "transfer-encoding";
							static std::string transferEncodingDefault = "";
							static std::string_view transferEncodingComparison = "chunked";

							if (request->GetHeader(transferEncodingKey, transferEncodingDefault) == transferEncodingComparison)
							{
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
					localConnectionData->invalid = true;

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
				// skip if this is an empty write
				if (data.empty())
				{
					break;
				}

				int contentLength = localConnectionData->contentLength;

				if (readQueue.size() >= contentLength)
				{
					// copy the deque into a vector for data purposes, again
					std::vector<uint8_t> requestData(readQueue.begin(), readQueue.begin() + contentLength);

					// remove the original bytes from the queue
					readQueue.erase(readQueue.begin(), readQueue.begin() + contentLength);

					if (localConnectionData->request.GetRef())
					{
						// call the data handler
						auto dataHandler = localConnectionData->request->GetDataHandler();

						if (dataHandler)
						{
							localConnectionData->request->SetDataHandler();

							(*dataHandler)(requestData);
						}
					}

					// clean up the req/res
					//localConnectionData->request = nullptr;
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
				// skip if this is an empty write
				if (data.empty())
				{
					break;
				}

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
					localConnectionData->invalid = true;

					stream->Close();
					return;
				}
				else
				{
					// remove the original bytes from the queue
					readQueue.erase(readQueue.begin(), readQueue.begin() + readQueue.size() - result);

					auto request = localConnectionData->request;

					if (request.GetRef())
					{
						// call the data handler
						auto dataHandler = request->GetDataHandler();

						if (dataHandler)
						{
							request->SetDataHandler();

							requestData.resize(localConnectionData->lastLength);

							(*dataHandler)(requestData);
						}
					}

					// clean up the response
					localConnectionData->response = nullptr;

					localConnectionData->requestData.clear();

					localConnectionData->readState = ReadStateRequest;
					localConnectionData->lastLength = 0;

					continueProcessing = (readQueue.size() > 0);
				}
			}
		}
	};

	{
		std::unique_lock<std::mutex> lock(reqState->pingLock);

		reqState->ping = [stream, readCallback]()
		{
			if (readCallback)
			{
				stream->ScheduleCallback([readCallback]()
				{
					readCallback({});
				});
			}
		};
	}

	stream->SetReadCallback(readCallback);

	stream->SetCloseCallback([=]()
	{
		if (connectionData && connectionData->request.GetRef())
		{
			auto cancelHandler = connectionData->request->GetCancelHandler();

			if (cancelHandler)
			{
				(*cancelHandler)();

				connectionData->request->SetCancelHandler();
			}

			connectionData->request = nullptr;
		}

		if (reqState)
		{
			std::unique_lock<std::mutex> lock(reqState->pingLock);
			reqState->ping = {};
		}
	});
}
}
