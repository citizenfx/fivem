/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "HttpClient.h"

#include <chrono>

#include <boost/algorithm/string.hpp>
#include <Error.h>

#include <sstream>

#define CURL_STATICLIB
#include <curl/multi.h>

#include <VFSManager.h>

#include <tbb/concurrent_queue.h>

#include <shared_mutex>

#include <UvLoopManager.h>

class HttpClientImpl
{
public:
	CURLM* multi;
	tbb::concurrent_queue<CURL*> handlesToAdd;
	tbb::concurrent_queue<std::function<void()>> cbsToRun;
	HttpClient* client;
	uv_loop_t* loop;
	uv_timer_t timeout;
	std::string userAgent;

	std::shared_ptr<uvw::AsyncHandle> addHandle;
	std::shared_ptr<uvw::AsyncHandle> runCb;

	std::shared_mutex mutex;

	HttpClientImpl()
		: multi(nullptr), client(nullptr), loop(nullptr)
	{

	}

	void AddCurlHandle(CURL* easy);
};

void HttpClientImpl::AddCurlHandle(CURL* easy)
{
	handlesToAdd.push(easy);

	std::shared_lock<std::shared_mutex> _(mutex);

	if (addHandle)
	{
		addHandle->send();
	}
}

class CurlData final
{
public:
	std::string url;
	std::string postData;
	std::function<void(bool, std::string_view)> callback;
	std::function<size_t(const void*, size_t)> writeFunction;
	std::function<void()> preCallback;
	std::function<void(const ProgressInfo&)> progressCallback;
	std::stringstream ss;
	char errBuffer[CURL_ERROR_SIZE];
	CURL* curlHandle;
	HttpClientImpl* impl;
	curl_slist* headersSList = nullptr;
	int defaultWeight;
	int weight;
	HttpHeaderListPtr responseHeaders;
	std::shared_ptr<int> responseCode;
	std::chrono::milliseconds timeoutNoResponse;
	std::chrono::high_resolution_clock::duration reqStart;

	std::stringstream errorBody;
	std::stringstream rawBody;

	bool addErrorBody = false;
	bool addRawBody = false;

	CurlData();

	virtual ~CurlData();

	void HandleResult(CURL* handle, CURLcode result);

	size_t HandleWrite(const void* data, size_t size, size_t nmemb);
};

CurlData::CurlData()
{
	timeoutNoResponse = std::chrono::milliseconds(0);
	reqStart = std::chrono::high_resolution_clock::duration(0);

	writeFunction = [this] (const void* data, size_t size)
	{
		ss << std::string((const char*)data, size);
		return size;
	};
}

CurlData::~CurlData()
{
	curl_slist_free_all(headersSList);
	headersSList = nullptr;
}

size_t CurlData::HandleWrite(const void* data, size_t size, size_t nmemb)
{
	if (addErrorBody)
	{
		long code;
		curl_easy_getinfo(curlHandle, CURLINFO_RESPONSE_CODE, &code);

		if (code >= 400)
		{
			errorBody << std::string_view{ (const char*)data, size * nmemb };
		}
	}

	if (addRawBody)
	{
		rawBody << std::string_view{ (const char*)data, size * nmemb };
	}

	return writeFunction(data, size * nmemb);
}

void CurlData::HandleResult(CURL* handle, CURLcode result)
{
	if (preCallback)
	{
		preCallback();
		preCallback = {};
	}

	if (!callback)
	{
		return;
	}

	if (result != CURLE_OK)
	{
		auto failure = fmt::sprintf("%s - CURL error code %d (%s)", errBuffer, (int)result, curl_easy_strerror(result));

		callback(false, failure);
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
			auto failure = fmt::sprintf("HTTP %d%s",
				code,
				addErrorBody
					? fmt::sprintf(": %s", this->errorBody.str())
					: "");

			callback(false, failure);
		}
		else
		{
			auto str = ss.str();

			callback(true, str);
		}
	}

	callback = {};
}

struct curl_context_t
{
	uv_poll_t poll_handle;
	curl_socket_t sockfd;
	HttpClientImpl* impl;
};

static curl_context_t* CreateCurlContext(HttpClientImpl* i, curl_socket_t sockfd)
{
	curl_context_t* context = new curl_context_t;
	context->sockfd = sockfd;

	uv_poll_init_socket(i->loop, &context->poll_handle, sockfd);
	context->poll_handle.data = context;
	context->impl = i;

	return context;
}

