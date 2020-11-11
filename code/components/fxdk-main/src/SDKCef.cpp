#include <SDK.h>


namespace {
	SDKCefClient* g_instance = NULL;
}


#pragma region SDKCefApp
SDKCefApp::SDKCefApp()
{
}

void SDKCefApp::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line)
{
	command_line->AppendSwitch("disable-extensions");
	command_line->AppendSwitch("disable-pdf-extension");

	command_line->AppendSwitchWithValue("use-angle", "d3d11");
}

void SDKCefApp::OnContextInitialized()
{
	CEF_REQUIRE_UI_THREAD();
}
#pragma endregion


#pragma region SDKCefClient
SDKCefClient::SDKCefClient()
	: is_closing_(false)
{
	DCHECK(!g_instance);
	g_instance = this;
}

SDKCefClient::~SDKCefClient()
{
	g_instance = NULL;
}

// static
SDKCefClient* SDKCefClient::GetInstance()
{
	return g_instance;
}

void SDKCefClient::OnTitleChange(CefRefPtr<CefBrowser> browser,
	const CefString& title)
{
	CEF_REQUIRE_UI_THREAD();

	// Set the title of the window using the Views framework.
	CefRefPtr<CefBrowserView> browser_view = CefBrowserView::GetForBrowser(browser);
	if (browser_view)
	{
		CefRefPtr<CefWindow> window = browser_view->GetWindow();
		if (window)
			window->SetTitle(title);
	}
}

void SDKCefClient::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
	CEF_REQUIRE_UI_THREAD();

	// Add to the list of existing browsers.
	browser_list_.push_back(browser);
}

CefRefPtr<CefBrowser> SDKCefClient::GetBrowser()
{
	return browser_list_.front();
}

bool SDKCefClient::DoClose(CefRefPtr<CefBrowser> browser)
{
	CEF_REQUIRE_UI_THREAD();

	// Closing the main window requires special handling. See the DoClose()
	// documentation in the CEF header for a detailed description of this
	// process.
	if (browser_list_.size() == 1)
	{
		// Set a flag to indicate that the window close should be allowed.
		is_closing_ = true;
	}

	// Allow the close. For windowed browsers this will result in the OS close
	// event being sent.
	return false;
}

void SDKCefClient::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
	CEF_REQUIRE_UI_THREAD();

	// Remove from the list of existing browsers.
	BrowserList::iterator bit = browser_list_.begin();
	for (; bit != browser_list_.end(); ++bit)
	{
		if ((*bit)->IsSame(browser))
		{
			browser_list_.erase(bit);
			break;
		}
	}

	if (browser_list_.empty())
	{
		// All browser windows have closed. Quit the application message loop.
		CefQuitMessageLoop();
	}
}

void SDKCefClient::OnLoadError(CefRefPtr<CefBrowser> browser,
	CefRefPtr<CefFrame> frame,
	ErrorCode errorCode,
	const CefString& errorText,
	const CefString& failedUrl)
{
	CEF_REQUIRE_UI_THREAD();

	// Don't display an error for downloaded files.
	if (errorCode == ERR_ABORTED)
		return;
}

void SDKCefClient::CloseAllBrowsers(bool force_close)
{
	if (!CefCurrentlyOn(TID_UI))
	{
		// Execute on the UI thread.
		CefPostTask(TID_UI, base::Bind(&SDKCefClient::CloseAllBrowsers, this,
			force_close));
		return;
	}

	if (browser_list_.empty())
		return;

	BrowserList::const_iterator it = browser_list_.begin();
	for (; it != browser_list_.end(); ++it)
		(*it)->GetHost()->CloseBrowser(force_close);

	CefQuitMessageLoop();
}

auto SDKCefClient::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, CefRefPtr<CefRequestCallback> callback) -> ReturnValue
{
	return RV_CONTINUE;
}

bool SDKCefClient::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
{
	auto messageName = message->GetName();

	if (messageName == "invokeNative")
	{
		auto args = message->GetArgumentList();
		auto nativeType = args->GetString(0);
		auto messageData = args->GetString(1);

		fxdk::GetLauncherTalk().Call("sdk:invokeNative", std::string{ nativeType }, std::string{ messageData });
	}
	else if (messageName == "resizeGame")
	{
		auto args = message->GetArgumentList();
		auto width = args->GetInt(0);
		auto height = args->GetInt(1);

		fxdk::ResizeRender(width, height);
	}
	else if (messageName == "openDevTools")
	{
		CefWindowInfo wi;
		wi.SetAsPopup(NULL, "FxDK DevTools");

		CefBrowserSettings s;

		browser->GetHost()->ShowDevTools(wi, SDKCefClient::GetInstance(), s, {});
	}

	return true;
}

void SDKCefClient::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
}

#pragma endregion

