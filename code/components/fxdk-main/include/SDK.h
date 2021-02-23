#pragma once

#include "StdInc.h"

#include <LauncherIPC.h>
#include <ICoreGameInit.h>

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"
#include "include/cef_app.h"
#include "include/cef_parser.h"
#include "include/wrapper/cef_closure_task.h"


namespace fxdk
{
	void InitRender();
	void Render();

	void ResizeRender(int w, int h);

	ipc::Endpoint& GetLauncherTalk();
}

namespace fxdk::ioUtils
{
	typedef std::function<void(const std::string& error)> RecycleShellItemsCallback;

	void RecycleShellItems(const std::vector<std::string> items, const RecycleShellItemsCallback cb);
}

class SDKInit : public ICoreGameInit
{
public:
	SDKInit()
	{
		SetVariable("networkInited");
	}

	virtual bool GetGameLoaded() override
	{
		return false;
	}
	virtual void KillNetwork(const wchar_t* errorString) override
	{
	}
	virtual bool TryDisconnect() override
	{
		return false;
	}
	virtual void SetPreventSavePointer(bool* preventSaveValue) override
	{
	}
	virtual void LoadGameFirstLaunch(bool (*callBeforeLoad)()) override
	{
	}
	virtual void ReloadGame() override
	{
	}
	virtual bool TriggerError(const char* errorString) override
	{
		return false;
	}
};

class SDKCefClient : public CefClient,
	public CefDisplayHandler,
	public CefLifeSpanHandler,
	public CefLoadHandler,
	public CefRequestHandler,
	public CefResourceRequestHandler
{
public:
	explicit SDKCefClient();
	~SDKCefClient();

	// Provide access to the single global instance of this object.
	static SDKCefClient* GetInstance();

	// CefClient methods:
	virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler() OVERRIDE
	{
		return this;
	}
	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() OVERRIDE
	{
		return this;
	}
	virtual CefRefPtr<CefRequestHandler> GetRequestHandler() OVERRIDE
	{
		return this;
	}
	virtual CefRefPtr<CefLoadHandler> GetLoadHandler() OVERRIDE
	{
		return this;
	}

	virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefProcessId source_process, CefRefPtr<CefProcessMessage> message) OVERRIDE;

	// CefDisplayHandler methods:
	virtual void OnTitleChange(CefRefPtr<CefBrowser> browser,
		const CefString& title) OVERRIDE;

	// CefLifeSpanHandler methods:
	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
	virtual bool DoClose(CefRefPtr<CefBrowser> browser) OVERRIDE;
	virtual void OnBeforeClose(CefRefPtr<CefBrowser> browser) OVERRIDE;

	// CefLoadHandler methods:
	virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode) OVERRIDE;

	virtual void OnLoadError(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefFrame> frame,
		ErrorCode errorCode,
		const CefString& errorText,
		const CefString& failedUrl) OVERRIDE;

	virtual ReturnValue OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefRequestCallback> callback) OVERRIDE;

	virtual CefRefPtr<CefResourceRequestHandler> GetResourceRequestHandler(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool is_navigation, bool is_download, const CefString& request_initiator, bool& disable_default_handling) OVERRIDE
	{
		return this;
	}

	// Request that all existing browser windows close.
	void CloseAllBrowsers(bool force_close);

	bool IsClosing() const
	{
		return is_closing_;
	}

	CefRefPtr<CefBrowser> GetBrowser();

private:
	// Platform-specific implementation.
	void PlatformTitleChange(CefRefPtr<CefBrowser> browser,
		const CefString& title);

	// List of existing browser windows. Only accessed on the CEF UI thread.
	typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
	BrowserList browser_list_;

	bool is_closing_;

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(SDKCefClient);
};

class SDKCefApp : public CefApp, public CefBrowserProcessHandler/*, public CefRenderProcessHandler*/
{
public:
	typedef std::function<CefRefPtr<CefV8Value>(const CefV8ValueList&, CefString&)> TV8Handler;

	SDKCefApp();

	// CefApp methods:
	virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler()
		OVERRIDE
	{
		return this;
	}

	virtual void OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line) OVERRIDE;

	// CefBrowserProcessHandler methods:
	virtual void OnContextInitialized() OVERRIDE;

	// CefRenderProcessHandler methods:
	//virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) OVERRIDE;

private:
	int m_v8Callbacks;

	// Include the default reference counting implementation.
	IMPLEMENT_REFCOUNTING(SDKCefApp);
};

class SimpleWindowDelegate : public CefWindowDelegate
{
public:
	explicit SimpleWindowDelegate(CefRefPtr<CefBrowserView> browser_view);

	void OnWindowCreated(CefRefPtr<CefWindow> window) OVERRIDE;
	void OnWindowDestroyed(CefRefPtr<CefWindow> window) OVERRIDE;

	bool CanClose(CefRefPtr<CefWindow> window) OVERRIDE;

	CefSize GetMinimumSize(CefRefPtr<CefView> view) OVERRIDE;

private:
	void LoadPlacement(CefRefPtr<CefWindow> window);
	void SavePlacement(CefRefPtr<CefWindow> window);

private:
	CefRefPtr<CefBrowserView> browser_view_;

	IMPLEMENT_REFCOUNTING(SimpleWindowDelegate);
	DISALLOW_COPY_AND_ASSIGN(SimpleWindowDelegate);
};

class SubViewDelegate : public CefBrowserViewDelegate
{
public:
	SubViewDelegate();

	virtual CefRefPtr<CefBrowserViewDelegate> GetDelegateForPopupBrowserView(CefRefPtr<CefBrowserView> browser_view, const CefBrowserSettings& settings, CefRefPtr<CefClient> client, bool is_devtools);

	virtual bool OnPopupBrowserViewCreated(CefRefPtr<CefBrowserView> browser_view, CefRefPtr<CefBrowserView> popup_browser_view, bool is_devtools);

private:
	IMPLEMENT_REFCOUNTING(SubViewDelegate);
	DISALLOW_COPY_AND_ASSIGN(SubViewDelegate);
};