static void FinalizeCurlHandle(CURL* curl, CURLM* multi = nullptr, const CURLcode* result = nullptr)
{
	char* dataPtr;
	curl_easy_getinfo(curl, CURLINFO_PRIVATE, &dataPtr);

	auto data = reinterpret_cast<std::shared_ptr<CurlData>*>(dataPtr);

	if (result)
	{
		(*data)->HandleResult(curl, *result);
	}

	if (multi)
	{
		curl_multi_remove_handle(multi, curl);
	}

	curl_easy_cleanup(curl);

	// delete data
	(*data)->curlHandle = nullptr;
	delete data;
}

static void CheckMultiInfo(HttpClientImpl* impl)
{
	// read infos
	CURLMsg* msg;

	do
	{
		int nq;
		msg = curl_multi_info_read(impl->multi, &nq);

		// is this a completed transfer?
		if (msg && msg->msg == CURLMSG_DONE)
		{
			// get the handle and the result
			CURL* curl = msg->easy_handle;
			CURLcode result = msg->data.result;

			FinalizeCurlHandle(curl, impl->multi, &result);
		}
	} while (msg);
}

static void CurlPerform(uv_poll_t* req, int status, int events)
{
	int running_handles;
	int flags = 0;
	curl_context_t* context;

	if (events & UV_READABLE)
		flags |= CURL_CSELECT_IN;
	if (events & UV_WRITABLE)
		flags |= CURL_CSELECT_OUT;

	context = (curl_context_t*)req->data;

	curl_multi_socket_action(context->impl->multi, context->sockfd, flags,
	&running_handles);

	CheckMultiInfo(context->impl);
}

static void CurlCloseCb(uv_handle_t* handle)
{
	curl_context_t* context = (curl_context_t*)handle->data;
	delete context;
}

static void DestroyCurlContext(curl_context_t* context)
{
	uv_close((uv_handle_t*)&context->poll_handle, CurlCloseCb);
}

static int CurlHandleSocket(CURL* easy, curl_socket_t s, int action, void* userp, void* socketp)
{
	auto impl = reinterpret_cast<HttpClientImpl*>(userp);

	curl_context_t* curl_context;
	int events = 0;

	switch (action)
	{
		case CURL_POLL_IN:
		case CURL_POLL_OUT:
		case CURL_POLL_INOUT:
			curl_context = socketp ? (curl_context_t*)socketp : CreateCurlContext(impl, s);

			curl_multi_assign(impl->multi, s, (void*)curl_context);

			if (action != CURL_POLL_IN)
				events |= UV_WRITABLE;
			if (action != CURL_POLL_OUT)
				events |= UV_READABLE;

			uv_poll_start(&curl_context->poll_handle, events, CurlPerform);
			break;
		case CURL_POLL_REMOVE:
			if (socketp)
			{
				uv_poll_stop(&((curl_context_t*)socketp)->poll_handle);
				DestroyCurlContext((curl_context_t*)socketp);
				curl_multi_assign(impl->multi, s, NULL);
			}
			break;
	}

	return 0;
}

static void OnTimeout(uv_timer_t* req)
{
	auto impl = reinterpret_cast<HttpClientImpl*>(req->data);

	int running_handles;
	curl_multi_socket_action(impl->multi, CURL_SOCKET_TIMEOUT, 0,
	&running_handles);
	CheckMultiInfo(impl);
}

static int CurlStartTimeout(CURLM* multi, long timeout_ms, void* userp)
{
	auto impl = reinterpret_cast<HttpClientImpl*>(userp);

	if (timeout_ms < 0)
	{
		uv_timer_stop(&impl->timeout);
	}
	else
	{
		if (timeout_ms == 0)
			timeout_ms = 1; /* 0 means directly call socket_action, but we'll do it
                         in a bit */
		uv_timer_start(&impl->timeout, OnTimeout, timeout_ms, 0);
	}
	return 0;
}

