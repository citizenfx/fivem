#include "StdInc.h"

#include "CefOverlay.h"

#include <delayimp.h>
#include <include/cef_origin_whitelist.h>

#include <include/cef_sandbox_win.h>

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
	}

	virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler()
	{
		return this;
	}

private:
	std::map<int, std::pair<CefRefPtr<CefV8Context>, CefRefPtr<CefV8Value>>> m_callbacks;

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

				m_callbacks[context->GetBrowser()->GetIdentifier()] = std::make_pair(context, arguments[1]);
			}

			retval = CefV8Value::CreateNull();

			return true;
		}

		return false;
	}

	IMPLEMENT_REFCOUNTING(NUIApp);
};

static NUIApp* g_app;

static CefBrowser* authUIBrowser;
HWND g_mainWindow;

#include <sstream>

class NUIClient : public CefClient, public CefLifeSpanHandler, public CefDisplayHandler, public CefContextMenuHandler, public CefLoadHandler, public CefRenderHandler
{
private:
	NUIWindow* m_window;

	CefRefPtr<CefBrowser> m_browser;

public:
	NUIClient(NUIWindow* window)
		: m_window(window)
	{
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

				m_window->dirtyRects.push(rect);
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

			m_window->dirtyRects.push(_popupRect);
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
	}

	virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show)
	{
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

	if (renderBufferDirty)
	{
		//int timeBegin = timeGetTime();

		D3DLOCKED_RECT lockedRect;
		//nuiTexture->m_pITexture->LockRect(0, &lockedRect, NULL, 0);

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

		//nuiTexture->m_pITexture->UnlockRect(0);

		//int duration = timeGetTime() - timeBegin;

		renderBufferDirty = false;
	}
}

