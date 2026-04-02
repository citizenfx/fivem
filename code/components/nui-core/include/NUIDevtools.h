/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */
#pragma once


#include <CefOverlay.h>
#include <include/cef_app.h>
#include <include/cef_browser.h>

class NUIDevtoolsMessageObserver : public CefDevToolsMessageObserver 
{
public:
  virtual bool OnDevToolsMessage(CefRefPtr<CefBrowser> browser, const void* message, size_t message_size) override;

  virtual void OnDevToolsAgentAttached(CefRefPtr<CefBrowser> browser) override;

  void SetAllowed(bool allowed)
  {
	  m_isAllowed = allowed;
  }

  bool IsAllowed() const
  {
	  return m_isAllowed;
  }

private:
    std::atomic<bool> m_isAllowed = true;
	IMPLEMENT_REFCOUNTING(NUIDevtoolsMessageObserver);
};

extern CefRefPtr<NUIDevtoolsMessageObserver> g_devToolsObserver;

class NUIDevtoolsClient : public CefClient,
	public CefLifeSpanHandler,
	public CefRequestHandler,
	public CefResourceRequestHandler,
	public CefDialogHandler,
	public CefDownloadHandler
{
// Handlers
protected:
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override
	{
		return this;
	}

	virtual CefRefPtr<CefDialogHandler> GetDialogHandler() override
	{
		return this;
	}

	virtual CefRefPtr<CefDownloadHandler> GetDownloadHandler() override
	{
		return this;
	}
	
	virtual CefRefPtr<CefRequestHandler> GetRequestHandler() override
	{
		return this;
	}

// CefRequestHandler
protected:
    CefRefPtr<CefResourceRequestHandler> GetResourceRequestHandler(
		CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefRequest> request,
		bool is_navigation,
		bool is_download,
		const CefString& request_initiator,
		bool& disable_default_handling) override
	{
		return this;
	}

	cef_return_value_t OnBeforeResourceLoad(
		CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		CefRefPtr<CefRequest> request,
		CefRefPtr<CefCallback> callback) override;
// CefLifeSpanHandler
protected:
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
		bool* no_javascript_access) override;

// CefDownloadHandler
protected:
	bool CanDownload(CefRefPtr<CefBrowser> browser,
		const CefString& url,
		const CefString& request_method) override;

	bool OnBeforeDownload(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefDownloadItem> download_item,
		const CefString& suggested_name,
		CefRefPtr<CefBeforeDownloadCallback> callback) override;

// CefDialogHandler
protected:
	bool OnFileDialog(CefRefPtr<CefBrowser> browser,
		FileDialogMode mode,
		const CefString& title,
		const CefString& default_file_path,
		const std::vector<CefString>& accept_filters,
		const std::vector<CefString>& accept_extensions,
		const std::vector<CefString>& accept_descriptions,
		CefRefPtr<CefFileDialogCallback> callback) override;

private:
	IMPLEMENT_REFCOUNTING(NUIDevtoolsClient);

};