HttpClient::HttpClient(const wchar_t* userAgent /* = L"CitizenFX/1" */, const std::string& loopId)
	: m_impl(new HttpClientImpl())
{
	m_impl->client = this;
	m_impl->userAgent = ToNarrow(userAgent);
	m_impl->multi = curl_multi_init();
	curl_multi_setopt(m_impl->multi, CURLMOPT_PIPELINING, CURLPIPE_HTTP1 | CURLPIPE_MULTIPLEX);
	curl_multi_setopt(m_impl->multi, CURLMOPT_MAX_HOST_CONNECTIONS, 8);
	curl_multi_setopt(m_impl->multi, CURLMOPT_SOCKETFUNCTION, CurlHandleSocket);
	curl_multi_setopt(m_impl->multi, CURLMOPT_SOCKETDATA, m_impl.get());
	curl_multi_setopt(m_impl->multi, CURLMOPT_TIMERFUNCTION, CurlStartTimeout);
	curl_multi_setopt(m_impl->multi, CURLMOPT_TIMERDATA, m_impl.get());
	
	auto loop = Instance<net::UvLoopManager>::Get()->GetOrCreate(loopId.empty() ? "httpClient" : loopId);
	m_impl->loop = loop->GetLoop();

	loop->EnqueueCallback([this, loop]()
	{
		auto impl = m_impl.get();
		uv_timer_init(loop->GetLoop(), &impl->timeout);
		impl->timeout.data = impl;

		auto runCbs = [impl]()
		{
			std::function<void()> runCb;

			while (impl->cbsToRun.try_pop(runCb))
			{
				runCb();
			}
		};

		auto runAdds = [impl]()
		{
			CURL* addHandle;

			while (impl->handlesToAdd.try_pop(addHandle))
			{
				curl_multi_add_handle(impl->multi, addHandle);
			}
		};

		auto addHandle = loop->Get()->resource<uvw::AsyncHandle>();
		addHandle->on<uvw::AsyncEvent>([runAdds](const uvw::AsyncEvent& ev, uvw::AsyncHandle& handle)
		{
			runAdds();
		});

		auto runCb = loop->Get()->resource<uvw::AsyncHandle>();
		runCb->on<uvw::AsyncEvent>([runCbs](const uvw::AsyncEvent& ev, uvw::AsyncHandle& handle)
		{
			runCbs();
		});

		std::lock_guard<std::shared_mutex> _(impl->mutex);

		runCbs();
		runAdds();

		impl->runCb = runCb;
		impl->addHandle = addHandle;
	});
}

HttpClient::~HttpClient()
{

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
			(*cd->responseHeaders).emplace(str.substr(0, colonPos), str.substr(colonPos + 2, str.length() - 2 - colonPos - 2));
		}
	}

	return size * nitems;
}

static std::shared_ptr<CurlData> SetupCURLHandle(const std::unique_ptr<HttpClientImpl>& impl, const std::string& url, const HttpRequestOptions& options)
{
	if (boost::algorithm::to_lower_copy(url).find("file://") == 0)
	{
		FatalError("Invalid URL in HttpClient\nHit a file:// URL in HttpClient (%s). Please report this somewhere.", url);
	}

	auto curlHandle = curl_easy_init();

	auto curlData = std::make_shared<CurlData>();
	curlData->url = url;
	curlData->progressCallback = options.progressCallback;
	curlData->curlHandle = curlHandle;
	curlData->impl = impl.get();
	curlData->defaultWeight = curlData->weight = options.weight;
	curlData->responseHeaders = options.responseHeaders;
	curlData->responseCode = options.responseCode;
	curlData->timeoutNoResponse = options.timeoutNoResponse;
	curlData->addErrorBody = options.addErrorBody;
	curlData->addRawBody = options.addRawBody;

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

	curl_easy_setopt(curlHandle, CURLOPT_USERAGENT, impl->userAgent.c_str());
	curl_easy_setopt(curlHandle, CURLOPT_URL, curlData->url.c_str());
	curl_easy_setopt(curlHandle, CURLOPT_PRIVATE, curlDataPtr);
	curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, curlDataPtr);
	curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, CurlWrite);
	curl_easy_setopt(curlHandle, CURLOPT_XFERINFODATA, curlDataPtr);
	curl_easy_setopt(curlHandle, CURLOPT_XFERINFOFUNCTION, CurlXferInfo);
	curl_easy_setopt(curlHandle, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, options.followLocation);
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

	curlData->headersSList = headers;

	curl_easy_setopt(curlHandle, CURLOPT_HTTPHEADER, headers);

	if (options.ipv4)
	{
		curl_easy_setopt(curlHandle, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
	}

	impl->client->OnSetupCurlHandle(curlHandle, url);

	return curlData;
}

class HttpRequestHandleImpl final : public ManualHttpRequestHandle
{
public:
	HttpRequestHandleImpl(const std::shared_ptr<CurlData>& reqData)
		: m_request(reqData)
	{

	}

