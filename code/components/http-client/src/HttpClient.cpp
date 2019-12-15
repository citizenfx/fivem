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

#include <tbb/concurrent_queue.h>

class HttpClientImpl
{
public:
	CURLM* multi;
	bool shouldRun;
	std::thread thread;
	tbb::concurrent_queue<CURL*> handlesToAdd;
	tbb::concurrent_queue<std::function<void()>> cbsToRun;
	HttpClient* client;

	HttpClientImpl()
		: multi(nullptr), shouldRun(true)
	{

	}

	void AddCurlHandle(CURL* easy);
};

void HttpClientImpl::AddCurlHandle(CURL* easy)
{
	handlesToAdd.push(easy);
}

class CurlData
{
public:
	std::string url;
	std::string postData;
	std::function<void(bool, const char*, size_t)> callback;
	std::function<size_t(const void*, size_t)> writeFunction;
	std::function<void()> preCallback;
	std::function<void(const ProgressInfo&)> progressCallback;
	std::stringstream ss;
	char errBuffer[CURL_ERROR_SIZE];
	CURL* curlHandle;
	HttpClientImpl* impl;
	int defaultWeight;
	int weight;
	HttpHeaderListPtr responseHeaders;
	std::shared_ptr<int> responseCode;
	std::chrono::milliseconds timeoutNoResponse;
	std::chrono::high_resolution_clock::duration reqStart;

	CurlData();

	void HandleResult(CURL* handle, CURLcode result);

	size_t HandleWrite(const void* data, size_t size, size_t nmemb);
};

