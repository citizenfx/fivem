/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "HttpClient.h"
#include "fiDevice.h"
#include <sstream>

HttpClient::HttpClient(const wchar_t* userAgent)
{
	hWinHttp = WinHttpOpen(userAgent, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, WINHTTP_FLAG_ASYNC);
	WinHttpSetTimeouts(hWinHttp, 2000, 5000, 5000, 5000);
}

HttpClient::~HttpClient()
{
	WinHttpCloseHandle(hWinHttp);
}

void HttpClient::DoPostRequest(fwWString host, uint16_t port, fwWString url, fwMap<fwString, fwString>& fields, fwAction<bool, const char*, size_t> callback)
{
	fwString postData = BuildPostString(fields);

	DoPostRequest(host, port, url, postData, callback);
}

struct HttpClientRequestContext
{
	HttpClient* client;
	HttpClient::ServerPair server;
	HINTERNET hConnection;
	HINTERNET hRequest;
	fwString postData;
	fwAction<bool, const char*, size_t> callback;

	std::stringstream resultData;
	char buffer[32768];

	rage::fiDevice* outDevice;
	int outHandle;

	HttpClientRequestContext()
		: outDevice(nullptr)
	{

	}

	void DoCallback(bool success, fwString& resData)
	{
		if (outDevice)
		{
			outDevice->Close(outHandle);
		}

		callback(success, resData.c_str(), resData.size());

		if (server.second)
		{
			client->ReaddConnection(server, hConnection);
		}
		else
		{
			WinHttpCloseHandle(hConnection);
		}

		WinHttpCloseHandle(hRequest);

		delete this;
	}
};