	virtual ~HttpRequestHandleImpl()
	{
		if (!m_started)
		{
			FinalizeCurlHandle(m_request->curlHandle);
		}
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

		std::shared_lock<std::shared_mutex> _(request->impl->mutex);
		if (request->impl->runCb)
		{
			request->impl->runCb->send();
		}
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

			FinalizeCurlHandle(curl, impl->multi);
		});

		std::shared_lock<std::shared_mutex> _(request->impl->mutex);
		if (request->impl->runCb)
		{
			request->impl->runCb->send();
		}
	}

	virtual std::string GetRawBody() override
	{
		return m_request->rawBody.str();
	}

	virtual void Start() override
	{
		m_request->impl->AddCurlHandle(m_request->curlHandle);
		m_started = true;
	}

	virtual void OnCompletion(std::function<void(bool, std::string_view)>&& callback) override
	{
		m_request->callback = std::move(callback);
	}

private:
	std::shared_ptr<CurlData> m_request;

	bool m_started = false;
};

static ManualHttpRequestPtr SetupRequestHandle(const std::shared_ptr<CurlData>& data)
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

ManualHttpRequestPtr HttpClient::Get(const std::string& url, const HttpRequestOptions& options /* = */)
{
	auto curlData = SetupCURLHandle(m_impl, url, options);
	return SetupRequestHandle(curlData);
}

HttpRequestPtr HttpClient::DoGetRequest(const std::string& url, const HttpRequestOptions& options, const std::function<void(bool, const char *, size_t)>& callback)
{
	auto requestHandle = Get(url, options);

	requestHandle->OnCompletion([callback](bool success, std::string_view data)
	{
		callback(success, data.data(), data.size());
	});

	requestHandle->Start();

	return requestHandle;
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
	auto curlData = SetupCURLHandle(m_impl, url, options);

	// assign post data
	curlData->postData = postData;

	curl_easy_setopt(curlData->curlHandle, CURLOPT_POSTFIELDS, curlData->postData.c_str());

	// write out
	auto requestHandle = SetupRequestHandle(curlData);

	requestHandle->OnCompletion([callback](bool success, std::string_view data)
	{
		callback(success, data.data(), data.size());
	});

	requestHandle->Start();

	return requestHandle;
}

HttpRequestPtr HttpClient::DoMethodRequest(const std::string& method, const std::string& url, const std::string& postData, const HttpRequestOptions& options, const std::function<void(bool, const char*, size_t)>& callback, std::function<void(const std::map<std::string, std::string>&)> headerCallback /*= std::function<void(const std::map<std::string, std::string>&)>()*/)
{
	// make handle
	auto curlData = SetupCURLHandle(m_impl, url, options);

	if (!postData.empty())
	{
		// assign post data
		curlData->postData = postData;

		curl_easy_setopt(curlData->curlHandle, CURLOPT_POSTFIELDS, curlData->postData.c_str());
	}

	curl_easy_setopt(curlData->curlHandle, CURLOPT_CUSTOMREQUEST, method.c_str());

	// write out
	auto requestHandle = SetupRequestHandle(curlData);

	requestHandle->OnCompletion([callback](bool success, std::string_view data)
	{
		callback(success, data.data(), data.size());
	});

	requestHandle->Start();

	return requestHandle;
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
	auto curlData = SetupCURLHandle(m_impl, urlStr, options);

	auto handle = outDevice->Create(outFilename);

	curlData->writeFunction = [=] (const void* data, size_t length)
	{
		return outDevice->Write(handle, data, length);
	};

	curlData->preCallback = [=] ()
	{
		outDevice->Close(handle);
	};

	auto requestHandle = SetupRequestHandle(curlData);

	requestHandle->OnCompletion([callback](bool success, std::string_view data)
	{
		callback(success, data.data(), data.size());
	});

	requestHandle->Start();

	return requestHandle;
}

HttpRequestPtr HttpClient::DoFileGetRequest(const std::wstring& host, uint16_t port, const std::wstring& url, rage::fiDevice* outDevice, const std::string& outFilename, const std::function<void(bool, const char*, size_t)>& callback)
{
	return DoFileGetRequest(host, port, url, vfs::GetNativeDevice(outDevice), outFilename, callback);
}

#if defined(COMPILING_HTTP_CLIENT)
static InitFunction initFunction([]()
{
	Instance<HttpClient>::Set(new HttpClient());
});
#endif
