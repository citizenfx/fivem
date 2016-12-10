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

class InternalRPCHandler : public CefResourceHandler, public nui::RPCHandlerManager
{
private:
	bool m_found;

	std::string m_result;

	uint32_t m_cursor;

	std::unordered_map<std::string, TEndpointFn> m_endpointHandlers;

public:
	virtual bool ProcessRequest(CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback) override;

	virtual void GetResponseHeaders(CefRefPtr<CefResponse> response, int64& response_length, CefString& redirectUrl) override;

	virtual void Cancel() override;

	virtual bool ReadResponse(void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefCallback> callback) override;

	// RPCHandlerManager implementation
	virtual void RegisterEndpoint(std::string endpointName, TEndpointFn callback) override;

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

	auto it = m_endpointHandlers.find(endpoint);

	if (it == m_endpointHandlers.end() || endpointPos == std::string::npos)
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

			// split the string by the usual post map characters
			int curPos = 0;

			while (true)
			{
				int endPos = postDataString.find_first_of('&', curPos);

				int equalsPos = postDataString.find_first_of('=', curPos);

				std::string key;
				std::string value;

				UrlDecode(postDataString.substr(curPos, equalsPos - curPos), key);
				UrlDecode(postDataString.substr(equalsPos + 1, endPos - equalsPos - 1), value);

				postMap[key] = value;

				// save and continue
				curPos = endPos;

				if (curPos == std::string::npos)
				{
					break;
				}

				curPos++;
			}
		}
	}

	handler(funcName, args, postMap, [=] (std::string callResult)
	{
		m_result = callResult;

		callback->Continue();
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

void InternalRPCHandler::Cancel()
{

}

bool InternalRPCHandler::ReadResponse(void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefCallback> callback)
{
	if (m_found)
	{
		int toRead = min(m_result.size() - m_cursor, (size_t)bytes_to_read);

		memcpy(data_out, &m_result.c_str()[m_cursor], toRead);

		m_cursor += toRead;

		bytes_read = toRead;

		return (bytes_read > 0);
	}

	return false;
}

void InternalRPCHandler::RegisterEndpoint(std::string endpointName, TEndpointFn callback)
{
	m_endpointHandlers[endpointName] = callback;
}

static CefRefPtr<InternalRPCHandler> rpcHandler;

static InitFunction prefunction([] ()
{
    rpcHandler = new InternalRPCHandler();

    Instance<nui::RPCHandlerManager>::Set(rpcHandler.get());
});

static HookFunction initFunction([] ()
{
	OnSchemeCreateRequest.Connect([] (const char* scheme, CefRefPtr<CefRequest> request, CefRefPtr<CefResourceHandler>& handler)
	{
		if (!strcmp(scheme, "http"))
		{
			// parse the URL to get the hostname
			CefString url = request->GetURL();
			CefURLParts urlParts;

			if (CefParseURL(url, urlParts))
			{
				CefString hostString = &urlParts.host;

				if (hostString == "nui-internal")
				{
					handler = rpcHandler;

					return false;
				}
			}
		}

		return true;
	}, -100);

	CefRegisterSchemeHandlerFactory("http", "nui-internal", Instance<NUISchemeHandlerFactory>::Get());
});