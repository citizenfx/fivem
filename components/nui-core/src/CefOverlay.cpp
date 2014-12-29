/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "CrossLibraryInterfaces.h"
#include "DrawCommands.h"
#include <GfxUtil.h>

//#include "GameInit.h"

#include <strsafe.h>

//#include "NetLibrary.h"

#include <memory>

#include "CefOverlay.h"
#include "NUIApp.h"
#include "NUIClient.h"

#include <delayimp.h>
#include <include/cef_origin_whitelist.h>

#include <include/cef_sandbox_win.h>

#include <mutex>

bool g_mainUIFlag = true;

extern POINT g_cursorPos;

nui_s g_nui;

HWND g_mainWindow;

fwEvent<const wchar_t*, const wchar_t*> nui::OnInvokeNative;

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

	for (auto& item : pollQueue)
	{
		NUIClient* client = static_cast<NUIClient*>(m_client.get());
		auto browser = client->GetBrowser();

		auto message = CefProcessMessage::Create("doPoll");
		auto argList = message->GetArgumentList();

		argList->SetSize(1);
		argList->SetString(0, item);

		browser->SendProcessMessage(PID_RENDERER, message);
	}

	pollQueue.clear();

	if (renderBufferDirty)
	{
		//int timeBegin = timeGetTime();

		void* pBits = nullptr;
		int pitch;
		bool discarded = false;

#ifndef _HAS_GRCTEXTURE_MAP
		D3DLOCKED_RECT lockedRect;
		nuiTexture->m_pITexture->LockRect(0, &lockedRect, NULL, 0);

		pBits = lockedRect.pBits;
		pitch = lockedRect.Pitch;
#else
		rage::grcLockedTexture lockedTexture;

		if (nuiTexture->Map(0, 0, &lockedTexture, rage::grcLockFlags::Write))
		{
			pBits = lockedTexture.pBits;
			pitch = lockedTexture.pitch;
		}
		else if (nuiTexture->Map(0, 0, &lockedTexture, rage::grcLockFlags::WriteDiscard))
		{
			pBits = lockedTexture.pBits;
			pitch = lockedTexture.pitch;

			discarded = true;
		}
#endif

		if (pBits)
		{
			if (!discarded)
			{
				while (!dirtyRects.empty())
				{
					EnterCriticalSection(&renderBufferLock);
					CefRect rect = dirtyRects.front();
					dirtyRects.pop();
					LeaveCriticalSection(&renderBufferLock);

					for (int y = rect.y; y < (rect.y + rect.height); y++)
					{
						int* src = &((int*)(renderBuffer))[(y * roundedWidth) + rect.x];
						int* dest = &((int*)(pBits))[(y * (pitch / 4)) + rect.x];

						memcpy(dest, src, (rect.width * 4));
					}
				}
			}
			else
			{
				EnterCriticalSection(&renderBufferLock);
				dirtyRects = std::queue<CefRect>();
				LeaveCriticalSection(&renderBufferLock);

				memcpy(pBits, renderBuffer, height * pitch);
			}
		}

#ifndef _HAS_GRCTEXTURE_MAP
		nuiTexture->m_pITexture->UnlockRect(0);
#else
		if (pBits)
		{
			nuiTexture->Unmap(&lockedTexture);
		}
#endif

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
	if (pollQueue.find(argument) == pollQueue.end())
	{
		pollQueue.insert(argument);
	}
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

//#include "RenderCallbacks.h"
#include "DrawCommands.h"

static InitFunction initFunction([] ()
{
	OnD3DPostReset.Connect([] ()
	{
		g_nuiWindowsMutex.lock();

		for (auto& window : g_nuiWindows)
		{
			window->Invalidate();
		}

		g_nuiWindowsMutex.unlock();
	});

	OnPostFrontendRender.Connect([]()
	{
		if (g_mainUIFlag)
		{
			nui::GiveFocus(true);
		}

#if defined(GTA_NY)
		auto dc = new(0) CGenericDC([] ()
		{
#else
		uint32_t a1;
		uint32_t a2;

		EnqueueGenericDrawCommand([] (uint32_t, uint32_t)
		{
#endif
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

#if !defined(GTA_NY)
			for (auto& window : g_nuiWindows)
			{
				window->UpdateFrame();
			}
#endif

			for (auto& window : g_nuiWindows)
			{
				if (window->GetPaintType() != NUIPaintTypePostRender)
				{
					continue;
				}

				if (window->GetTexture())
				{
					SetTextureGtaIm(window->GetTexture());

					int resX, resY;
					GetGameResolution(resX, resY);

					// we need to subtract 0.5f from each vertex coordinate (half a pixel after scaling) due to the usual half-pixel/texel issue
					DrawImSprite(-0.5f, -0.5f, resX - 0.5f, resY - 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, &color, 0);
				}
			}

			g_nuiWindowsMutex.unlock();

			if (g_mainUIFlag)
			{
				//POINT cursorPos;
				//GetCursorPos(&cursorPos);

				POINT cursorPos = g_cursorPos;

				//ScreenToClient(*(HWND*)0x1849DDC, &cursorPos);

#if defined(GTA_NY)
				if (true)//!GameInit::GetGameLoaded())
				{
					SetTextureGtaIm(*(rage::grcTexture**)(0x10C8310));
				}
				else
				{
					SetTextureGtaIm(*(rage::grcTexture**)(0x18AAC20));
				}

				DrawImSprite((float)cursorPos.x, (float)cursorPos.y, (float)cursorPos.x + 40.0f, (float)cursorPos.y + 40.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, &color, 0);
#endif
			}
#if defined(GTA_NY)
		});

		dc->Enqueue();
#else
		}, &a1, &a2);
#endif
	});

	/*g_hooksDLL->SetHookCallback(StringHash("msgConfirm"), [] (void*)
	{
		nui::SetMainUI(true);

		nui::CreateFrame("mpMenu", "nui://game/ui/mpmenu.html");
	});*/
});

fwRefContainer<NUIWindow> NUIWindow::Create(bool primary, int width, int height, CefString url)
{
	auto window = new NUIWindow(primary, width, height);
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

	rage::grcManualTextureDef textureDef;
	memset(&textureDef, 0, sizeof(textureDef));
	textureDef.isStaging = 0;
	textureDef.arraySize = 1;

	nuiTexture = rage::grcTextureFactory::getInstance()->createManualTexture(width, height, FORMAT_A8R8G8B8, true, &textureDef);

	m_client = new NUIClient(this);

	CefWindowInfo info;
	info.SetAsWindowless(GetDesktopWindow(), true);

	CefBrowserSettings settings;
	settings.javascript_close_windows = STATE_DISABLED;

	CefRefPtr<CefRequestContext> rc = CefRequestContext::GetGlobalContext();
	CefBrowserHost::CreateBrowser(info, m_client, url, settings, rc);
}

void NUIWindow::AddDirtyRect(const CefRect& rect)
{
	EnterCriticalSection(&renderBufferLock);
	dirtyRects.push(rect);
	LeaveCriticalSection(&renderBufferLock);
}

static NUISchemeHandlerFactory* g_shFactory;

static fwRefContainer<NUIWindow> g_nuiResourceRootWindow;

namespace nui
{
	__declspec(dllexport) CefBrowser* GetBrowser()
	{
		if (!g_nuiResourceRootWindow.GetRef())
		{
			return nullptr;
		}

		return g_nuiResourceRootWindow->GetBrowser();
	}

	bool HasMainUI()
	{
		return g_mainUIFlag;
	}

	__declspec(dllexport) void ExecuteRootScript(const char* scriptBit)
	{
		g_nuiResourceRootWindow->GetBrowser()->GetMainFrame()->ExecuteJavaScript(scriptBit, "internal", 1);
	}

	__declspec(dllexport) void ReloadNUI()
	{
		g_nuiResourceRootWindow->GetBrowser()->ReloadIgnoreCache();
	}

	__declspec(dllexport) void SetMainUI(bool enable)
	{
		g_mainUIFlag = enable;
		nui::GiveFocus(enable);
	}

	__declspec(dllexport) void CreateFrame(fwString frameName, fwString frameURL)
	{
		auto procMessage = CefProcessMessage::Create("createFrame");
		auto argumentList = procMessage->GetArgumentList();

		argumentList->SetSize(2);
		argumentList->SetString(0, frameName.c_str());
		argumentList->SetString(1, frameURL.c_str());

		auto browser = g_nuiResourceRootWindow->GetBrowser();
		browser->SendProcessMessage(PID_RENDERER, procMessage);
	}

	__declspec(dllexport) void DestroyFrame(fwString frameName)
	{
		auto procMessage = CefProcessMessage::Create("destroyFrame");
		auto argumentList = procMessage->GetArgumentList();

		argumentList->SetSize(1);
		argumentList->SetString(0, frameName.c_str());

		auto browser = g_nuiResourceRootWindow->GetBrowser();
		browser->SendProcessMessage(PID_RENDERER, procMessage);
	}

	__declspec(dllexport) void SignalPoll(fwString frameName)
	{
		g_nuiResourceRootWindow->SignalPoll(std::string(frameName.c_str()));
	}

	NUISchemeHandlerFactory* GetSchemeHandlerFactory()
	{
		return g_shFactory;
	}

	//bool OnPreLoadGame(void* cefSandbox)
	//{
	static InitFunction initFunction([] ()
	{
		// load the CEF library
		HMODULE libcef = LoadLibraryW(MakeRelativeCitPath(L"bin/libcef.dll").c_str());

		if (!libcef)
		{
			MessageBoxW(NULL, L"Could not load bin/libcef.dll.", L"CitizenFX", MB_ICONSTOP | MB_OK);

			ExitProcess(0);
		}

		__HrLoadAllImportsForDll("libcef.dll");

		// instantiate a NUIApp
		auto selfApp = Instance<NUIApp>::Get();

		CefMainArgs args(GetModuleHandle(NULL));
		CefRefPtr<CefApp> app(selfApp);

		// try to execute as a CEF process
		int exitCode = CefExecuteProcess(args, app, nullptr);

		// and exit if we did
		if (exitCode >= 0)
		{
			ExitProcess(0);
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

		/*g_hooksDLL->SetHookCallback(StringHash("beginScene"), [] (void*)
		{
			if (g_nuiWindowsMutex.try_lock())
			{
				for (auto& window : g_nuiWindows)
				{
					window->UpdateFrame();
				}

				g_nuiWindowsMutex.unlock();
			}
		});*/

#if defined(GTA_NY)
		OnGrcBeginScene.Connect([] ()
		{
			if (g_nuiWindowsMutex.try_lock())
			{
				for (auto& window : g_nuiWindows)
				{
					window->UpdateFrame();
				}

				g_nuiWindowsMutex.unlock();
			}
		});
#else

#endif

		//g_hooksDLL->SetHookCallback(StringHash("d3dCreate"), [] (void*)
		OnGrcCreateDevice.Connect([]()
		{
			//int resX = *(int*)0xFDCEAC;
			//int resY = *(int*)0xFDCEB0;
			int resX, resY;
			GetGameResolution(resX, resY);

			g_nuiResourceRootWindow = NUIWindow::Create(true, resX, resY, "nui://game/ui/root.html");
			g_nuiResourceRootWindow->SetPaintType(NUIPaintTypePostRender);
		});

		return;
	}, 50);
}