#pragma once

#include <functional>
#include <winhttp.h>

class HttpClient
{
private:
	HINTERNET hWinHttp;

	static void CALLBACK StatusCallback(HINTERNET handle, DWORD_PTR context, DWORD code, void* info, DWORD length);

public:
	HttpClient();
	virtual ~HttpClient();

	std::string BuildPostString(std::map<std::string, std::string>& fields);

	void DoPostRequest(std::wstring host, uint16_t port, std::wstring url, std::map<std::string, std::string>& fields, std::function<void(bool, std::string)> callback);
	void DoPostRequest(std::wstring host, uint16_t port, std::wstring url, std::string postData, std::function<void(bool, std::string)> callback);
};