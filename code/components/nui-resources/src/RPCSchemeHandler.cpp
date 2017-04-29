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

		CefURLParts parts;
		CefParseURL(url, parts);

		hostname = CefString(&parts.host);
		path = CefString(&parts.path);

		fwString host(hostname.begin(), hostname.end());

		fx::ResourceManager* resourceManager = Instance<fx::ResourceManager>::Get();
		auto resource = resourceManager->GetResource(host);

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

		std::string postDataString = "null";

		if (request->GetMethod() == "POST")
		{
			auto postData = request->GetPostData();

			if (postData.get())
			{
				CefPostData::ElementVector elements;
				postData->GetElements(elements);

				auto element = elements[0];

				char* bytes = new char[element->GetBytesCount()];
				element->GetBytes(element->GetBytesCount(), bytes);

				postDataString = std::string(bytes, element->GetBytesCount());

				delete[] bytes;
			}
		}

		auto result = ui->InvokeCallback(path.substr(1), postDataString, [=] (const std::string& callResult)
		{
			m_result = callResult;

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
		response->SetMimeType("application/json");

		if (!m_found)
		{
			response->SetStatus(404);
		}
		else
		{
			response->SetStatus(200);
		}

		CefResponse::HeaderMap map;
		response->GetHeaderMap(map);

		map.insert(std::make_pair("cache-control", "no-cache, must-revalidate"));
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
	uint32_t m_cursor;

public:
	virtual bool ReadResponse(void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefCallback> callback)
	{
		if (m_found)
		{
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
		if (!strcmp(name, "http"))
		{
			handler = new RPCResourceHandler();
		}
	});
});