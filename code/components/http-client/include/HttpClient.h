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
#define HTTP_EXPORT DLL_EXPORT
#else
#define HTTP_EXPORT
#endif

namespace rage
{
	class fiDevice;
}

class HttpClientImpl;

struct ProgressInfo
{
	size_t downloadTotal;
	size_t downloadNow;
};

struct HttpRequestOptions
{
	std::map<std::string, std::string> headers;
	std::function<void(const ProgressInfo&)> progressCallback;
	std::function<bool(const std::string&)> streamingCallback;
	int weight;

	inline HttpRequestOptions()
	{
		weight = 16;
	}
};

struct HttpRequestHandle
{
public:
	virtual ~HttpRequestHandle() = default;

	virtual bool HasCompleted() = 0;

	virtual void SetRequestWeight(int weight) = 0;
};

using HttpRequestPtr = std::shared_ptr<HttpRequestHandle>;

class HTTP_EXPORT HttpClient
{
public:
	HttpClient(const wchar_t* userAgent = L"CitizenFX/1");
	virtual ~HttpClient();

	std::string BuildPostString(const std::map<std::string, std::string>& fields);

	HttpRequestPtr DoGetRequest(const std::string& url, const std::function<void(bool, const char*, size_t)>& callback);

	HttpRequestPtr DoGetRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const std::function<void(bool, const char*, size_t)>& callback);

	HttpRequestPtr DoPostRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const std::map<std::string, std::string>& fields, const std::function<void(bool, const char*, size_t)>& callback);
	HttpRequestPtr DoPostRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const std::string& postData, const std::function<void(bool, const char*, size_t)>& callback);

	HttpRequestPtr DoPostRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const std::string& postData, const fwMap<fwString, fwString>& headers, const std::function<void(bool, const char*, size_t)>& callback, std::function<void(const std::map<std::string, std::string>&)> headerCallback = std::function<void(const std::map<std::string, std::string>&)>());

	HttpRequestPtr DoPostRequest(const std::string& url, const std::map<std::string, std::string>& fields, const std::function<void(bool, const char*, size_t)>& callback);
	HttpRequestPtr DoPostRequest(const std::string& url, const std::string& postData, const std::function<void(bool, const char*, size_t)>& callback);
	HttpRequestPtr DoPostRequest(const std::string& url, const std::string& postData, const fwMap<fwString, fwString>& headers, const std::function<void(bool, const char*, size_t)>& callback, std::function<void(const std::map<std::string, std::string>&)> headerCallback = std::function<void(const std::map<std::string, std::string>&)>());

	HttpRequestPtr DoPostRequest(const std::string& url, const std::string& postData, const HttpRequestOptions& options, const std::function<void(bool, const char*, size_t)>& callback, std::function<void(const std::map<std::string, std::string>&)> headerCallback = std::function<void(const std::map<std::string, std::string>&)>());

	HttpRequestPtr DoFileGetRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const char* outDeviceBase, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback);
	HttpRequestPtr DoFileGetRequest(const std::wstring& host, uint16_t port, const std::wstring& url, fwRefContainer<vfs::Device> outDevice, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback);

	HttpRequestPtr DoFileGetRequest(const std::string& url, const char* outDeviceBase, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback);
	HttpRequestPtr DoFileGetRequest(const std::string& url, fwRefContainer<vfs::Device> outDevice, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback);

	// native call
	HttpRequestPtr DoFileGetRequest(const std::string& url, fwRefContainer<vfs::Device> outDevice, const std::string& outFilename, const HttpRequestOptions& options, const std::function<void(bool, const char*, size_t)>& callback);

	// compatibility wrapper
	HttpRequestPtr DoFileGetRequest(const std::wstring& host, uint16_t port, const std::wstring& url, rage::fiDevice* outDevice, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback);

private:
	HttpClientImpl* m_impl;
};

DECLARE_INSTANCE_TYPE(HttpClient);
