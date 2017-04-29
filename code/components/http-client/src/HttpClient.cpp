/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "HttpClient.h"

#include <chrono>

#include <Error.h>

#include <sstream>

#define CURL_STATICLIB
#include <curl/multi.h>

#include <VFSManager.h>

class HttpClientImpl
{
public:
	CURLM* multi;
	bool shouldRun;
	std::thread thread;
	std::mutex mutex;

	HttpClientImpl()
		: multi(nullptr), shouldRun(true)
	{

	}

	void AddCurlHandle(CURL* easy);
};

void HttpClientImpl::AddCurlHandle(CURL* easy)
{
	std::unique_lock<std::mutex> lock(mutex);
	curl_multi_add_handle(multi, easy);
}

class CurlData
{
public:
	std::string url;
	std::string postData;
	std::function<void(bool, const char*, size_t)> callback;
	std::function<size_t(const void*, size_t)> writeFunction;
	std::function<void()> preCallback;
	std::stringstream ss;
	char errBuffer[CURL_ERROR_SIZE];

	CurlData();

	void HandleResult(CURL* handle, CURLcode result);

	size_t HandleWrite(const void* data, size_t size, size_t nmemb);
};

CurlData::CurlData()
{
	writeFunction = [=] (const void* data, size_t size)
	{
		ss << std::string((const char*)data, size);
		return size;
	};
}

size_t CurlData::HandleWrite(const void* data, size_t size, size_t nmemb)
{
	return writeFunction(data, size * nmemb);
}

void CurlData::HandleResult(CURL* handle, CURLcode result)
{
	if (preCallback)
	{
		preCallback();
	}

	if (result != CURLE_OK)
	{
		auto failure = fmt::sprintf("%s - CURL error code %d (%s)", errBuffer, (int)result, curl_easy_strerror(result));

		callback(false, failure.c_str(), failure.size());
	}
	else
	{
		long code;
		curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &code);

		if (code >= 400)
		{
			auto failure = fmt::sprintf("HTTP %d", code);

			callback(false, failure.c_str(), failure.size());
		}
		else
		{
			auto str = ss.str();

			callback(true, str.c_str(), str.size());
		}
	}
}

HttpClient::HttpClient(const wchar_t* userAgent /* = L"CitizenFX/1" */)
	: m_impl(new HttpClientImpl())
{
	m_impl->multi = curl_multi_init();
	curl_multi_setopt(m_impl->multi, CURLMOPT_PIPELINING, CURLPIPE_HTTP1 | CURLPIPE_MULTIPLEX);
	curl_multi_setopt(m_impl->multi, CURLMOPT_MAX_HOST_CONNECTIONS, 8);
	
	m_impl->thread = std::move(std::thread([=]()
	{
		using namespace std::literals;

		int lastRunning = 0;

		do 
		{
			CURLMcode mc;
			int numfds;
			int nowRunning;

			// perform requests
			{
				std::unique_lock<std::mutex> lock(m_impl->mutex);
				mc = curl_multi_perform(m_impl->multi, &nowRunning);
			}

			if (mc == CURLM_OK)
			{
				// read infos
				CURLMsg* msg;

				do 
				{
					int nq;
					{
						std::unique_lock<std::mutex> lock(m_impl->mutex);
						msg = curl_multi_info_read(m_impl->multi, &nq);
					}

					// is this a completed transfer?
					if (msg && msg->msg == CURLMSG_DONE)
					{
						// get the handle and the result
						CURL* curl = msg->easy_handle;
						CURLcode result = msg->data.result;

						char* dataPtr;
						curl_easy_getinfo(curl, CURLINFO_PRIVATE, &dataPtr);

						CurlData* data = reinterpret_cast<CurlData*>(dataPtr);
						data->HandleResult(curl, result);

						delete data;

						{
							std::unique_lock<std::mutex> lock(m_impl->mutex);
							curl_multi_remove_handle(m_impl->multi, curl);
						}

						curl_easy_cleanup(curl);
					}
				} while (msg);

				// save the last value
				lastRunning = nowRunning;
			}

			{
				std::unique_lock<std::mutex> lock(m_impl->mutex);
				mc = curl_multi_wait(m_impl->multi, nullptr, 0, 50, &numfds);
			}

			if (mc != CURLM_OK)
			{
				FatalError("curl_multi_wait failed with error %s", curl_multi_strerror(mc));
				return;
			}

			if (numfds == 0)
			{
				std::this_thread::sleep_for(100ms);
			}
		} while (m_impl->shouldRun);
	}));
}

HttpClient::~HttpClient()
{
	m_impl->shouldRun = false;

	if (m_impl->thread.joinable())
	{
		m_impl->thread.join();
	}

	delete m_impl;
}

