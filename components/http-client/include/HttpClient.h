/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <functional>
#include <mutex>
#include <queue>
#include <winhttp.h>

#include "fiDevice.h"

#ifdef COMPILING_HTTP_CLIENT
#define HTTP_EXPORT __declspec(dllexport)
#else
#define HTTP_EXPORT
#endif

class HTTP_EXPORT HttpClient
{
friend struct HttpClientRequestContext;

public:
	typedef std::pair<fwWString, uint16_t> ServerPair;

private:
	HINTERNET hWinHttp;

	std::multimap<ServerPair, HINTERNET> m_connections;

	std::queue<fwAction<HINTERNET>> m_connectionFreeCBs;

	std::mutex m_connectionMutex;

	static void CALLBACK StatusCallback(HINTERNET handle, DWORD_PTR context, DWORD code, void* info, DWORD length);

	HINTERNET GetConnection(ServerPair server);

	void QueueOnConnectionFree(fwAction<HINTERNET> cb);

	void ReaddConnection(ServerPair server, HINTERNET connection);

public:
	HttpClient();
	virtual ~HttpClient();

	fwString BuildPostString(fwMap<fwString, fwString>& fields);

	bool CrackUrl(fwString url, fwWString& hostname, fwWString& path, uint16_t& port);

	void DoGetRequest(fwWString host, uint16_t port, fwWString url, fwAction<bool, const char*, size_t> callback);

	void DoPostRequest(fwWString host, uint16_t port, fwWString url, fwMap<fwString, fwString>& fields, fwAction<bool, const char*, size_t> callback);
	void DoPostRequest(fwWString host, uint16_t port, fwWString url, fwString postData, fwAction<bool, const char*, size_t> callback);

	void DoFileGetRequest(fwWString host, uint16_t port, fwWString url, const char* outDeviceBase, fwString outFilename, fwAction<bool, const char*, size_t> callback);
	void DoFileGetRequest(fwWString host, uint16_t port, fwWString url, rage::fiDevice* outDevice, fwString outFilename, fwAction<bool, const char*, size_t> callback, HANDLE hConnection = nullptr);
};