#include "StdInc.h"
#include "CrossLibraryInterfaces.h"

#include "GameInit.h"

#include <strsafe.h>

#include "NetLibrary.h"

#include <memory>

#include "CefOverlay.h"

#include <delayimp.h>
#include <include/cef_origin_whitelist.h>

#include <include/cef_sandbox_win.h>

#include <mutex>

bool g_mainUIFlag = true;

static int g_roundedWidth;
static int g_roundedHeight;

static HANDLE paintDoneEvent;

nui_s g_nui;

class NUIClient;

class NUIApp : public CefApp, public CefRenderProcessHandler, public CefResourceBundleHandler, public CefV8Handler
{
public:
	virtual void OnRegisterCustomSchemes(CefRefPtr<CefSchemeRegistrar> registrar)
	{
		registrar->AddCustomScheme("nui", true, false, false);
	}

	virtual bool GetDataResource(int resourceID, void*& data, size_t& data_size)
	{
		return false;
	}

	virtual bool GetLocalizedString(int messageID, CefString& string)
	{
		string = "";
		return true;
	}

	virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context)
	{
		CefRefPtr<CefV8Value> window = context->GetGlobal();

		window->SetValue("registerPollFunction", CefV8Value::CreateFunction("registerPollFunction", this), V8_PROPERTY_ATTRIBUTE_READONLY);
		window->SetValue("registerFrameFunction", CefV8Value::CreateFunction("registerFrameFunction", this), V8_PROPERTY_ATTRIBUTE_READONLY);
		window->SetValue("invokeNative", CefV8Value::CreateFunction("invokeNative", this), V8_PROPERTY_ATTRIBUTE_READONLY);
	}

	virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler()
	{
		return this;
	}

private:
	typedef std::map<int, std::pair<CefRefPtr<CefV8Context>, CefRefPtr<CefV8Value>>> TCallbackList;

	TCallbackList m_callbacks;
	TCallbackList m_frameCallbacks;

public:
	virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
	{
		if (message->GetName() == "doPoll")
		{
			auto it = m_callbacks.find(browser->GetIdentifier());

			if (it != m_callbacks.end())
			{
				auto context = it->second.first;
				auto callback = it->second.second;

				context->Enter();

				CefV8ValueList arguments;
				arguments.push_back(CefV8Value::CreateString(message->GetArgumentList()->GetString(0)));

				callback->ExecuteFunction(nullptr, arguments);

				context->Exit();

				return true;
			}
		}
		else if (message->GetName() == "createFrame" || message->GetName() == "destroyFrame")
		{
			auto it = m_frameCallbacks.find(browser->GetIdentifier());

			if (it != m_frameCallbacks.end())
			{
				auto context = it->second.first;
				auto callback = it->second.second;

				context->Enter();

				CefV8ValueList arguments;
				arguments.push_back(CefV8Value::CreateString(message->GetName()));
				arguments.push_back(CefV8Value::CreateString(message->GetArgumentList()->GetString(0)));

				if (message->GetName() == "createFrame")
				{
					arguments.push_back(CefV8Value::CreateString(message->GetArgumentList()->GetString(1)));
				}

				callback->ExecuteFunction(nullptr, arguments);

				context->Exit();

				return true;
			}
		}

		return false;
	}

	// CefV8Handler implementation
	virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception)
	{
		if (name == "registerPollFunction")
		{
			if (arguments.size() == 1 && arguments[0]->IsFunction())
			{
				auto context = CefV8Context::GetCurrentContext();

				m_callbacks[context->GetBrowser()->GetIdentifier()] = std::make_pair(context, arguments[0]);
			}

			retval = CefV8Value::CreateNull();

			return true;
		}
		else if (name == "registerFrameFunction")
		{
			if (arguments.size() == 1 && arguments[0]->IsFunction())
			{
				auto context = CefV8Context::GetCurrentContext();

				m_frameCallbacks[context->GetBrowser()->GetIdentifier()] = std::make_pair(context, arguments[0]);
			}

			retval = CefV8Value::CreateNull();

			return true;
		}
		else if (name == "invokeNative")
		{
			if (arguments.size() == 2)
			{
				auto msg = CefProcessMessage::Create("invokeNative");
				auto argList = msg->GetArgumentList();

				argList->SetSize(2);
				argList->SetString(0, arguments[0]->GetStringValue());
				argList->SetString(1, arguments[1]->GetStringValue());

				CefV8Context::GetCurrentContext()->GetBrowser()->SendProcessMessage(PID_BROWSER, msg);
			}

			retval = CefV8Value::CreateUndefined();

			return true;
		}

		return false;
	}

	IMPLEMENT_REFCOUNTING(NUIApp);
};

