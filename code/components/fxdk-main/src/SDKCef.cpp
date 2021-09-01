#include <SDK.h>

#include <shellapi.h>
#include <ShlObj_core.h>

#include <skyr/url.hpp>

#include <ResourceManager.h>
#include <ResourceEventComponent.h>


namespace {
	SDKCefClient* g_instance = NULL;
	HWND g_mainWindowHandle;
}


void BrowseToFile(LPCTSTR filename)
{
	ITEMIDLIST* pidl = ILCreateFromPath(filename);
	if (pidl) {
		SHOpenFolderAndSelectItems(pidl, 0, 0, 0);
		ILFree(pidl);
	}
}

void FixPath(wchar_t* outPath, const wchar_t* inPath)
{
	for (const wchar_t* c = inPath; *c && outPath - inPath < MAX_PATH - 1; ++c, ++outPath)
	{
		*outPath = *c == '/' ? '\\' : *c;
	}

	*outPath = 0;
}

template <class T>
void SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}

const std::string FxdkSelectFolder(const std::string& startPath, const std::string& title, bool onlyFolders, HWND parentWindow = NULL)
{
	IFileDialog* fileDialog;

	if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&fileDialog))))
	{
		SafeRelease(&fileDialog);
		return std::string{ 0 };
	}

	WCHAR wstartPath[MAX_PATH];
	FixPath(wstartPath, ToWide(startPath).c_str());

	// Set start folder
	{
		PIDLIST_ABSOLUTE pidl;
		if (FAILED(::SHParseDisplayName(wstartPath, 0, &pidl, SFGAO_FOLDER, 0)))
		{
			SafeRelease(&fileDialog);
			ILFree(pidl);
			return std::string{};
		}

		IShellItem* psi;
		if (FAILED(::SHCreateShellItem(NULL, NULL, pidl, &psi)))
		{
			SafeRelease(&fileDialog);
			SafeRelease(&psi);
			ILFree(pidl);
			return std::string{};
		}

		fileDialog->SetFolder(psi);

		ILFree(pidl);
	}
	
	fileDialog->SetTitle(ToWide(title).c_str());

	DWORD options;
	if (FAILED(fileDialog->GetOptions(&options)))
	{
		SafeRelease(&fileDialog);
		return std::string{};
	}

	if (onlyFolders)
	{
		options |= FOS_PICKFOLDERS;
	}

	fileDialog->SetOptions(options);

	HRESULT showResult = fileDialog->Show(parentWindow);
	if (FAILED(showResult) || showResult == HRESULT_FROM_WIN32(ERROR_CANCELLED))
	{
		SafeRelease(&fileDialog);
		return std::string{};
	}

	IShellItem* resultItem;
	if (FAILED(fileDialog->GetResult(&resultItem)))
	{
		SafeRelease(&fileDialog);
		SafeRelease(&resultItem);
		return std::string{};
	}

	LPWSTR pwszFilePath = NULL;
	if (FAILED(resultItem->GetDisplayName(SIGDN_FILESYSPATH, &pwszFilePath)) || pwszFilePath == NULL)
	{
		SafeRelease(&fileDialog);
		SafeRelease(&resultItem);
		CoTaskMemFree(pwszFilePath);
		return std::string{};
	}

	SafeRelease(&fileDialog);
	SafeRelease(&resultItem);

	const std::string selectedFolder = ToNarrow(pwszFilePath).c_str();

	CoTaskMemFree(pwszFilePath);

	return selectedFolder;
}

#pragma region SDKCefApp
SDKCefApp::SDKCefApp()
{
}

void SDKCefApp::OnBeforeCommandLineProcessing(const CefString& process_type, CefRefPtr<CefCommandLine> command_line)
{
	auto sdk_url = ConsoleVariableManager::GetDefaultInstance()->FindEntryRaw("sdk_url");

	auto parsedSdkUrl = skyr::make_url(sdk_url->GetValue());

	if (parsedSdkUrl && !parsedSdkUrl->origin().empty())
	{
		// Allow secure context for insecure localhost
		command_line->AppendSwitchWithValue("unsafely-treat-insecure-origin-as-secure", parsedSdkUrl->origin());
	}

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
void SDKCefClient::SetMainWindowHandle(HWND handle)
{
	g_mainWindowHandle = handle;
}
HWND SDKCefClient::GetMainWindowHandle()
{
	return g_mainWindowHandle;
}

void SDKCefClient::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
	CEF_REQUIRE_UI_THREAD();

	// Set the title of the window using the Views framework.
	CefRefPtr<CefBrowserView> browser_view = CefBrowserView::GetForBrowser(browser);
	if (browser_view)
	{
		CefRefPtr<CefWindow> window = browser_view->GetWindow();
		if (window && window->GetWindowHandle() == GetMainWindowHandle())
		{
			window->SetTitle(title);
		}
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
	if (browser_list_.size() == 0)
	{
		return nullptr;
	}

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
		std::string nativeType = args->GetString(0);
		std::string messageData = args->GetString(1);

		if (nativeType == "openUrl")
		{
			ShellExecute(nullptr, L"open", ToWide(messageData).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
		}
		else if (nativeType == "openFolderAndSelectFile")
		{
			BrowseToFile(ToWide(messageData).c_str());
		}
		else
		{
			fxdk::GetLauncherTalk().Call("sdk:invokeNative", nativeType, messageData);
		}
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

		wi.SetAsPopup(GetMainWindowHandle(), "fxdk:devtools");

		CefBrowserSettings s;

		browser->GetHost()->ShowDevTools(wi, SDKCefClient::GetInstance(), s, {});
	}
	else if (messageName == "fxdkSendApiMessage")
	{
		std::string apiMsg = message->GetArgumentList()->GetString(0);

		fx::ResourceManager::GetCurrent()->GetComponent<fx::ResourceEventManagerComponent>()->QueueEvent2("sdk:api:recv", {}, apiMsg);
	}
	else if (messageName == "fxdkOpenSelectFolderDialog")
	{
		std::string defaultPath = message->GetArgumentList()->GetString(0);
		std::string title = message->GetArgumentList()->GetString(1);
		bool onlyFolders = message->GetArgumentList()->GetBool(2);

		std::thread([defaultPath, title, onlyFolders, browser, hwnd = g_mainWindowHandle]()
		{
			std::string path = FxdkSelectFolder(defaultPath, title, onlyFolders, hwnd);

			auto cefMsg = CefProcessMessage::Create("fxdkOpenSelectFolderDialogResult");
			auto cefMsgArgs = cefMsg->GetArgumentList();

			cefMsgArgs->SetSize(1);
			cefMsgArgs->SetString(0, path);

			browser->GetMainFrame()->SendProcessMessage(PID_RENDERER, cefMsg);
		})
		.detach();
	}
	else if (messageName == "sendGameClientEvent")
	{
		std::string eventName = message->GetArgumentList()->GetString(0);
		std::string eventPayload = message->GetArgumentList()->GetString(1);

		fxdk::GetLauncherTalk().Call("sdk:clientEvent", eventName, eventPayload);
	}

	return true;
}

void SDKCefClient::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
}

#pragma endregion