std::string HttpClient::BuildPostString(const std::map<std::string, std::string>& fields)
{
	std::stringstream retval;

	for (auto& field : fields)
	{
		retval << field.first << "=" << url_encode(field.second) << "&";
	}

	fwString str = fwString(retval.str().c_str());
	return str.substr(0, str.length() - 1);
}

static std::string MakeURL(const std::wstring& host, uint16_t port, const std::wstring& url)
{
	return fmt::sprintf("http://%s:%d%s", ToNarrow(host), port, ToNarrow(url));
}

static auto CurlWrite(char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t
{
	CurlData* cd = reinterpret_cast<CurlData*>(userdata);

	return cd->HandleWrite(ptr, size, nmemb);
}

static std::tuple<CURL*, CurlData*> SetupCURLHandle(const std::string& url, const std::function<void(bool, const char*, size_t)>& callback)
{
	auto curlHandle = curl_easy_init();

	auto curlData = new CurlData();
	curlData->url = url;
	curlData->callback = callback;

	curl_easy_setopt(curlHandle, CURLOPT_URL, curlData->url.c_str());
	curl_easy_setopt(curlHandle, CURLOPT_PRIVATE, curlData);
	curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, curlData);
	curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, CurlWrite);
	curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, true);
	curl_easy_setopt(curlHandle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
	curl_easy_setopt(curlHandle, CURLOPT_ERRORBUFFER, &curlData->errBuffer);

	return { curlHandle, curlData };
}

void HttpClient::DoGetRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const std::function<void(bool, const char*, size_t)>& callback)
{
	auto urlStr = MakeURL(host, port, url);
	
	CURL* curlHandle;
	CurlData* curlData;
	std::tie(curlHandle, curlData) = SetupCURLHandle(urlStr, callback);

	m_impl->AddCurlHandle(curlHandle);
}

void HttpClient::DoPostRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const std::map<std::string, std::string>& fields, const std::function<void(bool, const char*, size_t)>& callback)
{
	return DoPostRequest(host, port, url, BuildPostString(fields), callback);
}

void HttpClient::DoPostRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const std::string& postData, const std::function<void(bool, const char*, size_t)>& callback)
{
	return DoPostRequest(host, port, url, postData, {}, callback);
}

void HttpClient::DoPostRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const std::string& postData, const fwMap<fwString, fwString>& headersMap, const std::function<void(bool, const char*, size_t)>& callback, std::function<void(const std::map<std::string, std::string>&)> headerCallback /*= std::function<void(const std::map<std::string, std::string>&)>()*/)
{
	auto urlStr = MakeURL(host, port, url);

	// make handle
	CURL* curlHandle;
	CurlData* curlData;
	std::tie(curlHandle, curlData) = SetupCURLHandle(urlStr, callback);

	// assign post data
	curlData->postData = postData;

	curl_easy_setopt(curlHandle, CURLOPT_POSTFIELDS, curlData->postData.c_str());

	curl_slist* headers = nullptr;
	for (const auto& header : headersMap)
	{
		headers = curl_slist_append(headers, va("%s: %s", header.first, header.second));
	}

	curl_easy_setopt(curlHandle, CURLOPT_HTTPHEADER, headers);

	// write out
	m_impl->AddCurlHandle(curlHandle);
}

void HttpClient::DoFileGetRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const char* outDeviceBase, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback)
{
	return DoFileGetRequest(host, port, url, vfs::GetDevice(outDeviceBase), outFilename, callback);
}

void HttpClient::DoFileGetRequest(const std::string& url, const char* outDeviceBase, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback)
{
	return DoFileGetRequest(url, vfs::GetDevice(outDeviceBase), outFilename, callback);
}

void HttpClient::DoFileGetRequest(const std::wstring& host, uint16_t port, const std::wstring& url, fwRefContainer<vfs::Device> outDevice, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback)
{
	return DoFileGetRequest(MakeURL(host, port, url), outDevice, outFilename, callback);
}

void HttpClient::DoFileGetRequest(const std::string& urlStr, fwRefContainer<vfs::Device> outDevice, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback)
{
	CURL* curlHandle;
	CurlData* curlData;
	std::tie(curlHandle, curlData) = SetupCURLHandle(urlStr, callback);

	auto handle = outDevice->Create(outFilename);

	curlData->writeFunction = [=] (const void* data, size_t length)
	{
		outDevice->Write(handle, data, length);

		return length;
	};

	curlData->preCallback = [=] ()
	{
		outDevice->Close(handle);
	};

	m_impl->AddCurlHandle(curlHandle);
}

void HttpClient::DoFileGetRequest(const std::wstring& host, uint16_t port, const std::wstring& url, rage::fiDevice* outDevice, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback)
{
	return DoFileGetRequest(host, port, url, vfs::GetNativeDevice(outDevice), outFilename, callback);
}

static InitFunction initFunction([]()
{
	Instance<HttpClient>::Set(new HttpClient());
});