void HttpClient::DoPostRequest(fwWString host, uint16_t port, fwWString url, fwString postData, fwAction<bool, const char*, size_t> callback)
{
	HINTERNET hConnection = WinHttpConnect(hWinHttp, host.c_str(), port, 0);
	HINTERNET hRequest = WinHttpOpenRequest(hConnection, L"POST", url.c_str(), 0, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

	WinHttpSetStatusCallback(hRequest, StatusCallback, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, 0);

	HttpClientRequestContext* context = new HttpClientRequestContext;
	context->client = this;
	context->hConnection = hConnection;
	context->hRequest = hRequest;
	context->postData = postData;
	context->callback = callback;

	WinHttpSendRequest(hRequest, L"Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n", -1, const_cast<char*>(context->postData.c_str()), context->postData.length(), context->postData.length(), (DWORD_PTR)context);
}

void HttpClient::DoGetRequest(fwWString host, uint16_t port, fwWString url, fwAction<bool, const char*, size_t> callback)
{
	HINTERNET hConnection = WinHttpConnect(hWinHttp, host.c_str(), port, 0);
	HINTERNET hRequest = WinHttpOpenRequest(hConnection, L"GET", url.c_str(), 0, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

	WinHttpSetStatusCallback(hRequest, StatusCallback, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, 0);

	HttpClientRequestContext* context = new HttpClientRequestContext;
	context->client = this;
	context->hConnection = hConnection;
	context->hRequest = hRequest;
	context->callback = callback;

	WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, nullptr, 0, 0, (DWORD_PTR)context);
}

void HttpClient::DoFileGetRequest(fwWString host, uint16_t port, fwWString url, const char* outDeviceBase, fwString outFilename, fwAction<bool, const char*, size_t> callback)
{
	DoFileGetRequest(host, port, url, rage::fiDevice::GetDevice(outDeviceBase, true), outFilename, callback);
}

void HttpClient::ReaddConnection(ServerPair server, HINTERNET connection)
{
	m_connectionMutex.lock();

	if (!m_connectionFreeCBs.empty())
	{
		auto cb = m_connectionFreeCBs.front();
		m_connectionFreeCBs.pop();

		m_connectionMutex.unlock();

		cb(connection);

		return;
	}
	else
	{
		auto range = m_connections.equal_range(server);

		for (auto& it = range.first; it != range.second; it++)
		{
			if (!it->second)
			{
				it->second = connection;

				m_connectionMutex.unlock();

				return;
			}
		}
	}

	m_connectionMutex.unlock();
}

HINTERNET HttpClient::GetConnection(ServerPair server)
{
	auto range = m_connections.equal_range(server);

	int numConnections = 0;

	for (auto& it = range.first; it != range.second; it++)
	{
		numConnections++;

		if (it->second)
		{
			auto conn = it->second;
			it->second = nullptr;

			return conn;
		}
	}

	if (numConnections >= 8)
	{
		return nullptr;
	}

	HINTERNET hConnection = WinHttpConnect(hWinHttp, server.first.c_str(), server.second, 0);

	if (!hConnection)
	{
		return INVALID_HANDLE_VALUE;
	}

	m_connections.insert(std::make_pair(server, nullptr));

	return hConnection;
}

void HttpClient::QueueOnConnectionFree(fwAction<HINTERNET> cb)
{
	m_connectionFreeCBs.push(cb);

	// FIXME: possible race condition if a request just completed?
}

void HttpClient::DoFileGetRequest(fwWString host, uint16_t port, fwWString url, rage::fiDevice* outDevice, fwString outFilename, fwAction<bool, const char*, size_t> callback, HANDLE hConnection)
{
	ServerPair pair = std::make_pair(host, port);

	m_connectionMutex.lock();

	if (!hConnection)
	{
		hConnection = GetConnection(pair);

		if (hConnection == INVALID_HANDLE_VALUE)
		{
			callback(false, "", 0);

			m_connectionMutex.unlock();

			return;
		}

		if (!hConnection)
		{
			QueueOnConnectionFree([=] (HINTERNET connection)
			{
				DoFileGetRequest(host, port, url, outDevice, outFilename, callback, connection);
			});

			m_connectionMutex.unlock();

			return;
		}
	}

	m_connectionMutex.unlock();

	//HINTERNET hConnection = WinHttpConnect(hWinHttp, host.c_str(), port, 0);
	HINTERNET hRequest = WinHttpOpenRequest(hConnection, L"GET", url.c_str(), 0, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

	WinHttpSetStatusCallback(hRequest, StatusCallback, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, 0);

	HttpClientRequestContext* context = new HttpClientRequestContext;
	context->client = this;
	context->hConnection = hConnection;
	context->hRequest = hRequest;
	context->callback = callback;
	context->outDevice = outDevice;
	context->server = pair;

	if (context->outDevice == nullptr)
	{
		GlobalError("context->outDevice was null in " __FUNCTION__);
		return;
	}

	context->outHandle = context->outDevice->Create(outFilename.c_str());

	WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, nullptr, 0, 0, (DWORD_PTR)context);
}

void HttpClient::StatusCallback(HINTERNET handle, DWORD_PTR context, DWORD code, void* info, DWORD length)
{
	HttpClientRequestContext* ctx = (HttpClientRequestContext*)context;

	switch (code)
	{
		case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
			ctx->DoCallback(false, fwString());
			break;

		case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
			if (!WinHttpReceiveResponse(ctx->hRequest, 0))
			{
				ctx->DoCallback(false, fwString());
			}

			break;

		case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
		{
			uint32_t statusCode;
			DWORD statusCodeLength = sizeof(uint32_t);

			if (!WinHttpQueryHeaders(ctx->hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeLength, WINHTTP_NO_HEADER_INDEX))
			{
				ctx->DoCallback(false, fwString());
				return;
			}

			if (statusCode != HTTP_STATUS_OK)
			{
				ctx->DoCallback(false, fwString());
				return;
			}

			if (!WinHttpReadData(ctx->hRequest, ctx->buffer, sizeof(ctx->buffer) - 1, nullptr))
			{
				ctx->DoCallback(false, fwString());
			}

			break;
		}
		case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
			if (ctx->outDevice)
			{
				ctx->outDevice->Write(ctx->outHandle, ctx->buffer, length);
			}
			else
			{
				ctx->buffer[length] = '\0';

				ctx->resultData << fwString(ctx->buffer, length);
			}

			if (length > 0)
			{
				if (!WinHttpReadData(ctx->hRequest, ctx->buffer, sizeof(ctx->buffer) - 1, nullptr))
				{
					ctx->DoCallback(false, fwString());
				}
			}
			else
			{
				std::string str = ctx->resultData.str();
				ctx->DoCallback(true, fwString(str.c_str(), str.size()));
			}

			break;
	}
}

bool HttpClient::CrackUrl(fwString url, fwWString& hostname, fwWString& path, uint16_t& port)
{
	wchar_t wideUrl[1024];
	mbstowcs(wideUrl, url.c_str(), _countof(wideUrl));
	wideUrl[1023] = L'\0';

	URL_COMPONENTS components = { 0 };
	components.dwStructSize = sizeof(components);

	components.dwHostNameLength = -1;
	components.dwUrlPathLength = -1;
	components.dwExtraInfoLength = -1;

	if (!WinHttpCrackUrl(wideUrl, wcslen(wideUrl), 0, &components))
	{
		return false;
	}

	hostname = fwWString(components.lpszHostName, components.dwHostNameLength);
	path = fwWString(components.lpszUrlPath, components.dwUrlPathLength);
	path += fwWString(components.lpszExtraInfo, components.dwExtraInfoLength);
	port = components.nPort;

	return true;
}

fwString HttpClient::BuildPostString(fwMap<fwString, fwString>& fields)
{
	std::stringstream retval;

	for (auto& field : fields)
	{
		retval << field.first << "=" << url_encode(field.second) << "&";
	}

	fwString str = fwString(retval.str().c_str());
	return str.substr(0, str.length() - 1);
}