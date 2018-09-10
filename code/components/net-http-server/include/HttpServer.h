/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "TcpServer.h"

#include <forward_list>

namespace net
{
struct HeaderComparator : std::binary_function<std::string, std::string, bool>
{
	bool operator()(const std::string& left, const std::string& right) const
	{
		return _stricmp(left.c_str(), right.c_str()) < 0;
	}
};

typedef std::multimap<std::string, std::string, HeaderComparator> HeaderMap;

class HttpRequest : public fwRefCountable
{
private:
	int m_httpVersionMajor;
	int m_httpVersionMinor;

	std::string m_requestMethod;

	std::string m_path;

	std::string m_remoteAddress;

	HeaderMap m_headerList;

	std::function<void(const std::vector<uint8_t>&)> m_dataHandler;

	std::function<void()> m_cancelHandler;

public:
	HttpRequest(int httpVersionMajor, int httpVersionMinor, const std::string& requestMethod, const std::string& path, const HeaderMap& headerList, const std::string& remoteAddress);

	virtual ~HttpRequest() override;

	inline const std::function<void(const std::vector<uint8_t>& data)>& GetDataHandler() const
	{
		return m_dataHandler;
	}

	inline void SetDataHandler(const std::function<void(const std::vector<uint8_t>& data)>& handler)
	{
		m_dataHandler = handler;
	}

	inline const std::function<void()>& GetCancelHandler() const
	{
		return m_cancelHandler;
	}

	inline void SetCancelHandler(const std::function<void()>& handler)
	{
		m_cancelHandler = handler;
	}

	inline std::pair<int, int> GetHttpVersion() const
	{
		return std::make_pair(m_httpVersionMajor, m_httpVersionMinor);
	}

	inline const std::string& GetRequestMethod() const
	{
		return m_requestMethod;
	}

	inline const std::string& GetPath() const
	{
		return m_path;
	}

	inline const HeaderMap& GetHeaders() const
	{
		return m_headerList;
	}

	inline const std::string& GetHeader(const std::string& key, const std::string& default_ = std::string()) const
	{
		auto it = m_headerList.find(key);

		return (it != m_headerList.end()) ? it->second : default_;
	}

	inline const std::string& GetRemoteAddress() const
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
};

class
#ifdef COMPILING_NET_HTTP_SERVER
	DLL_EXPORT
#endif
	HttpResponse : public fwRefCountable
{
protected:
	fwRefContainer<HttpRequest> m_request;

	int m_statusCode;

	bool m_ended;

	bool m_sentHeaders;

	bool m_closeConnection;

	HeaderMap m_headerList;

protected:
	static std::string GetStatusMessage(int statusCode);

public:
	HttpResponse(fwRefContainer<HttpRequest> request);

	std::string GetHeader(const std::string& name);

	void RemoveHeader(const std::string& name);

	void SetHeader(const std::string& name, const std::string& value);

	void SetHeader(const std::string& name, const std::vector<std::string>& values);

	void WriteHead(int statusCode);

	void WriteHead(int statusCode, const HeaderMap& headers);

	void WriteHead(int statusCode, const std::string& statusMessage);

	virtual void WriteHead(int statusCode, const std::string& statusMessage, const HeaderMap& headers) = 0;

	void Write(const std::string& data);

	virtual void End() = 0;

	virtual void BeforeWriteHead(const std::string& data);

	virtual void WriteOut(const std::vector<uint8_t>& data) = 0;

	void End(const std::string& data);

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

class HttpServer : public HttpServerBase
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