static NUIApp* g_app;

static CefBrowser* authUIBrowser;
HWND g_mainWindow;

void GSClient_Refresh();

#include <sstream>

class NUIClient : public CefClient, public CefLifeSpanHandler, public CefDisplayHandler, public CefContextMenuHandler, public CefLoadHandler, public CefRenderHandler
{
private:
	NUIWindow* m_window;
	bool m_windowValid;

	std::mutex m_windowLock;

	CefRefPtr<CefBrowser> m_browser;

public:
	NUIClient(NUIWindow* window)
		: m_window(window)
	{
		m_windowValid = true;
		_paintingPopup = false;
	}

	NUIWindow* GetWindow()
	{
		return m_window;
	}

	CefBrowser* GetBrowser()
	{
		return m_browser.get();
	}

	virtual void OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
	{
		GetWindow()->OnClientContextCreated(browser, frame, nullptr);
	}

	virtual void OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
	{
		auto url = frame->GetURL();

		if (url == "nui://game/ui/root.html" && g_mainUIFlag)
		{
			nui::CreateFrame("mpMenu", "nui://game/ui/mpmenu.html");
		}
	}

	virtual void OnAfterCreated(CefRefPtr<CefBrowser> browser)
	{
		m_browser = browser;

		if (!m_window->primary)
		{
			return;
		}

		if (browser.get())
		{
			authUIBrowser = browser.get();
			browser->AddRef();
		}
	}

	virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, CefProcessId source_process, CefRefPtr<CefProcessMessage> message)
	{
		if (message->GetName() == "invokeNative")
		{
			auto args = message->GetArgumentList();
			auto nativeType = args->GetString(0);

			if (nativeType == "refreshServers")
			{
				GSClient_Refresh();
			}
			else if (nativeType == "connectTo")
			{
				std::string hostnameStr = args->GetString(1);
				static char hostname[256];
				
				StringCbCopyA(hostname, sizeof(hostname), hostnameStr.c_str());

				char* port = strrchr(hostname, ':');

				if (!port)
				{
					port = "30120";
				}
				else
				{
					*port = '\0';
					port++;
				}

				g_netLibrary->ConnectToServer(hostname, atoi(port));
				g_mainUIFlag = false;
				nui::GiveFocus(false);

				nui::DestroyFrame("mpMenu");
			}
			else if (nativeType == "quit")
			{
				// TODO: CEF shutdown and native stuff related to it (set a shutdown flag)
				ExitProcess(0);
			}
			
			return true;
		}

		return false;
	}

	virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler()
	{
		return this;
	}

	virtual CefRefPtr<CefDisplayHandler> GetDisplayHandler()
	{
		return this;
	}

	virtual CefRefPtr<CefContextMenuHandler> GetContextMenuHandler()
	{
		return this;
	}

	virtual CefRefPtr<CefLoadHandler> GetLoadHandler()
	{
		return this;
	}

	virtual CefRefPtr<CefRenderHandler> GetRenderHandler()
	{
		return this;
	}

	virtual void OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model)
	{
		model->Clear();
	}

	virtual bool OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line)
	{
		std::wstring sourceStr = source.ToWString();
		std::wstring messageStr = message.ToWString();

		std::wstringstream msg;
		msg << sourceStr << ":" << line << ", " << messageStr << std::endl;

		OutputDebugStringW(msg.str().c_str());

		return false;
	}

	virtual bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
	{
		rect.Set(0, 0, m_window->width, m_window->height);

		return true;
	}

	bool _paintingPopup;
	CefRect _popupRect;

	virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height)
	{
		m_windowLock.lock();

		if (!m_windowValid)
		{
			m_windowLock.unlock();
			return;
		}

		if (type == PET_VIEW)
		{
			//EnterCriticalSection(&m_window->renderBufferLock);

			//int timeBegin = timeGetTime();

			//LeaveCriticalSection(&m_window->renderBufferLock);

			for (RectList::const_iterator iter = dirtyRects.begin(); iter != dirtyRects.end(); iter++)
			{
				CefRect rect = *iter;

				for (int y = rect.y; y < (rect.y + rect.height); y++)
				{
					int* src = &((int*)(buffer))[(y * width) + rect.x];
					int* dest = &((int*)(m_window->renderBuffer))[(y * m_window->roundedWidth) + rect.x];

					memcpy(dest, src, (rect.width * 4));
				}

				EnterCriticalSection(&m_window->renderBufferLock);
				m_window->dirtyRects.push(rect);
				LeaveCriticalSection(&m_window->renderBufferLock);
			}

			//int duration = timeGetTime() - timeBegin;
		}
		else if (type == PET_POPUP)
		{
			int skip_pixels = 0, x = _popupRect.x;
			int skip_rows = 0, yy = _popupRect.y;

			int w = width;
			int h = height;

			if (x < 0)
			{
				skip_pixels = -x;
				x = 0;
			}

			if (yy < 0)
			{
				skip_rows = -yy;
				yy = 0;
			}

			if ((x + w) > m_window->width)
			{
				w -= ((x + w) - m_window->width);
			}

			if ((yy + h) > m_window->height)
			{
				h -= ((yy + h) - m_window->height);
			}

			for (int y = yy; y < (yy + h); y++)
			{
				int* src = &((int*)(buffer))[((y - yy) * width)];
				int* dest = &((int*)(m_window->renderBuffer))[(y * m_window->roundedWidth) + x];

				memcpy(dest, src, (w * 4));
			}

			EnterCriticalSection(&m_window->renderBufferLock);
			m_window->dirtyRects.push(_popupRect);
			LeaveCriticalSection(&m_window->renderBufferLock);
		}

		if (!_paintingPopup)
		{
			if (!_popupRect.IsEmpty())
			{
				_paintingPopup = true;

				CefRect clientPopupRect(0, 0, _popupRect.width, _popupRect.height);

				browser->GetHost()->Invalidate(clientPopupRect, PET_POPUP);

				_paintingPopup = false;
			}

			m_window->renderBufferDirty = true;
			SetEvent(paintDoneEvent);
		}

		m_windowLock.unlock();
	}

	virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show)
	{
		if (!m_windowValid)
		{
			return;
		}

		if (!show)
		{
			CefRect rect = _popupRect;
			_popupRect.Set(0, 0, 0, 0);
			browser->GetHost()->Invalidate(rect, PET_VIEW);
		}
	}

	virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect)
	{
		if (rect.width <= 0 || rect.height <= 0)
		{
			return;
		}

		_popupRect = rect;

		if (_popupRect.x < 0)
		{
			_popupRect.x = 0;
		}

		if (_popupRect.y < 0)
		{
			_popupRect.y = 0;
		}

		if ((_popupRect.x + _popupRect.width) > m_window->width)
		{
			_popupRect.x = m_window->width - _popupRect.width;
		}

		if ((_popupRect.y + _popupRect.height) > m_window->height)
		{
			_popupRect.y = m_window->height - _popupRect.height;
		}

		if (_popupRect.x < 0)
		{
			_popupRect.x = 0;
		}

		if (_popupRect.y < 0)
		{
			_popupRect.y = 0;
		}
	}

	void SetWindowValid(bool valid)
	{
		m_windowValid = valid;
	}

	std::mutex& GetWindowLock()
	{
		return m_windowLock;
	}

	IMPLEMENT_REFCOUNTING(NUIClient);
};

