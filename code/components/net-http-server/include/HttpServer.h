/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "TcpServer.h"

#include <forward_list>
#include <shared_mutex>

#include <EASTL/fixed_map.h>
#include <EASTL/fixed_string.h>

#include <optional>

#include "ComponentExport.h"

namespace net
{
struct HeaderComparator
{
	using is_transparent = void;

	template<typename TString, typename TOtherString>
	bool operator()(const TString& left, const TOtherString& right) const
	{
		return std::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end(), [](char a, char b)
		{
			return ToLower(a) < ToLower(b);
		});
	}
};

using HeaderString = eastl::fixed_string<char, 64, true>;
using HeaderMap = eastl::fixed_multimap<HeaderString, HeaderString, 16, true, HeaderComparator>;

class HttpRequest : public fwRefCountable
{
private:
	int m_httpVersionMajor;
	int m_httpVersionMinor;

	HeaderString m_requestMethod;

	HeaderString m_path;

	net::PeerAddress m_remoteAddress;

	HeaderMap m_headerList;

	std::shared_ptr<std::function<void(const std::vector<uint8_t>&)>> m_dataHandler;

	std::shared_mutex m_dataHandlerMutex;

	std::shared_ptr<std::function<void()>> m_cancelHandler;

	std::shared_mutex m_cancelHandlerMutex;

	std::optional<std::vector<uint8_t>> m_pendingData;

public:
	HttpRequest(int httpVersionMajor, int httpVersionMinor, const HeaderString& requestMethod, const HeaderString& path, const HeaderMap& headerList, const net::PeerAddress& remoteAddress);

	virtual ~HttpRequest() override;

	inline std::shared_ptr<std::function<void(const std::vector<uint8_t>& data)>> GetDataHandler()
	{
		std::shared_lock<std::shared_mutex> lock(m_dataHandlerMutex);
		return m_dataHandler;
	}

	inline void SetDataHandler()
	{
		std::unique_lock<std::shared_mutex> lock(m_dataHandlerMutex);
		m_dataHandler = {};
	}

	inline void SetDataHandler(const std::function<void(const std::vector<uint8_t>& data)>& handler)
	{
		if (m_pendingData)
		{
			if (handler)
			{
				handler(*m_pendingData);
			}

			m_pendingData = {};

			return;
		}

		std::unique_lock<std::shared_mutex> lock(m_dataHandlerMutex);
		m_dataHandler = std::make_shared<std::remove_const_t<std::remove_reference_t<decltype(handler)>>>(handler);
	}

	inline void SetPendingData(std::vector<uint8_t>&& data)
	{
		m_pendingData = std::move(data);
	}

	inline std::shared_ptr<std::function<void()>> GetCancelHandler()
	{
		std::shared_lock<std::shared_mutex> lock(m_cancelHandlerMutex);
		return m_cancelHandler;
	}

	inline void SetCancelHandler()
	{
		std::unique_lock<std::shared_mutex> lock(m_cancelHandlerMutex);
		m_cancelHandler = {};
	}

	inline void SetCancelHandler(const std::function<void()>& handler)
	{
		std::unique_lock<std::shared_mutex> lock(m_cancelHandlerMutex);
		m_cancelHandler = std::make_shared<std::remove_const_t<std::remove_reference_t<decltype(handler)>>>(handler);
	}

	inline std::pair<int, int> GetHttpVersion() const
	{
		return std::make_pair(m_httpVersionMajor, m_httpVersionMinor);
	}

	inline const auto& GetRequestMethod() const
	{
		return m_requestMethod;
	}

	inline const auto& GetPath() const
	{
		return m_path;
	}

	inline const HeaderMap& GetHeaders() const
	{
		return m_headerList;
	}

	inline std::string GetHeader(eastl::string_view key, const std::string& default_ = {}) const
	{
		auto it = m_headerList.find_as(key, HeaderComparator{});

		return (it != m_headerList.end()) ? std::string{ it->second.c_str(), it->second.size() } : default_;
	}

	inline std::string GetRemoteAddress() const
	{
		return m_remoteAddress.ToString();
	}

	inline const net::PeerAddress& GetRemotePeer() const
	{
		return m_remoteAddress;
	}
};

struct HttpState
{
	// should this request parser be blocked?
	bool blocked;

