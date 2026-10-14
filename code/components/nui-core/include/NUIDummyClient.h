/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */
#pragma once

#include <include/cef_client.h>
#include <include/cef_command_handler.h>

/// 
/// With the Chrome runtime (M128+), if we do not give cef a default client
/// It will use a default CefClient which can enable behaviours we don't want
/// as a result, this dummy client is the default client and is no-op.
/// 
class NUIDummyClient : public CefClient, 
	public CefCommandHandler,
	public CefLifeSpanHandler,
	public CefRequestHandler
{
public:
	virtual CefRefPtr<CefCommandHandler> GetCommandHandler() override
	{
		return this;
	}

	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override
	{
		return this;
	}

	virtual CefRefPtr<CefRequestHandler> GetRequestHandler() override
	{
		return this;
	}

	// CefCommandHandler
	virtual bool OnChromeCommand(CefRefPtr<CefBrowser> browser, int command_id, cef_window_open_disposition_t disposition) override
	{
		return true;
	}

	virtual bool IsChromeAppMenuItemVisible(CefRefPtr<CefBrowser> browser, int command_id) override
	{
		return false;
	}

	virtual bool IsChromeToolbarButtonVisible(cef_chrome_toolbar_button_type_t button_type) override
	{
		return false;
	}

	virtual bool IsChromePageActionIconVisible(cef_chrome_page_action_icon_type_t icon_type) override
	{
		return false;
	}

	virtual bool IsChromeAppMenuItemEnabled(CefRefPtr<CefBrowser> browser, int command_id) override
	{
		return false;
	}
	//

	// CefLifeSpanHandler 
    virtual bool OnBeforePopup(CefRefPtr<CefBrowser> browser,
                             CefRefPtr<CefFrame> frame,
                             int popup_id,
                             const CefString& target_url,
                             const CefString& target_frame_name,
                             CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                             bool user_gesture,
                             const CefPopupFeatures& popupFeatures,
                             CefWindowInfo& windowInfo,
                             CefRefPtr<CefClient>& client,
                             CefBrowserSettings& settings,
                             CefRefPtr<CefDictionaryValue>& extra_info,
                             bool* no_javascript_access) 
	{
		return true;
	}

	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) override
	{
		if (browser && browser->GetHost())
		{
			browser->GetHost()->CloseBrowser(true);
		}
	}
	//

	// CefRequestHandler
	virtual bool OnOpenURLFromTab(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
		const CefString& target_url,
		CefRequestHandler::WindowOpenDisposition target_disposition,
		bool user_gesture) override
	{
		return true;
	}

	virtual bool OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
		CefRefPtr<CefRequest> request, bool user_gesture, bool is_redirect) override
	{
		return true;
	}
	//

	IMPLEMENT_REFCOUNTING(NUIDummyClient);
};