CefBrowser* NUIWindow::GetBrowser()
{
	return ((NUIClient*)m_client.get())->GetBrowser();
}

bool RenderWindowMade();

LRESULT CALLBACK CefWndProc(HWND, UINT, WPARAM, LPARAM);

void NUIWindow::UpdateFrame()
{
	//WaitForSingleObject(paintDoneEvent, INFINITE);
	if (!nuiTexture)
	{
		return;
	}

	if (renderBufferDirty)
	{
		//int timeBegin = timeGetTime();

		D3DLOCKED_RECT lockedRect;
		nuiTexture->m_pITexture->LockRect(0, &lockedRect, NULL, 0);

		while (!dirtyRects.empty())
		{
			EnterCriticalSection(&renderBufferLock);
			CefRect rect = dirtyRects.front();
			dirtyRects.pop();
			LeaveCriticalSection(&renderBufferLock);

			for (int y = rect.y; y < (rect.y + rect.height); y++)
			{
				int* src = &((int*)(renderBuffer))[(y * roundedWidth) + rect.x];
				int* dest = &((int*)(lockedRect.pBits))[(y * (lockedRect.Pitch / 4)) + rect.x];

				memcpy(dest, src, (rect.width * 4));
			}
		}

		nuiTexture->m_pITexture->UnlockRect(0);

		//int duration = timeGetTime() - timeBegin;

		renderBufferDirty = false;
	}
}

static int roundUp(int number, int multiple)
{
	if ((number % multiple) == 0)
	{
		return number;
	}

	int added = number + multiple;

	return (added)-(added % multiple);
}

static std::vector<NUIWindow*> g_nuiWindows;
static std::mutex g_nuiWindowsMutex;

NUIWindow::NUIWindow(bool primary, int width, int height)
	: primary(primary), width(width), height(height), renderBuffer(nullptr), renderBufferDirty(false), m_onClientCreated(nullptr), nuiTexture(nullptr)
{
	g_nuiWindowsMutex.lock();
	g_nuiWindows.push_back(this);
	g_nuiWindowsMutex.unlock();
}

NUIWindow::~NUIWindow()
{
	auto nuiClient = ((NUIClient*)m_client.get());
	auto& mutex = nuiClient->GetWindowLock();

	mutex.lock();
	nuiClient->SetWindowValid(false);
	nuiClient->GetBrowser()->GetHost()->CloseBrowser(true);
	mutex.unlock();

	g_nuiWindowsMutex.lock();

	for (auto it = g_nuiWindows.begin(); it != g_nuiWindows.end(); )
	{
		if (*it == this)
		{
			it = g_nuiWindows.erase(it);
		}
		else
		{
			it++;
		}
	}

	g_nuiWindowsMutex.unlock();
}

void NUIWindow::SignalPoll(std::string& argument)
{
	NUIClient* client = static_cast<NUIClient*>(m_client.get());
	auto browser = client->GetBrowser();

	auto message = CefProcessMessage::Create("doPoll");
	auto argList = message->GetArgumentList();

	argList->SetSize(1);
	argList->SetString(0, argument);

	browser->SendProcessMessage(PID_RENDERER, message);
}

void NUIWindow::SetPaintType(NUIPaintType type)
{
	paintType = type;
}

void NUIWindow::Invalidate()
{
	CefRect fullRect(0, 0, width, height);
	((NUIClient*)m_client.get())->GetBrowser()->GetHost()->Invalidate(fullRect, PET_VIEW);
}

#include "RenderCallbacks.h"
#include "DrawCommands.h"

