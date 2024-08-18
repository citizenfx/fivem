/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "CefOverlay.h"

#include "ResourceManager.h"
#include "ResourceUI.h"

#include <include/cef_parser.h>

#ifdef min
#undef min
#endif

class RPCResourceHandler : public CefResourceHandler
{
private:
	bool m_found;

	fwString m_result;
	std::multimap<std::string, std::string> m_headers;
	int m_statusCode = 200;

	std::mutex m_mutex;

public:
	RPCResourceHandler()
	{
		m_found = false;
	}

	virtual ~RPCResourceHandler()
	{
	}

	virtual bool ProcessRequest(CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback)
	{
		std::string url = request->GetURL();
		std::string hostname;
		std::string path;
		std::string query;

		CefURLParts parts;
		CefParseURL(url, parts);

		hostname = CefString(&parts.host);
		path = CefString(&parts.path);
		query = CefString(&parts.query);

		fwString host(hostname.begin(), hostname.end());

		fx::ResourceManager* resourceManager = Instance<fx::ResourceManager>::Get();
		fwRefContainer<fx::Resource> resource;

		resourceManager->ForAllResources([&host, &resource](const fwRefContainer<fx::Resource>& resourceRef)
		{
			if (_stricmp(resourceRef->GetName().c_str(), host.c_str()) == 0)
			{
				resource = resourceRef;
			}
		});

		if (!resource.GetRef())
		{
			m_found = false;

			callback->Continue();

			return true;
		}

		fwRefContainer<ResourceUI> ui = resource->GetComponent<ResourceUI>();

		// remove # parts
		auto hash = path.find_first_of('#');

		if (hash != std::string::npos)
		{
			path.erase(hash);
		}

		hash = path.find_first_of('?');

		if (hash != std::string::npos)
		{
			path.erase(hash);
		}

		std::string postDataString;

		if (request->GetMethod() == "POST")
		{
			auto postData = request->GetPostData();

			if (postData.get())
			{
				CefPostData::ElementVector elements;
				postData->GetElements(elements);

				// count the amount of bytes
				size_t size = 0;

				for (auto& element : elements)
				{
					if (element->GetType() == PDE_TYPE_BYTES)
					{
						size += element->GetBytesCount();
					}
				}

				// allocate a temporary buffer
				std::vector<char> tempBytes(size);

				// get data for each element
				size_t curPos = 0;

				for (auto& element : elements)
				{
					if (element->GetType() == PDE_TYPE_BYTES)
					{
						size_t thisSize = element->GetBytesCount();
						element->GetBytes(thisSize, &tempBytes[curPos]);

						curPos += thisSize;
					}
				}
				
				// assign post data string
				postDataString = std::string(tempBytes.data(), tempBytes.size());
			}
		}

		std::multimap<std::string, std::string> headers;

		CefRequest::HeaderMap origHeaders;
		request->GetHeaderMap(origHeaders);

		for (auto& header : origHeaders)
		{
			headers.emplace(header.first.ToString(), header.second.ToString());
		}

		CefRefPtr<RPCResourceHandler> self = this;

		auto result = ui->InvokeCallback(path.substr(1), query, headers, postDataString, [self, callback] (int statusCode, const std::multimap<std::string, std::string>& headers, const std::string& callResult)
		{
			{
				std::unique_lock _(self->m_mutex);
				self->m_headers = headers;
				self->m_statusCode = statusCode;
				self->m_result = callResult;
			}

			callback->Continue();
		});

		if (!result)
		{
			m_found = false;

			callback->Continue();

			return true;
		}

		m_found = true;

		return true;
	}

	virtual void GetResponseHeaders(CefRefPtr<CefResponse> response, int64& response_length, CefString& redirectUrl)
	{
		std::unique_lock _(m_mutex);

		if (auto hit = m_headers.find("content-type"); hit != m_headers.end())
		{
			response->SetMimeType(hit->second.substr(0, hit->second.find_first_of(';')));
		}
		else
		{
			response->SetMimeType("application/json");
		}
		
		if (!m_found)
		{
			response->SetStatus(404);
		}
		else
		{
			response->SetStatus(m_statusCode);
		}

		CefResponse::HeaderMap map;
		response->GetHeaderMap(map);

		if (!m_headers.empty())
		{
			for (auto& header : m_headers)
			{
				map.emplace(header.first, header.second);
			}
		}
		
		map.insert({ "cache-control", "no-cache, must-revalidate" });
		if (map.find("access-control-allow-origin") == map.end())
		{
			map.insert({ "access-control-allow-origin", "*" });
		}
		if (map.find("access-control-allow-methods") == map.end())
		{
			map.insert({ "access-control-allow-methods", "POST, GET, OPTIONS" });
		}

		response->SetHeaderMap(map);

		if (m_found)
		{
			response_length = m_result.size();
		}
		else
		{
			response_length = 0;
		}

		m_cursor = 0;
	}

	virtual void Cancel()
	{
		// TODO: try to remove some call token?
	}

private:
	uint32_t m_cursor = 0;

public:
	virtual bool ReadResponse(void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefCallback> callback)
	{
		if (m_found)
		{
			std::unique_lock _(m_mutex);

			int toRead = std::min((unsigned int)m_result.size() - m_cursor, (unsigned int)bytes_to_read);

			memcpy(data_out, &m_result.c_str()[m_cursor], toRead);

			m_cursor += toRead;
			bytes_read = toRead;

			return (bytes_read > 0);
		}

		return false;
	}

	IMPLEMENT_REFCOUNTING(RPCResourceHandler);
};


static InitFunction initFunction([] ()
{
	OnSchemeCreateRequest.Connect([] (const char* name, CefRefPtr<CefRequest> request, CefRefPtr<CefResourceHandler>& handler)
	{
		if (!strcmp(name, "http") || !strcmp(name, "https"))
		{
			handler = new RPCResourceHandler();
		}
	});
});