CurlData::CurlData()
{
	timeoutNoResponse = std::chrono::milliseconds(0);
	reqStart = std::chrono::high_resolution_clock::duration(0);

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

		if (this->responseCode)
		{
			*this->responseCode = (int)code;
		}

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
	m_impl->client = this;
	m_impl->multi = curl_multi_init();
	curl_multi_setopt(m_impl->multi, CURLMOPT_PIPELINING, CURLPIPE_HTTP1 | CURLPIPE_MULTIPLEX);
	curl_multi_setopt(m_impl->multi, CURLMOPT_MAX_HOST_CONNECTIONS, 8);
	
	m_impl->thread = std::move(std::thread([=]()
	{
		using namespace std::literals;

		SetThreadName(-1, "[Cfx] HttpClient Thread");

		int lastRunning = 0;

		do 
		{
			// add new handles
			{
				CURL* addHandle;

				while (m_impl->handlesToAdd.try_pop(addHandle))
				{
					curl_multi_add_handle(m_impl->multi, addHandle);
				}
			}

			// run callback queue
			{
				std::function<void()> runCb;

				while (m_impl->cbsToRun.try_pop(runCb))
				{
					runCb();
				}
			}

			// run iteration
			CURLMcode mc;
			int numfds;
			int nowRunning;

			// perform requests
			mc = curl_multi_perform(m_impl->multi, &nowRunning);

			if (mc == CURLM_OK)
			{
				// read infos
				CURLMsg* msg;

				do 
				{
					int nq;
					msg = curl_multi_info_read(m_impl->multi, &nq);

					// is this a completed transfer?
					if (msg && msg->msg == CURLMSG_DONE)
					{
						// get the handle and the result
						CURL* curl = msg->easy_handle;
						CURLcode result = msg->data.result;

						char* dataPtr;
						curl_easy_getinfo(curl, CURLINFO_PRIVATE, &dataPtr);

						auto data = reinterpret_cast<std::shared_ptr<CurlData>*>(dataPtr);
						(*data)->HandleResult(curl, result);

						curl_multi_remove_handle(m_impl->multi, curl);
						curl_easy_cleanup(curl);

						// delete data
						(*data)->curlHandle = nullptr;
						delete data;
					}
				} while (msg);

				// save the last value
				lastRunning = nowRunning;
			}

			mc = curl_multi_wait(m_impl->multi, nullptr, 0, 20, &numfds);

			if (mc != CURLM_OK)
			{
				FatalError("curl_multi_wait failed with error %s", curl_multi_strerror(mc));
				return;
			}

			if (numfds == 0)
			{
				std::this_thread::sleep_for(20ms);
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
	auto cd = reinterpret_cast<std::shared_ptr<CurlData>*>(userdata);

	return (*cd)->HandleWrite(ptr, size, nmemb);
}

static int CurlXferInfo(void *userdata, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
	auto cdPtr = reinterpret_cast<std::shared_ptr<CurlData>*>(userdata);
	CurlData* cd = cdPtr->get();

	if (cd->progressCallback)
	{
		ProgressInfo info;
		info.downloadNow = dlnow;
		info.downloadTotal = dltotal;

		cd->progressCallback(info);
	}

	using namespace std::chrono_literals;

	if (cd->timeoutNoResponse != 0ms)
	{
		// first progress callback is the start of the timeout
		// if we do this any earlier, we run the risk of having concurrent connections that
		// are throttled due to max-connection limit time out instantly
		if (cd->reqStart.count() == 0)
		{
			cd->reqStart = std::chrono::high_resolution_clock::now().time_since_epoch();
		}

		if (dlnow == 0 && dltotal == 0)
		{
			if (std::chrono::high_resolution_clock::now().time_since_epoch() - cd->reqStart > cd->timeoutNoResponse)
			{
				// abort due to timeout
				return 1;
			}
		}
	}

	return 0;
}

static size_t CurlHeaderInfo(char* buffer, size_t size, size_t nitems, void* userdata)
{
	auto cdPtr = reinterpret_cast<std::shared_ptr<CurlData>*>(userdata);
	CurlData* cd = cdPtr->get();

	if (cd->responseHeaders)
	{
		std::string str(buffer, size * nitems);

		// reset HTTP headers if we followed a Location and got a new HTTP response
		if (str.find("HTTP/") == 0)
		{
			(*cd->responseHeaders).clear();
		}

		auto colonPos = str.find(": ");
		
		if (colonPos != std::string::npos)
		{
			(*cd->responseHeaders)[str.substr(0, colonPos)] = str.substr(colonPos + 2, str.length() - 2 - colonPos - 2);
		}
	}

	return size * nitems;
}

static std::tuple<CURL*, std::shared_ptr<CurlData>> SetupCURLHandle(HttpClientImpl* impl, const std::string& url, const HttpRequestOptions& options, const std::function<void(bool, const char*, size_t)>& callback)
{
	auto curlHandle = curl_easy_init();

	auto curlData = std::make_shared<CurlData>();
	curlData->url = url;
	curlData->callback = callback;
	curlData->progressCallback = options.progressCallback;
	curlData->curlHandle = curlHandle;
	curlData->impl = impl;
	curlData->defaultWeight = curlData->weight = options.weight;
	curlData->responseHeaders = options.responseHeaders;
	curlData->responseCode = options.responseCode;
	curlData->timeoutNoResponse = options.timeoutNoResponse;

	auto scb = options.streamingCallback;

	if (scb)
	{
		curlData->writeFunction = [scb](const void* data, size_t len)
		{
			bool success = scb(std::string(reinterpret_cast<const char*>(data), len));

			return (success) ? len : 0;
		};
	}

	auto curlDataPtr = new std::shared_ptr<CurlData>(curlData);

	curl_easy_setopt(curlHandle, CURLOPT_URL, curlData->url.c_str());
	curl_easy_setopt(curlHandle, CURLOPT_PRIVATE, curlDataPtr);
	curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, curlDataPtr);
	curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, CurlWrite);
	curl_easy_setopt(curlHandle, CURLOPT_XFERINFODATA, curlDataPtr);
	curl_easy_setopt(curlHandle, CURLOPT_XFERINFOFUNCTION, CurlXferInfo);
	curl_easy_setopt(curlHandle, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, true);
	curl_easy_setopt(curlHandle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
	curl_easy_setopt(curlHandle, CURLOPT_ERRORBUFFER, &curlData->errBuffer);
	curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYPEER, 0);
	curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYHOST, 0);
	curl_easy_setopt(curlHandle, CURLOPT_STREAM_WEIGHT, long(options.weight));
	
	if (options.responseHeaders)
	{
		curl_easy_setopt(curlHandle, CURLOPT_HEADERFUNCTION, CurlHeaderInfo);
		curl_easy_setopt(curlHandle, CURLOPT_HEADERDATA, curlDataPtr);
	}

	curl_slist* headers = nullptr;
	for (const auto& header : options.headers)
	{
		headers = curl_slist_append(headers, va("%s: %s", header.first, header.second));
	}

	curl_easy_setopt(curlHandle, CURLOPT_HTTPHEADER, headers);

	if (options.ipv4)
	{
		curl_easy_setopt(curlHandle, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
	}

	impl->client->OnSetupCurlHandle(curlHandle, url);

	return { curlHandle, curlData };
}

class HttpRequestHandleImpl : public HttpRequestHandle
{
private:
	std::shared_ptr<CurlData> m_request;

public:
	HttpRequestHandleImpl(const std::shared_ptr<CurlData>& reqData)
		: m_request(reqData)
	{

	}

	virtual bool HasCompleted() override
	{
		return m_request->curlHandle == nullptr;
	}

	virtual void SetRequestWeight(int weight) override
	{
		int newWeight = weight;

		if (newWeight == -1)
		{
			newWeight = m_request->defaultWeight;
		}

		if (m_request->weight == newWeight)
		{
			return;
		}

		auto request = m_request;
		request->weight = newWeight;

		request->impl->cbsToRun.push([request, newWeight]()
		{
			if (request->curlHandle)
			{
				curl_easy_setopt(request->curlHandle, CURLOPT_STREAM_WEIGHT, long(newWeight));
			}
		});
	}

	virtual void Abort() override
	{
		auto request = m_request;

		request->impl->cbsToRun.push([request]()
		{
			auto curl = request->curlHandle;
			auto impl = request->impl;

			//Request has completed and curl has been cleaned up.
			if (curl == nullptr)
				return;

			// delete the data pointer
			char* dataPtr;
			curl_easy_getinfo(curl, CURLINFO_PRIVATE, &dataPtr);

			auto data = reinterpret_cast<std::shared_ptr<CurlData>*>(dataPtr);

			// remove and delete the handle
			curl_multi_remove_handle(impl->multi, curl);
			curl_easy_cleanup(curl);

			// delete cURL data
			(*data)->curlHandle = nullptr;

			delete data;
		});
	}
};

static HttpRequestPtr SetupRequestHandle(const std::shared_ptr<CurlData>& data)
{
	return std::make_shared<HttpRequestHandleImpl>(data);
}

HttpRequestPtr HttpClient::DoGetRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const std::function<void(bool, const char*, size_t)>& callback)
{
	auto urlStr = MakeURL(host, port, url);

	return DoGetRequest(urlStr, callback);
}

HttpRequestPtr HttpClient::DoGetRequest(const std::string& url, const std::function<void(bool, const char*, size_t)>& callback)
{
	return DoGetRequest(url, {}, callback);
}

HttpRequestPtr HttpClient::DoGetRequest(const std::string& url, const HttpRequestOptions& options, const std::function<void(bool, const char *, size_t)>& callback)
{
	auto[curlHandle, curlData] = SetupCURLHandle(m_impl, url, options, callback);

	m_impl->AddCurlHandle(curlHandle);

	return SetupRequestHandle(curlData);
}

HttpRequestPtr HttpClient::DoPostRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const std::map<std::string, std::string>& fields, const std::function<void(bool, const char*, size_t)>& callback)
{
	return DoPostRequest(host, port, url, BuildPostString(fields), callback);
}

HttpRequestPtr HttpClient::DoPostRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const std::string& postData, const std::function<void(bool, const char*, size_t)>& callback)
{
	return DoPostRequest(host, port, url, postData, {}, callback);
}