static InitFunction initFunction([] ()
{
	RegisterD3DPostResetCallback([] ()
	{
		g_nuiWindowsMutex.lock();

		for (auto& window : g_nuiWindows)
		{
			window->Invalidate();
		}

		g_nuiWindowsMutex.unlock();
	});

	RenderCallbacks::AddRenderCallback("endScene", [] ()
	{
		float bottomLeft[2] = { 0.f, 1.f };
		float bottomRight[2] = { 1.f, 1.f };
		float topLeft[2] = { 0.f, 0.f };
		float topRight[2] = { 1.f, 0.f };

		if (g_mainUIFlag)
		{
			//nui::ProcessInput();
		}

		uint32_t color = 0xFFFFFFFF;

		if (g_mainUIFlag)
		{
			ClearRenderTarget(true, -1, true, 1.0f, true, -1);
		}

		g_nuiWindowsMutex.lock();

		for (auto& window : g_nuiWindows)
		{
			if (window->GetPaintType() != NUIPaintTypePostRender)
			{
				continue;
			}

			window->UpdateFrame();

			if (window->GetTexture())
			{
				SetTextureGtaIm(window->GetTexture());

				int resX = *(int*)0xFDCEAC;
				int resY = *(int*)0xFDCEB0;

				// we need to subtract 0.5f from each vertex coordinate (half a pixel after scaling) due to the usual half-pixel/texel issue
				DrawImSprite(-0.5f, -0.5f, resX - 0.5f, resY - 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, &color, 0);
			}
		}

		g_nuiWindowsMutex.unlock();

		if (g_mainUIFlag)
		{
			POINT cursorPos;
			GetCursorPos(&cursorPos);

			//ScreenToClient(*(HWND*)0x1849DDC, &cursorPos);

			if (!GameInit::GetGameLoaded())
			{
				SetTextureGtaIm(*(rage::grcTexture**)(0x10C8310));
			}
			else
			{
				SetTextureGtaIm(*(rage::grcTexture**)(0x18AAC20));
			}

			DrawImSprite(cursorPos.x, cursorPos.y, cursorPos.x + 40.0f, cursorPos.y + 40.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, &color, 0);
		}
	});

	g_hooksDLL->SetHookCallback(StringHash("msgConfirm"), [] (void*)
	{
		//g_mainUIFlag = true;
		nui::GiveFocus(true);

		nui::CreateFrame("mpMenu", "nui://game/ui/mpmenu.html");
	});
});

std::shared_ptr<NUIWindow> NUIWindow::Create(bool primary, int width, int height, CefString url)
{
	auto window = std::make_shared<NUIWindow>(primary, width, height);
	window->Initialize(url);

	return window;
}

void NUIWindow::Initialize(CefString url)
{
	InitializeCriticalSection(&renderBufferLock);

	if (renderBuffer)
	{
		delete[] renderBuffer;
	}

	roundedHeight = roundUp(height, 16);
	roundedWidth = roundUp(width, 16);

	renderBuffer = new char[4 * roundedWidth * roundedHeight];

	nuiTexture = rage::grcTextureFactory::getInstance()->createManualTexture(width, height, FORMAT_A8R8G8B8, 0, nullptr);

	m_client = new NUIClient(this);

	CefWindowInfo info;
	info.SetAsWindowless(GetDesktopWindow(), true);

	CefBrowserSettings settings;
	settings.javascript_close_windows = STATE_DISABLED;

	CefRefPtr<CefRequestContext> rc = CefRequestContext::GetGlobalContext();
	CefBrowserHost::CreateBrowser(info, m_client, url, settings, rc);
}

static NUISchemeHandlerFactory* g_shFactory;

static std::shared_ptr<NUIWindow> g_nuiResourceRootWindow;

namespace nui
{
	CefBrowser* GetBrowser()
	{
		return g_nuiResourceRootWindow->GetBrowser();
	}

	void ExecuteRootScript(const char* scriptBit)
	{
		g_nuiResourceRootWindow->GetBrowser()->GetMainFrame()->ExecuteJavaScript(scriptBit, "internal", 1);
	}

	void ReloadNUI()
	{
		g_nuiResourceRootWindow->GetBrowser()->ReloadIgnoreCache();
	}

