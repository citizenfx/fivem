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

#include <VFSDevice.h>

#ifdef COMPILING_HTTP_CLIENT
#define HTTP_EXPORT __declspec(dllexport)
#else
#define HTTP_EXPORT
#endif

namespace rage
{
	class fiDevice;
}

class HttpClientImpl;

class HTTP_EXPORT HttpClient
{
public:
	HttpClient(const wchar_t* userAgent = L"CitizenFX/1");
	virtual ~HttpClient();

	std::string BuildPostString(const std::map<std::string, std::string>& fields);

	void DoGetRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const std::function<void(bool, const char*, size_t)>& callback);

	void DoPostRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const std::map<std::string, std::string>& fields, const std::function<void(bool, const char*, size_t)>& callback);
	void DoPostRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const std::string& postData, const std::function<void(bool, const char*, size_t)>& callback);

	void DoPostRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const std::string& postData, const fwMap<fwString, fwString>& headers, const std::function<void(bool, const char*, size_t)>& callback, std::function<void(const std::map<std::string, std::string>&)> headerCallback = std::function<void(const std::map<std::string, std::string>&)>());

	void DoFileGetRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const char* outDeviceBase, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback);
	void DoFileGetRequest(const std::wstring& host, uint16_t port, const std::wstring& url, fwRefContainer<vfs::Device> outDevice, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback);

	void DoFileGetRequest(const std::string& url, const char* outDeviceBase, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback);
	void DoFileGetRequest(const std::string& url, fwRefContainer<vfs::Device> outDevice, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback);

	// compatibility wrapper
	void DoFileGetRequest(const std::wstring& host, uint16_t port, const std::wstring& url, rage::fiDevice* outDevice, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback);

private:
	HttpClientImpl* m_impl;
};

DECLARE_INSTANCE_TYPE(HttpClient);