HttpRequestPtr HttpClient::DoPostRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const std::string& postData, const fwMap<fwString, fwString>& headersMap, const std::function<void(bool, const char*, size_t)>& callback, std::function<void(const std::map<std::string, std::string>&)> headerCallback /*= std::function<void(const std::map<std::string, std::string>&)>()*/)
{
	auto urlStr = MakeURL(host, port, url);

	return DoPostRequest(urlStr, postData, headersMap, callback, headerCallback);
}

HttpRequestPtr HttpClient::DoPostRequest(const std::string& url, const std::map<std::string, std::string>& fields, const std::function<void(bool, const char*, size_t)>& callback)
{
	return DoPostRequest(url, BuildPostString(fields), callback);
}

HttpRequestPtr HttpClient::DoPostRequest(const std::string& url, const std::string& postData, const std::function<void(bool, const char*, size_t)>& callback)
{
	return DoPostRequest(url, postData, HttpRequestOptions{}, callback);
}

HttpRequestPtr HttpClient::DoPostRequest(const std::string& url, const std::string& postData, const fwMap<fwString, fwString>& headersMap, const std::function<void(bool, const char*, size_t)>& callback, std::function<void(const std::map<std::string, std::string>&)> headerCallback /*= std::function<void(const std::map<std::string, std::string>&)>()*/)
{
	HttpRequestOptions options;
	options.headers = headersMap;

	return DoPostRequest(url, postData, options, callback, headerCallback);
}

