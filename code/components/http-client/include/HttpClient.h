/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <chrono>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

#include <VFSDevice.h>

#ifndef HTTP_EXPORT
#ifdef COMPILING_HTTP_CLIENT
#define HTTP_EXPORT DLL_EXPORT
#else
#define HTTP_EXPORT DLL_IMPORT
#endif
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

struct HttpIgnoreCaseLess
{
	inline bool operator()(const std::string& left, const std::string& right) const
	{
		return _stricmp(left.c_str(), right.c_str()) < 0;
	}
};

using HttpHeaderList = std::multimap<std::string, std::string, HttpIgnoreCaseLess>;
using HttpHeaderListPtr = std::shared_ptr<HttpHeaderList>;

struct HttpRequestOptions
{
	std::map<std::string, std::string> headers;
	HttpHeaderListPtr responseHeaders;
	std::shared_ptr<int> responseCode;
	std::function<void(const ProgressInfo&)> progressCallback;
	std::function<bool(const std::string&)> streamingCallback;
	std::chrono::milliseconds timeoutNoResponse{ 0 };
	int weight = 16;
	uint64_t maxFilesize = 0;

	bool ipv4 = false;
	bool addErrorBody = false;
	bool followLocation = true;
	bool addRawBody = false;
};

struct HttpRequestHandle
{
public:
	virtual ~HttpRequestHandle() = default;

	virtual bool HasCompleted() = 0;

	virtual void SetRequestWeight(int weight) = 0;

	virtual void Abort() = 0;

	virtual std::string GetRawBody() = 0;

	inline void Wait()
	{
		while (!HasCompleted())
		{
			std::this_thread::yield();
		}
	}
};

struct ManualHttpRequestHandle : HttpRequestHandle
{
public:
	virtual void Start() = 0;

	virtual void OnCompletion(std::function<void(bool, std::string_view)>&& callback) = 0;
};

using HttpRequestPtr = std::shared_ptr<HttpRequestHandle>;
using ManualHttpRequestPtr = std::shared_ptr<ManualHttpRequestHandle>;

class HTTP_EXPORT HttpClient final
{
public:
	HttpClient(const wchar_t* userAgent = L"CitizenFX/1", const std::string& loopId = {});
	virtual ~HttpClient();

	std::string BuildPostString(const std::map<std::string, std::string>& fields);

	HttpRequestPtr DoGetRequest(const std::string& url, const std::function<void(bool, const char*, size_t)>& callback);

	HttpRequestPtr DoGetRequest(const std::string& url, const HttpRequestOptions& options, const std::function<void(bool, const char*, size_t)>& callback);

	HttpRequestPtr DoGetRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const std::function<void(bool, const char*, size_t)>& callback);

	HttpRequestPtr DoPostRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const std::map<std::string, std::string>& fields, const std::function<void(bool, const char*, size_t)>& callback);
	HttpRequestPtr DoPostRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const std::string& postData, const std::function<void(bool, const char*, size_t)>& callback);

	HttpRequestPtr DoPostRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const std::string& postData, const fwMap<fwString, fwString>& headers, const std::function<void(bool, const char*, size_t)>& callback, std::function<void(const std::map<std::string, std::string>&)> headerCallback = std::function<void(const std::map<std::string, std::string>&)>());

	HttpRequestPtr DoPostRequest(const std::string& url, const std::map<std::string, std::string>& fields, const std::function<void(bool, const char*, size_t)>& callback);
	HttpRequestPtr DoPostRequest(const std::string& url, const std::string& postData, const std::function<void(bool, const char*, size_t)>& callback);
	HttpRequestPtr DoPostRequest(const std::string& url, const std::string& postData, const fwMap<fwString, fwString>& headers, const std::function<void(bool, const char*, size_t)>& callback, std::function<void(const std::map<std::string, std::string>&)> headerCallback = std::function<void(const std::map<std::string, std::string>&)>());

	HttpRequestPtr DoPostRequest(const std::string& url, const std::string& postData, const HttpRequestOptions& options, const std::function<void(bool, const char*, size_t)>& callback, std::function<void(const std::map<std::string, std::string>&)> headerCallback = std::function<void(const std::map<std::string, std::string>&)>());

	HttpRequestPtr DoMethodRequest(const std::string& method, const std::string& url, const std::string& postData, const HttpRequestOptions& options, const std::function<void(bool, const char*, size_t)>& callback, std::function<void(const std::map<std::string, std::string>&)> headerCallback = std::function<void(const std::map<std::string, std::string>&)>());

	HttpRequestPtr DoFileGetRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const char* outDeviceBase, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback);
	HttpRequestPtr DoFileGetRequest(const std::wstring& host, uint16_t port, const std::wstring& url, fwRefContainer<vfs::Device> outDevice, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback);

	HttpRequestPtr DoFileGetRequest(const std::string& url, const char* outDeviceBase, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback);
	HttpRequestPtr DoFileGetRequest(const std::string& url, fwRefContainer<vfs::Device> outDevice, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback);

	// native call
	HttpRequestPtr DoFileGetRequest(const std::string& url, fwRefContainer<vfs::Device> outDevice, const std::string& outFilename, const HttpRequestOptions& options, const std::function<void(bool, const char*, size_t)>& callback);

	// compatibility wrapper
	HttpRequestPtr DoFileGetRequest(const std::wstring& host, uint16_t port, const std::wstring& url, rage::fiDevice* outDevice, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback);

	// 'newer' API
	ManualHttpRequestPtr Get(const std::string& url, const HttpRequestOptions& options = {});

public:
	fwEvent<void*, const std::string&> OnSetupCurlHandle;

private:
	std::unique_ptr<HttpClientImpl> m_impl;
};

DECLARE_INSTANCE_TYPE(HttpClient);
