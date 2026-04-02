/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */
#include "StdInc.h"

#include "NUIDevtools.h"
#include <CoreConsole.h>

CefRefPtr<NUIDevtoolsMessageObserver> g_devToolsObserver = new NUIDevtoolsMessageObserver();

bool NUIDevtoolsMessageObserver::OnDevToolsMessage(CefRefPtr<CefBrowser> browser, const void* message, size_t message_size)
{
	if (!m_isAllowed)
	{
		return true;
	}

	return false;
}

void NUIDevtoolsMessageObserver::OnDevToolsAgentAttached(CefRefPtr<CefBrowser> browser)
{
	if (!m_isAllowed)
	{
		browser->GetHost()->CloseDevTools();
	}
}

cef_return_value_t NUIDevtoolsClient::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefCallback> callback)
{
	const auto& url = request->GetURL().ToString();

	// Restrict devtools to only load resources from devtools
	if (url.find("chrome-devtools://") == 0 || url.find("devtools://") == 0)
	{
		return RV_CONTINUE;
	}
	return RV_CANCEL;
}

bool NUIDevtoolsClient::OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int popup_id, const CefString& target_url, const CefString& target_frame_name, CefLifeSpanHandler::WindowOpenDisposition target_disposition, bool user_gesture, const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, CefRefPtr<CefClient>& client, CefBrowserSettings& settings, CefRefPtr<CefDictionaryValue>& extra_info, bool* no_javascript_access)
{
	return true;
}

bool NUIDevtoolsClient::CanDownload(CefRefPtr<CefBrowser> browser, const CefString& url, const CefString& request_method)
{
	return true;
}

bool NUIDevtoolsClient::OnBeforeDownload(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, const CefString& suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback)
{
	return true;
}

bool NUIDevtoolsClient::OnFileDialog(CefRefPtr<CefBrowser> browser, FileDialogMode mode, const CefString& title, const CefString& default_file_path, const std::vector<CefString>& accept_filters, const std::vector<CefString>& accept_extensions, const std::vector<CefString>& accept_descriptions, CefRefPtr<CefFileDialogCallback> callback)
{
	callback->Cancel();
	return true;
}