HttpRequestPtr HttpClient::DoPostRequest(const std::string& url, const std::string& postData, const HttpRequestOptions& options, const std::function<void(bool, const char*, size_t)>& callback, std::function<void(const std::map<std::string, std::string>&)> headerCallback /*= std::function<void(const std::map<std::string, std::string>&)>()*/)
{
	// make handle
	auto [curlHandle, curlData] = SetupCURLHandle(m_impl, url, options, callback);

	// assign post data
	curlData->postData = postData;

	curl_easy_setopt(curlHandle, CURLOPT_POSTFIELDS, curlData->postData.c_str());

	// write out
	m_impl->AddCurlHandle(curlHandle);

	return SetupRequestHandle(curlData);
}

HttpRequestPtr HttpClient::DoMethodRequest(const std::string& method, const std::string& url, const std::string& postData, const HttpRequestOptions& options, const std::function<void(bool, const char*, size_t)>& callback, std::function<void(const std::map<std::string, std::string>&)> headerCallback /*= std::function<void(const std::map<std::string, std::string>&)>()*/)
{
	// make handle
	auto[curlHandle, curlData] = SetupCURLHandle(m_impl, url, options, callback);

	if (!postData.empty())
	{
		// assign post data
		curlData->postData = postData;

		curl_easy_setopt(curlHandle, CURLOPT_POSTFIELDS, curlData->postData.c_str());
	}

	curl_easy_setopt(curlHandle, CURLOPT_CUSTOMREQUEST, method.c_str());

	// write out
	m_impl->AddCurlHandle(curlHandle);

	return SetupRequestHandle(curlData);
}

HttpRequestPtr HttpClient::DoFileGetRequest(const std::wstring& host, uint16_t port, const std::wstring& url, const char* outDeviceBase, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback)
{
	return DoFileGetRequest(host, port, url, vfs::GetDevice(outDeviceBase), outFilename, callback);
}

HttpRequestPtr HttpClient::DoFileGetRequest(const std::string& url, const char* outDeviceBase, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback)
{
	return DoFileGetRequest(url, vfs::GetDevice(outDeviceBase), outFilename, callback);
}

HttpRequestPtr HttpClient::DoFileGetRequest(const std::wstring& host, uint16_t port, const std::wstring& url, fwRefContainer<vfs::Device> outDevice, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback)
{
	return DoFileGetRequest(MakeURL(host, port, url), outDevice, outFilename, callback);
}

HttpRequestPtr HttpClient::DoFileGetRequest(const std::string& urlStr, fwRefContainer<vfs::Device> outDevice, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback)
{
	return DoFileGetRequest(urlStr, outDevice, outFilename, {}, callback);
}

HttpRequestPtr HttpClient::DoFileGetRequest(const std::string& urlStr, fwRefContainer<vfs::Device> outDevice, const std::string& outFilename, const HttpRequestOptions& options, const std::function<void(bool, const char*, size_t)>& callback)
{
	auto [curlHandle, curlData] = SetupCURLHandle(m_impl, urlStr, options, callback);

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

	return SetupRequestHandle(curlData);
}

HttpRequestPtr HttpClient::DoFileGetRequest(const std::wstring& host, uint16_t port, const std::wstring& url, rage::fiDevice* outDevice, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback)
{
	return DoFileGetRequest(host, port, url, vfs::GetNativeDevice(outDevice), outFilename, callback);
}

static InitFunction initFunction([]()
{
	Instance<HttpClient>::Set(new HttpClient());
});