	// a function to call when we want to unblock the request
	std::function<void()> ping;

	// a lock for ping being set
	std::mutex pingLock;
};

class COMPONENT_EXPORT(NET_HTTP_SERVER) HttpResponse : public fwRefCountable
{
protected:
	fwRefContainer<HttpRequest> m_request;

	int m_statusCode;

	bool m_ended;

	bool m_sentHeaders;

	bool m_closeConnection;

	HeaderMap m_headerList;

protected:
	static std::string_view GetStatusMessage(int statusCode);

public:
	HttpResponse(fwRefContainer<HttpRequest> request);

	inline auto GetHeader(eastl::string_view key, eastl::string_view default_ = {}) const
	{
		auto it = m_headerList.find_as(key, HeaderComparator{});

		return (it != m_headerList.end()) ? it->second : default_;
	}

	void RemoveHeader(const HeaderString& name);

	void SetHeader(const HeaderString& name, const HeaderString& value);

	void SetHeader(const HeaderString& name, const std::vector<HeaderString>& values);

	inline void SetHeader(const HeaderString& name, const char* value)
	{
		SetHeader(name, HeaderString{ value });
	}

	inline void SetHeader(const HeaderString& name, const std::string& value)
	{
		SetHeader(name, HeaderString{
						value.c_str(), value.size() });
	}

	inline void SetHeader(const HeaderString& name, const std::vector<std::string>& values)
	{
		std::vector<HeaderString> headers;
		headers.reserve(values.size());

		for (auto& value : values)
		{
			headers.push_back(HeaderString{
				value.c_str(), value.size() });
		}

		SetHeader(name, headers);
	}

	void WriteHead(int statusCode);

	void WriteHead(int statusCode, const HeaderMap& headers);

	void WriteHead(int statusCode, const std::string& statusMessage);

	virtual void WriteHead(int statusCode, const std::string& statusMessage, const HeaderMap& headers) = 0;

	void Write(const std::string& data, fu2::unique_function<void(bool)>&& onComplete = {});

	void Write(std::string&& data, fu2::unique_function<void(bool)>&& onComplete = {});

	void Write(std::unique_ptr<char[]> data, size_t length, fu2::unique_function<void(bool)>&& onComplete = {});

	virtual void End() = 0;

	virtual void BeforeWriteHead(size_t length);

	virtual void WriteOut(const std::vector<uint8_t>& data, fu2::unique_function<void(bool)>&& onComplete = {}) = 0;

	virtual void WriteOut(std::unique_ptr<char[]> data, size_t length, fu2::unique_function<void(bool)>&& onComplete = {}) = 0;

	virtual void WriteOut(std::vector<uint8_t>&& data, fu2::unique_function<void(bool)>&& onComplete = {});

	virtual void WriteOut(const std::string& data, fu2::unique_function<void(bool)>&& onComplete = {});

	virtual void WriteOut(std::string&& data, fu2::unique_function<void(bool)>&& onComplete = {});

	void End(const std::string& data);

	void End(std::string&& data);

	virtual void CloseSocket() = 0;

	inline int GetStatusCode()
	{
		return m_statusCode;
	}

	inline void SetStatusCode(int statusCode)
	{
		m_statusCode = statusCode;
	}

	inline bool HasSentHeaders()
	{
		return m_sentHeaders;
	}

	inline bool HasEnded()
	{
		return m_ended;
	}
};

class HttpHandler : public fwRefCountable
{
public:
	virtual bool HandleRequest(fwRefContainer<HttpRequest> request, fwRefContainer<HttpResponse> response) = 0;
};

class HttpServerBase : public fwRefCountable
{
public:
	virtual void AttachToServer(fwRefContainer<TcpServer> server) = 0;

	virtual void RegisterHandler(fwRefContainer<HttpHandler> handler) = 0;
};

class COMPONENT_EXPORT(NET_HTTP_SERVER) HttpServer : public HttpServerBase
{
public:
	virtual void AttachToServer(fwRefContainer<TcpServer> server) override;

	virtual void RegisterHandler(fwRefContainer<HttpHandler> handler) override;

protected:
	virtual void OnConnection(fwRefContainer<TcpServerStream> stream) = 0;

protected:
	fwRefContainer<TcpServer> m_server;

	std::forward_list<fwRefContainer<HttpHandler>> m_handlers;
};
};
