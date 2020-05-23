/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "CefOverlay.h"
#include "NUISchemeHandlerFactory.h"
#include <NUIClient.h>

#include <VFSManager.h>
#include <include/cef_parser.h>

#include "memdbgon.h"

static std::atomic<int> g_fileHandleCount;
static std::queue<std::function<void()>> g_deleteQueue;

static nui::TResourceLookupFn g_resourceLookupFunc;

namespace nui
{
	void SetResourceLookupFunction(const nui::TResourceLookupFn& fn)
	{
		g_resourceLookupFunc = fn;
	}
}

class NUIResourceHandler : public CefResourceHandler
{
private:
	std::string mimeType_;

	bool dataManaged_;

	int read_;

	fwRefContainer<vfs::Device> device_;

	std::string filename_;

	uintptr_t file_;

	bool closed_;
public:
	NUIResourceHandler()
	{
		closed_ = false;
		file_ = -1;
	}

	virtual ~NUIResourceHandler()
	{
		if (file_ && file_ != -1 && !closed_)
		{
			device_->Close(file_);
		}

		g_fileHandleCount.fetch_sub(1);

		if (!g_deleteQueue.empty())
		{
			g_deleteQueue.front()();
			g_deleteQueue.pop();
		}
	}

	virtual bool ProcessRequest(CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback)
	{
		std::string url = request->GetURL();
		std::wstring hostname;
		std::wstring path;

		CefURLParts parts;
		CefParseURL(url, parts);

		hostname = CefString(&parts.host);
		path = CefString(&parts.path);

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;

		if (hostname == L"game" || hostname == L"nui-game-internal")
		{
			filename_ = "citizen:/";
			
			filename_ += converter.to_bytes(path).substr(1);
		}
		else
		{
			if (hostname.find(L"cfx-nui-") == 0)
			{
				hostname = hostname.substr(8);
			}

			if (g_resourceLookupFunc)
			{
				filename_ = g_resourceLookupFunc(ToNarrow(hostname));
			}
			else
			{
				filename_ = "resources:/";
				filename_ += converter.to_bytes(hostname) + "/";
			}

			filename_ += converter.to_bytes(path);
		}

		// remove # parts
		auto hash = filename_.find_first_of(L'#');

		if (hash != std::string::npos)
		{
			//filename_.resize(hash);
			filename_.erase(hash);
		}

		hash = filename_.find_first_of(L'?');

		if (hash != std::string::npos)
		{
			//filename_.resize(hash);
			filename_.erase(hash);
		}

		if (filename_.length() >= 256)
		{
			filename_ = filename_.substr(0, 255);
		}

		device_ = vfs::GetDevice(filename_);

		int count = g_fileHandleCount.fetch_add(1);

		auto handleOpen = [=] ()
		{
			if (device_.GetRef() && filename_.find("..") == std::string::npos)
			{
				file_ = device_->Open(filename_.c_str(), true);
			}

			// set mime type
			std::string ext = url.substr(url.rfind('.') + 1);

			mimeType_ = "text/html";

			if (ext == "png")
			{
				mimeType_ = "image/png";
			}
			else if (ext == "js")
			{
				mimeType_ = "application/javascript";
			}
			else if (ext == "css")
			{
				mimeType_ = "text/css";
			}
			else if (ext == "svg")
			{
				mimeType_ = "image/svg+xml";
			}
			else if (ext == "wasm")
			{
				mimeType_ = "application/wasm";
			}

			callback->Continue();
		};

		if (count < 2)
		{
			handleOpen();
		}
		else
		{
			g_deleteQueue.push(handleOpen);
		}

		return true;
	}

	virtual void GetResponseHeaders(CefRefPtr<CefResponse> response, int64& response_length, CefString& redirectUrl)
	{
		response->SetMimeType(mimeType_);

		if (file_ == -1)
		{
			response->SetStatus(404);
		}
		else
		{
			response->SetStatus(200);
		}

		CefResponse::HeaderMap map;
		response->GetHeaderMap(map);

		if (mimeType_ == "text/html" || mimeType_ == "text/css" || mimeType_ == "application/javascript")
		{
			map.insert({ "content-type", mimeType_ + "; charset=utf-8" });
		}

		map.insert({ "cache-control", "no-cache, must-revalidate" });
		response->SetHeaderMap(map);

		if (file_ != -1)
		{
			response_length = device_->GetLength(file_);
		}
		else
		{
			response_length = 0;
		}
	}

	virtual void Cancel()
	{
		closed_ = true;

		if (file_ != -1)
		{
			device_->Close(file_);
		}
	}

	virtual bool ReadResponse(void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefCallback> callback)
	{
		if (file_ != -1)
		{
			bytes_read = device_->Read(file_, data_out, bytes_to_read);

			return (bytes_read > 0);
		}

		return false;
	}

	IMPLEMENT_REFCOUNTING(NUIResourceHandler);
};

class ForbiddenResourceHandler : public CefResourceHandler
{
public:
	IMPLEMENT_REFCOUNTING(ForbiddenResourceHandler);

	virtual bool ProcessRequest(CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback) override
	{
		callback->Continue();
		return true;
	}

	virtual void GetResponseHeaders(CefRefPtr<CefResponse> response, int64& response_length, CefString& redirectUrl) override
	{
		response->SetStatus(403);
		response_length = 0;
	}

	virtual bool ReadResponse(void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefCallback> callback) override
	{
		bytes_read = 0;

		return true;
	}

	virtual void Cancel() override
	{
	}
};

void NUISchemeHandlerFactory::SetRequestBlacklist(const std::vector<std::regex>& requestBlacklist)
{
	m_requestBlacklist = requestBlacklist;
}

CefRefPtr<CefResourceHandler> NUISchemeHandlerFactory::Create(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& scheme_name, CefRefPtr<CefRequest> request)
{
	if (scheme_name == "nui")
	{
		return new NUIResourceHandler();
	}
	else if (scheme_name == "https")
	{
		// parse the URL to get the hostname
		CefString url = request->GetURL();
		CefURLParts urlParts;

		if (CefParseURL(url, urlParts))
		{
			CefString hostString = &urlParts.host;

			if (hostString == "nui-game-internal")
			{
				return new NUIResourceHandler();
			}
			else if (hostString.ToString().find("cfx-nui-") == 0)
			{
				return new NUIResourceHandler();
			}
		}
	}
	else if (scheme_name == "ws" || scheme_name == "wss")
	{
		for (auto& reg : m_requestBlacklist)
		{
			std::string url = request->GetURL().ToString();

			if (std::regex_search(url, reg))
			{
				trace("Blocked a request for blacklisted URI %s\n", url);
				return new ForbiddenResourceHandler();
			}
		}

		return nullptr;
	}

	CefRefPtr<CefResourceHandler> outHandler;

	OnSchemeCreateRequest(scheme_name.ToString().c_str(), request, outHandler);

	return outHandler;
}

OVERLAY_DECL fwEvent<const char*, CefRefPtr<CefRequest>, CefRefPtr<CefResourceHandler>&> OnSchemeCreateRequest;
