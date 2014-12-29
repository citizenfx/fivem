/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "CefOverlay.h"
#include "NUISchemeHandlerFactory.h"

#include <fiDevice.h>
#include <include/cef_url.h>

class NUIResourceHandler : public CefResourceHandler
{
private:
	std::string mimeType_;

	bool dataManaged_;

	int read_;

	rage::fiDevice* device_;

	std::string filename_;

	uint32_t file_;

	bool closed_;
public:
	NUIResourceHandler()
	{
		closed_ = false;
		file_ = -1;
	}

	virtual ~NUIResourceHandler()
	{
		if (file_ && !closed_)
		{
			device_->close(file_);
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

		if (hostname == L"game")
		{
			filename_ = "citizen:/";
			
			filename_ += std::string(path.begin(), path.end());
		}
		else
		{
			filename_ = "resources:/";
			filename_ += std::string(hostname.begin(), hostname.end()) + "/";
			filename_ += std::string(path.begin(), path.end());
		}

		//filename_ = exeName + std::wstring(L"/citiv/ui/") + url.substr(11);

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

		//file_ = _wfopen(filename_.c_str(), "rb");
		device_ = rage::fiDevice::GetDevice(filename_.c_str(), true);
		
		if (device_)
		{
			file_ = device_->open(filename_.c_str(), true);
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

		callback->Continue();

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

		map.insert(std::make_pair("cache-control", "no-cache, must-revalidate"));
		response->SetHeaderMap(map);

		if (file_ != -1)
		{
			response_length = device_->fileLength(file_);
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
			device_->close(file_);
		}
	}

	virtual bool ReadResponse(void* data_out, int bytes_to_read, int& bytes_read, CefRefPtr<CefCallback> callback)
	{
		if (file_ != -1)
		{
			bytes_read = device_->read(file_, data_out, bytes_to_read);

			return (bytes_read > 0);
		}

		return false;
	}

	IMPLEMENT_REFCOUNTING(NUIResourceHandler);
};

CefRefPtr<CefResourceHandler> CreateRPCResourceHandler();

CefRefPtr<CefResourceHandler> NUISchemeHandlerFactory::Create(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& scheme_name, CefRefPtr<CefRequest> request)
{
	if (scheme_name == "nui")
	{
		return new NUIResourceHandler();
	}

	CefRefPtr<CefResourceHandler> outHandler;

	OnSchemeCreateRequest(scheme_name.ToString().c_str(), outHandler);

	return outHandler;
}

OVERLAY_DECL fwEvent<const char*, CefRefPtr<CefResourceHandler>&> OnSchemeCreateRequest;