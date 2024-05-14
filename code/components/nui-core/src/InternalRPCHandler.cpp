/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "InternalRPCHandler.h"

#include "CefOverlay.h"
#include <include/cef_parser.h>
#include "NUISchemeHandlerFactory.h"

#include <HookFunction.h>

#include "memdbgon.h"

/// Lazy singleton
class InternalHandlerMap : public nui::RPCHandlerManager, public std::unordered_map<std::string, nui::RPCHandlerManager::TEndpointFn>
{
public:
	virtual void RegisterEndpoint(std::string endpointName, TEndpointFn callback) override
	{
		emplace(std::move(endpointName), std::move(callback));
	}

private:
	InternalHandlerMap() {};
public:
	InternalHandlerMap(InternalHandlerMap const&) = delete;
	InternalHandlerMap& operator=(InternalHandlerMap const&) = delete;

	static InternalHandlerMap& GetInstance()
	{
		static InternalHandlerMap instance;
		return instance;
	}
};

class InternalRPCHandler : public CefResourceHandler
{
private:
	struct Response
	{
		std::string m_result;

		uint32_t m_cursor = 0;

		bool m_cancelled = false;
	};

	bool m_found;

	std::shared_ptr<Response> m_response;

public:
	InternalRPCHandler()
		: m_found(false), m_response(std::make_shared<Response>())
	{
	}

	virtual bool ProcessRequest(CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback) override;

	virtual void GetResponseHeaders(CefRefPtr<CefResponse> response, int64& response_length, CefString& redirectUrl) override;

	virtual void Cancel() override;

	virtual bool ReadResponse(void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefCallback> callback) override;

	IMPLEMENT_REFCOUNTING(InternalRPCHandler);
};

bool InternalRPCHandler::ProcessRequest(CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback)
{
	std::string url = request->GetURL();
	std::string hostname;
	std::string path;

	CefURLParts parts;
	CefParseURL(url, parts);

	// convert the path to a string
	path = CefString(&parts.path);
	path = path.substr(1);

	// find the endpoint
	int endpointPos = path.find_first_of('/');
	std::string endpoint = path.substr(0, endpointPos);

	auto& endpointHandlers = InternalHandlerMap::GetInstance();
	auto it = endpointHandlers.find(endpoint);

	if (it == endpointHandlers.end() || endpointPos == std::string::npos)
	{
		m_found = false;

		callback->Continue();

		return true;
	}

	auto handler = it->second;

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

	// find the function name
	int funcEnd = path.find_first_of('/', endpointPos + 1);
	std::string funcName = path.substr(endpointPos + 1, funcEnd - (endpointPos + 1));

	// add additional arguments
	std::string args = (funcEnd != std::string::npos) ? path.substr(funcEnd + 1) : "";

	// get post data and parse it into a map
	std::map<std::string, std::string> postMap;

	if (request->GetMethod() == "POST")
	{
		// get the first element
		auto postData = request->GetPostData();

		if (postData.get())
		{
			CefPostData::ElementVector elements;
			postData->GetElements(elements);

			auto element = elements[0];

			// get bytes and put them into a string
			char* bytes = new char[element->GetBytesCount()];
			element->GetBytes(element->GetBytesCount(), bytes);

			std::string postDataString = std::string(bytes, element->GetBytesCount());

			delete[] bytes;

			postMap = ParsePOSTString(postDataString);
		}
	}

	std::weak_ptr weakState(m_response);
	handler(funcName, args, postMap, [=] (std::string callResult)
	{
		if (auto state = weakState.lock(); state && !state->m_cancelled)
		{
			state->m_result = callResult;

			callback->Continue();
		}
	});

	m_found = true;

	return true;
}

void InternalRPCHandler::GetResponseHeaders(CefRefPtr<CefResponse> response, int64& response_length, CefString& redirectUrl)
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

	map.insert({ "cache-control", "no-cache, must-revalidate" });
	map.insert({ "access-control-allow-origin", "*" });
	map.insert({ "access-control-allow-methods", "POST, GET, OPTIONS" });
	response->SetHeaderMap(map);

	if (m_found)
	{
		response_length = m_response->m_result.size();
	}
	else
	{
		response_length = 0;
	}

	m_response->m_cursor = 0;
}

void InternalRPCHandler::Cancel()
{
	m_response->m_cancelled = true;
}

bool InternalRPCHandler::ReadResponse(void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefCallback> callback)
{
	if (m_found)
	{
		auto& m_result = m_response->m_result;
		auto& m_cursor = m_response->m_cursor;
		int toRead = fwMin(m_result.size() - m_cursor, (size_t)bytes_to_read);

		memcpy(data_out, &m_result.c_str()[m_cursor], toRead);

		m_cursor += toRead;

		bytes_read = toRead;

		return (bytes_read > 0);
	}

	return false;
}

static InitFunction prefunction([] ()
{
    Instance<nui::RPCHandlerManager>::Set(&InternalHandlerMap::GetInstance());
});

static HookFunction initFunction([] ()
{
	OnSchemeCreateRequest.Connect([] (const char* scheme, CefRefPtr<CefRequest> request, CefRefPtr<CefResourceHandler>& handler)
	{
		if (!strcmp(scheme, "http") || !strcmp(scheme, "https"))
		{
			// parse the URL to get the hostname
			CefString url = request->GetURL();
			CefURLParts urlParts;

			if (CefParseURL(url, urlParts)) 
			{
				CefString hostString = &urlParts.host;

				if (hostString == "nui-internal")
				{
					handler = new InternalRPCHandler();

					return false;
				}
			}
		}

		return true;
	}, -100);

	nui::RegisterSchemeHandlerFactory("http", "nui-internal", Instance<NUISchemeHandlerFactory>::Get());
	nui::RegisterSchemeHandlerFactory("https", "nui-internal", Instance<NUISchemeHandlerFactory>::Get());
});