	void SetMainUI(bool enable)
	{
		g_mainUIFlag = enable;
		nui::GiveFocus(enable);
	}

	void CreateFrame(std::string frameName, std::string frameURL)
	{
		auto procMessage = CefProcessMessage::Create("createFrame");
		auto argumentList = procMessage->GetArgumentList();

		argumentList->SetSize(2);
		argumentList->SetString(0, frameName);
		argumentList->SetString(1, frameURL);

		auto browser = g_nuiResourceRootWindow->GetBrowser();
		browser->SendProcessMessage(PID_RENDERER, procMessage);
	}

	void DestroyFrame(std::string frameName)
	{
		auto procMessage = CefProcessMessage::Create("destroyFrame");
		auto argumentList = procMessage->GetArgumentList();

		argumentList->SetSize(1);
		argumentList->SetString(0, frameName);

		auto browser = g_nuiResourceRootWindow->GetBrowser();
		browser->SendProcessMessage(PID_RENDERER, procMessage);
	}

	void SignalPoll(std::string frameName)
	{
		g_nuiResourceRootWindow->SignalPoll(frameName);
	}

	NUISchemeHandlerFactory* GetSchemeHandlerFactory()
	{
		return g_shFactory;
	}

	bool OnPreLoadGame(void* cefSandbox)
	{
		// load the CEF library
		HMODULE libcef = LoadLibraryW(MakeRelativeCitPath(L"bin/libcef.dll").c_str());

		if (!libcef)
		{
			MessageBoxW(NULL, L"Could not load bin/libcef.dll.", L"CitizenFX", MB_ICONSTOP | MB_OK);

			return true;
		}

		__HrLoadAllImportsForDll("libcef.dll");

		// instantiate a NUIApp
		g_app = new NUIApp();

		CefMainArgs args(GetModuleHandle(NULL));
		CefRefPtr<CefApp> app(g_app);

		// try to execute as a CEF process
		int exitCode = CefExecuteProcess(args, app, cefSandbox);

		// and exit if we did
		if (exitCode >= 0)
		{
			return true;
		}

		// set up CEF as well here as we can do so anyway
		CefSettings cSettings;
		
		// TODO: change to GTA5 when released for PC
		cef_string_utf16_set(L"ros zc3Nzajw/OG58KC9/uG98L2uv6K+4bvw/Pw=", 40, &cSettings.user_agent, true);
		cSettings.multi_threaded_message_loop = true;
		cSettings.remote_debugging_port = 13172;
		cSettings.pack_loading_disabled = false; // true;
		cSettings.windowless_rendering_enabled = true;
		cef_string_utf16_set(L"en-US", 5, &cSettings.locale, true);

		std::wstring resPath = MakeRelativeCitPath(L"bin/cef/");
		cef_string_utf16_set(resPath.c_str(), resPath.length(), &cSettings.resources_dir_path, true);
		cef_string_utf16_set(resPath.c_str(), resPath.length(), &cSettings.locales_dir_path, true);

		g_shFactory = new NUISchemeHandlerFactory();

		// 2014-06-30: sandbox disabled as it breaks scheme handler factories (results in blank page being loaded)
		CefInitialize(args, cSettings, app.get(), /*cefSandbox*/ nullptr);
		CefRegisterSchemeHandlerFactory("nui", "", g_shFactory);
		//CefRegisterSchemeHandlerFactory("rpc", "", shFactory);
		CefAddCrossOriginWhitelistEntry("nui://game", "http", "", true);

		g_hooksDLL->SetHookCallback(StringHash("d3dCreate"), [] (void*)
		{
			int resX = *(int*)0xFDCEAC;
			int resY = *(int*)0xFDCEB0;

			g_nuiResourceRootWindow = NUIWindow::Create(true, resX, resY, "nui://game/ui/root.html");
			g_nuiResourceRootWindow->SetPaintType(NUIPaintTypePostRender);
		});

		return false;
	}
}
