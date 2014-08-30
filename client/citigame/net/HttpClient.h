#pragma once

#include <functional>
#include <mutex>
#include <queue>
#include <winhttp.h>

#include "fiDevice.h"

class HttpClient
{
public:
	typedef std::pair<std::wstring, uint16_t> ServerPair;

private:
	HINTERNET hWinHttp;

	std::multimap<ServerPair, HINTERNET> m_connections;

	std::queue<std::function<void(HINTERNET)>> m_connectionFreeCBs;

	std::mutex m_connectionMutex;

	static void CALLBACK StatusCallback(HINTERNET handle, DWORD_PTR context, DWORD code, void* info, DWORD length);

	HINTERNET GetConnection(ServerPair server);

	void QueueOnConnectionFree(std::function<void(HINTERNET)> cb);

public:
	HttpClient();
	virtual ~HttpClient();

	std::string BuildPostString(std::map<std::string, std::string>& fields);

	void ReaddConnection(ServerPair server, HINTERNET connection);

	bool CrackUrl(std::string url, std::wstring& hostname, std::wstring& path, uint16_t& port);

	void DoPostRequest(std::wstring host, uint16_t port, std::wstring url, std::map<std::string, std::string>& fields, std::function<void(bool, std::string)> callback);
	void DoPostRequest(std::wstring host, uint16_t port, std::wstring url, std::string postData, std::function<void(bool, std::string)> callback);

	void DoFileGetRequest(std::wstring host, uint16_t port, std::wstring url, const char* outDeviceBase, std::string outFilename, std::function<void(bool, std::string)> callback);
	void DoFileGetRequest(std::wstring host, uint16_t port, std::wstring url, rage::fiDevice* outDevice, std::string outFilename, std::function<void(bool, std::string)> callback, HANDLE hConnection = nullptr);
};