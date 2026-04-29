#include <StdInc.h>

#if __has_include(<CefOverlay.h>)
#include <include/cef_parser.h>

#include <CefOverlay.h>

#include <HttpClient.h>
#include <NetLibrary.h>

class NucleusResourceHandler : public CefResourceHandler
{
public:
	virtual bool ProcessRequest(CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback) override
	{
		CefRefPtr<NucleusResourceHandler> self = this;

		std::string url = request->GetURL().ToString();
		std::string method = request->GetMethod().ToString();

		// Replace the host in the URL with the connected server's address
		if (m_netLibrary && m_netLibrary->GetConnectionState() == NetLibrary::CS_ACTIVE)
		{
			CefURLParts urlParts;
			if (CefParseURL(url, urlParts))
			{
				auto peerAddress = m_netLibrary->GetCurrentPeer().ToString();
				if (!peerAddress.empty())
				{
					std::string scheme = CefString(&urlParts.scheme).ToString();
					std::string path = CefString(&urlParts.path).ToString();
					std::string query = CefString(&urlParts.query).ToString();

					url = scheme + "://" + peerAddress + path;
					if (!query.empty())
					{
						url += "?" + query;
					}
				}
			}
		}

		// Extract post data
		std::string postData;
		if (request->GetPostData())
		{
			CefRefPtr<CefPostData> data = request->GetPostData();
			CefPostData::ElementVector elements;
			data->GetElements(elements);

			for (auto& element : elements)
			{
				if (element->GetType() == PDE_TYPE_BYTES)
				{
					size_t elementSize = element->GetBytesCount();
					std::vector<char> buffer(elementSize);
					element->GetBytes(elementSize, buffer.data());
					postData.append(buffer.data(), elementSize);
				}
			}
		}

		// Extract request headers
		CefRequest::HeaderMap headerMap;
		request->GetHeaderMap(headerMap);

		HttpRequestOptions options;
		for (const auto& [key, value] : headerMap)
		{
			options.headers[key.ToString()] = value.ToString();
		}

		auto responseHeaders = std::make_shared<HttpHeaderList>();
		auto responseCode = std::make_shared<int>(0);
		options.responseHeaders = responseHeaders;
		options.responseCode = responseCode;
		options.addErrorBody = true;

		Instance<HttpClient>::Get()->DoMethodRequest(method, url, postData, options,
			[self, responseHeaders, responseCode, callback](bool success, const char* data, size_t length)
			{
				if (!success && *responseCode == 0)
				{
					self->m_statusCode = 502;
					self->m_statusText = "Bad Gateway";
					self->m_mimeType = "text/plain";
					self->m_responseBody = std::string(data, length);
					if (self->m_responseBody.empty())
					{
						self->m_responseBody = "Request failed";
					}
				}
				else
				{
					self->m_statusCode = *responseCode;
					self->m_statusText = (self->m_statusCode >= 200 && self->m_statusCode < 300) ? "OK" : "Error";

					// Extract content type from response headers
					auto contentTypeIt = responseHeaders->find("content-type");
					if (contentTypeIt != responseHeaders->end())
					{
						std::string ct = contentTypeIt->second;
						auto semicolonPos = ct.find(';');
						self->m_mimeType = (semicolonPos != std::string::npos) ? ct.substr(0, semicolonPos) : ct;
					}
					else
					{
						self->m_mimeType = "application/octet-stream";
					}

					// Store response headers
					for (const auto& [key, value] : *responseHeaders)
					{
						self->m_responseHeaders.emplace(key, value);
					}

					self->m_responseBody = std::string(data, length);
				}

				callback->Continue();
			});

		return true;
	}

	virtual void GetResponseHeaders(CefRefPtr<CefResponse> response, int64& responseLength, CefString& redirectUrl) override
	{
		response->SetStatus(m_statusCode);
		response->SetStatusText(m_statusText);
		response->SetMimeType(m_mimeType);

		CefResponse::HeaderMap headers;
		response->GetHeaderMap(headers);

		for (const auto& [key, value] : m_responseHeaders)
		{
			headers.emplace(key, value);
		}

		// Prevent CORS issues
		headers.erase("access-control-allow-origin");
		headers.erase("access-control-allow-methods");
		headers.erase("access-control-allow-headers");
		headers.erase("access-control-expose-headers");
		headers.erase("access-control-max-age");
		headers.insert({"access-control-allow-origin", "*"});
		headers.insert({"access-control-allow-methods", "GET, POST, PUT, PATCH, DELETE, OPTIONS"});
		headers.insert({"access-control-allow-headers", "*"});
		headers.insert({"access-control-expose-headers", "*"});

		response->SetHeaderMap(headers);

		responseLength = static_cast<int64>(m_responseBody.size());
	}

	virtual bool ReadResponse(void* dataOut, int bytesToRead, int& bytesRead, CefRefPtr<CefCallback> callback) override
	{
		if (m_readOffset >= m_responseBody.size())
		{
			bytesRead = 0;
			return false;
		}

		size_t remaining = m_responseBody.size() - m_readOffset;
		size_t toRead = std::min(remaining, static_cast<size_t>(bytesToRead));
		memcpy(dataOut, m_responseBody.data() + m_readOffset, toRead);
		m_readOffset += toRead;
		bytesRead = static_cast<int>(toRead);

		return bytesRead > 0;
	}

	virtual void Cancel() override
	{
	}

	void SetNetLibrary(NetLibrary* netLibrary)
	{
		m_netLibrary = netLibrary;
	}

private:
	NetLibrary* m_netLibrary = nullptr;

	int m_statusCode = 0;
	std::string m_statusText;
	std::string m_mimeType;
	std::string m_responseBody;
	std::multimap<std::string, std::string> m_responseHeaders;
	size_t m_readOffset = 0;

	IMPLEMENT_REFCOUNTING(NucleusResourceHandler);
};

static bool HostMatchesUsersCfxRe(const std::string& host)
{
	// Match *.users.cfx.re
	static const std::string suffix = ".users.cfx.re";
	if (host.size() > suffix.size())
	{
		return host.compare(host.size() - suffix.size(), suffix.size(), suffix) == 0;
	}
	return false;
}

static InitFunction initFunction([]()
{
	static NetLibrary* netLibrary = nullptr;

	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* lib)
	{
		netLibrary = lib;
	});

	OnGetResourceHandler.Connect([](CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefResourceHandler>& handler)
	{
		CefString url = request->GetURL();
		CefURLParts urlParts;

		if (CefParseURL(url, urlParts))
		{
			std::string hostString = CefString(&urlParts.host).ToString();

			if (HostMatchesUsersCfxRe(hostString))
			{
				auto nucleusHandler = new NucleusResourceHandler();
				nucleusHandler->SetNetLibrary(netLibrary);
				handler = nucleusHandler;
				return false;
			}
		}

		return true;
	});
});
#endif
