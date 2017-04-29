/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "TcpServer.h"

namespace net
{
struct HeaderComparator : std::binary_function<std::string, std::string, bool>
{
	bool operator()(const std::string& left, const std::string& right) const
	{
		return _stricmp(left.c_str(), right.c_str()) < 0;
	}
};

typedef std::map<std::string, std::string, HeaderComparator> HeaderMap;

class HttpRequest : public fwRefCountable
{
private:
	int m_httpVersionMajor;
	int m_httpVersionMinor;

	std::string m_requestMethod;

	std::string m_path;

	HeaderMap m_headerList;

	std::function<void(const std::vector<uint8_t>&)> m_dataHandler;

public:
	HttpRequest(int httpVersionMajor, int httpVersionMinor, const std::string& requestMethod, const std::string& path, const HeaderMap& headerList);

	virtual ~HttpRequest() override;

	inline const std::function<void(const std::vector<uint8_t>& data)>& GetDataHandler() const
	{
		return m_dataHandler;
	}

	inline void SetDataHandler(const std::function<void(const std::vector<uint8_t>& data)>& handler)
	{
		m_dataHandler = handler;
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

	inline const std::string& GetHeader(const std::string& key, const std::string& default = std::string()) const
	{
		auto it = m_headerList.find(key);

		return (it != m_headerList.end()) ? it->second : default;
	}
};

class
#ifdef COMPILING_NET_HTTP_SERVER
	DLL_EXPORT
#endif
	HttpResponse : public fwRefCountable
{
private:
	fwRefContainer<HttpRequest> m_request;

	fwRefContainer<TcpServerStream> m_clientStream;

	int m_statusCode;

	bool m_ended;

	bool m_sentHeaders;

	bool m_closeConnection;

	HeaderMap m_headerList;

private:
	static std::string GetStatusMessage(int statusCode);

public:
	HttpResponse(fwRefContainer<TcpServerStream> clientStream, fwRefContainer<HttpRequest> request);

	std::string GetHeader(const std::string& name);

	void RemoveHeader(const std::string& name);

	void SetHeader(const std::string& name, const std::string& value);

	void WriteHead(int statusCode);

	void WriteHead(int statusCode, const HeaderMap& headers);

	void WriteHead(int statusCode, const std::string& statusMessage);

	void WriteHead(int statusCode, const std::string& statusMessage, const HeaderMap& headers);

	void Write(const std::string& data);

	void End();

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

class HttpServer : public fwRefCountable
{
public:
	virtual void AttachToServer(fwRefContainer<TcpServer> server) = 0;

	virtual void RegisterHandler(fwRefContainer<HttpHandler> handler) = 0;
};
};