/*static DWORD origRenderFrontEnd;

class CDrawSpriteDC
{
private:
	char blah[48];

public:
	void ctor(float* v1, float* v2, float* v3, float* v4, DWORD color, grcTexture* texture);
};

void __declspec(naked) CDrawSpriteDC::ctor(float* v1, float* v2, float* v3, float* v4, DWORD color, grcTexture* texture)
{
	__asm
	{
		mov eax, 7C0F00h
			jmp eax
	}
}

class CGenericDCNoArgs
{
private:
	char blah[12];

public:
	void ctor(void(*func)());
};

void WRAPPER CGenericDCNoArgs::ctor(void(*)()) { EAXJMP(0x44CE70); }

// NOTE: NOT THE REAL CONSTRUCTOR
void WRAPPER CGenericDCMatrixArg::ctor(void(*)(float*)) { EAXJMP(0x44CE70); }

static NUIWindow* mainWindow;

grcTexture* GetMainNUITexture()
{
	return mainWindow->nuiTexture;
}

void DrawJobs(float*);

extern NUIWindow* g_jobWindow;
extern NUIWindow* rosWindow;

static Viewport*& vp = *(Viewport**)0x10F47F0;

void AddJobDC()
{
	CGenericDCMatrixArg* jobsDC = (CGenericDCMatrixArg*)gta_alloc(sizeof(CGenericDCMatrixArg), 0);
	jobsDC->ctor(DrawJobs);
	jobsDC->vtable = 0xDB9254; // T_CB_Generic_1Arg<void (__cdecl *)(class rage::Matrix44 &), class rage::Matrix44>
	memcpy(jobsDC->matrix, vp->matrix4, sizeof(vp->matrix4));

	PlaceDCInQueue(jobsDC);
}

void BlurBackBuffer();

extern bool g_isFocused;

void RenderPhaseFrontEndWrap()
{
	if (*(BYTE*)0x10C7F6F)
	{
		((void(*)())origRenderFrontEnd)();
	}

	CGenericDCNoArgs* jobsDC = (CGenericDCNoArgs*)gta_alloc(sizeof(CGenericDCNoArgs), 0);
	jobsDC->ctor(BlurBackBuffer);
	PlaceDCInQueue(jobsDC);

	mainWindow->UpdateFrame();
	g_jobWindow->UpdateFrame();

	if (rosWindow)
	{
		rosWindow->UpdateFrame();
	}

	int resY = *(int*)0xFDCEB0;

	float bottomLeft[2] = { 0.f, 1.f };
	float bottomRight[2] = { 1.f, 1.f };
	float topLeft[2] = { 0.f, 0.f };
	float topRight[2] = { 1.f, 0.f };

	CDrawSpriteDC* drawSpriteDC = (CDrawSpriteDC*)gta_alloc(sizeof(CDrawSpriteDC), 0);
	drawSpriteDC->ctor(bottomLeft, topLeft, bottomRight, topRight, 0xFFFFFFFF, mainWindow->nuiTexture);

	PlaceDCInQueue(drawSpriteDC);

	if (rosWindow)
	{
		bottomLeft[1] -= (1.0f / resY);
		bottomRight[1] -= (1.0f / resY);

		topLeft[1] -= (1.0f / resY);
		topRight[1] -= (1.0f / resY);

		drawSpriteDC = (CDrawSpriteDC*)gta_alloc(sizeof(CDrawSpriteDC), 0);
		drawSpriteDC->ctor(bottomLeft, topLeft, bottomRight, topRight, 0xFFFFFFFF, rosWindow->nuiTexture);

		PlaceDCInQueue(drawSpriteDC);
	}

	// stuff :(
	if (rosWindow && g_isFocused)
	{
		static int lastX, lastY;
		POINT point;

		GetCursorPos(&point);

		RECT rect;
		GetWindowRect(*(HWND*)0x1849DDC, &rect);

		int x = point.x - rect.left;
		int y = point.y - rect.top;

		if (x != lastX || y != lastY)
		{
			CefMouseEvent mouseEvent;
			mouseEvent.x = x;
			mouseEvent.y = y;

			rosWindow->GetBrowser()->GetHost()->SendMouseMoveEvent(mouseEvent, false);

			lastX = x;
			lastY = y;
		}

		static bool lastLeft, lastRight;

		bool left = GetAsyncKeyState(VK_LBUTTON);
		bool right = GetAsyncKeyState(VK_RBUTTON);

		if (left != lastLeft)
		{
			CefMouseEvent mouseEvent;
			mouseEvent.x = x;
			mouseEvent.y = y;

			rosWindow->GetBrowser()->GetHost()->SendFocusEvent(true);
			rosWindow->GetBrowser()->GetHost()->SendMouseClickEvent(mouseEvent, MBT_LEFT, !left, 1);

			lastLeft = left;
		}

		if (right != lastRight)
		{
			CefMouseEvent mouseEvent;
			mouseEvent.x = x;
			mouseEvent.y = y;

			rosWindow->GetBrowser()->GetHost()->SendMouseClickEvent(mouseEvent, MBT_RIGHT, !right, 1);

			lastRight = right;
		}
	}
}
*/

/*InitFunction nuiInit([] ()
{
	origRenderFrontEnd = *(DWORD*)0xE9F1AC;
	*(DWORD*)0xE9F1AC = (DWORD)RenderPhaseFrontEndWrap;

	// in-menu check for renderphasefrontend
	*(BYTE*)0x43AF21 = 0xEB;

	// load actual CEF module in our own base
	wchar_t exeName[512];
	GetModuleFileNameW(GetModuleHandle(NULL), exeName, sizeof(exeName) / 2);

	wchar_t* exeBaseName = wcsrchr(exeName, L'\\');
	exeBaseName[0] = L'\0';
	exeBaseName++;

	static wchar_t path[32768];
	static wchar_t newPath[32768];
	GetEnvironmentVariableW(L"PATH", path, sizeof(path));

	// step 1: stuff
	LoadLibraryW(L"gdipp_client_32.dll");

	// step 2: loadlibrary
	_snwprintf(newPath, sizeof(newPath), L"%s\\plaza\\libcef.dll", exeName);

	HMODULE libcef = LoadLibraryW(newPath);

	if (!libcef)
	{
		MessageBoxW(NULL, L"Could not load libcef.dll from LibNP.", L"Terminal/NPx", MB_ICONSTOP | MB_OK);

		return;
	}

	__HrLoadAllImportsForDll("libcef.dll");
});*/

static int roundUp(int number, int multiple)
{
	if ((number % multiple) == 0)
	{
		return number;
	}

	int added = number + multiple;

	return (added)-(added % multiple);
}

NUIWindow::NUIWindow(bool primary, int width, int height)
	: primary(primary), width(width), height(height), renderBuffer(nullptr), renderBufferDirty(false), m_onClientCreated(nullptr)
{

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

	/*grcTextureFactory* textureFactory = *(grcTextureFactory**)0x18A8630;
	nuiTexture = textureFactory->createManualTexture(width, height, FORMAT_A8R8G8B8, 0, nullptr);

	RegisterD3DResetCB(false, [&] ()
	{
		grcTextureFactory* textureFactory = *(grcTextureFactory**)0x18A8630;

		CefRect fullRect(0, 0, width, height);
		((NUIClient*)m_client.get())->GetBrowser()->GetHost()->Invalidate(fullRect, PET_VIEW);
	});*/

	m_client = new NUIClient(this);

	CefWindowInfo info;
	info.SetAsWindowless(GetDesktopWindow(), true);

	CefBrowserSettings settings;
	settings.javascript_close_windows = STATE_DISABLED;

	CefRefPtr<CefRequestContext> rc = CefRequestContext::GetGlobalContext();
	CefBrowserHost::CreateBrowser(info, m_client, url, settings, rc);
}

void JobNUIInit();

void NUI_Init()
{
	/*int resX = *(int*)0xFDCEAC;
	int resY = *(int*)0xFDCEB0;

	g_nui.nuiWidth = resX;
	g_nui.nuiHeight = resY;

	paintDoneEvent = CreateEvent(0, FALSE, TRUE, NULL);

	g_app = new NUIApp();

	CefMainArgs args(GetModuleHandle(NULL));
	CefRefPtr<CefApp> app(g_app);

	CefSettings cSettings;
	cef_string_utf16_set(L"ros zc3Nzajw/OG58KC9/uG98L2uv6K+4bvw/Pw=", 40, &cSettings.user_agent, true);
	cSettings.multi_threaded_message_loop = true;
	cSettings.single_process = true;
	cSettings.remote_debugging_port = 13172;
	cSettings.pack_loading_disabled = false; // true;
	cef_string_utf16_set(L"en-US", 5, &cSettings.locale, true);

	CefInitialize(args, cSettings, app.get(), nullptr);
	CefRegisterSchemeHandlerFactory("nui", "game", new NUISchemeHandlerFactory());
	CefAddCrossOriginWhitelistEntry("nui://game", "http", "", true);

	NUIWindow* window = new NUIWindow(true, resX, resY);
	window->Initialize("nui://game/index.html");
	//window->Initialize("nui://game/dot.html");

	mainWindow = window;

	JobNUIInit();*/
}

namespace nui
{
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
		cef_string_utf16_set(L"en-US", 5, &cSettings.locale, true);

		std::wstring resPath = MakeRelativeCitPath(L"bin/cef/");
		cef_string_utf16_set(resPath.c_str(), resPath.length(), &cSettings.resources_dir_path, true);
		cef_string_utf16_set(resPath.c_str(), resPath.length(), &cSettings.locales_dir_path, true);

		CefInitialize(args, cSettings, app.get(), nullptr);
		CefRegisterSchemeHandlerFactory("nui", "", new NUISchemeHandlerFactory());
		CefAddCrossOriginWhitelistEntry("nui://game", "http", "", true);

		return false;
	}
}

extern NUIWindow* g_jobWindow;

void ReloadNUI()
{
	//authUIBrowser->ReloadIgnoreCache();

	//g_jobWindow->GetBrowser()->ReloadIgnoreCache();
}
