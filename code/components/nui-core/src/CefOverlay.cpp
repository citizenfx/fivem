/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include "CefOverlay.h"
#include "NUIApp.h"
#include "NUIClient.h"
#include "NUIWindowManager.h"

#include <CL2LaunchMode.h>

#include <shared_mutex>
#include <unordered_set>

#include "memdbgon.h"

bool g_mainUIFlag = true;
bool g_shouldHideCursor;

fwEvent<const wchar_t*, const wchar_t*> nui::OnInvokeNative;
fwEvent<bool> nui::OnDrawBackground;

extern bool g_shouldCreateRootWindow;
extern nui::GameInterface* g_nuiGi;

#ifdef USE_NUI_ROOTLESS
extern std::shared_mutex g_nuiFocusStackMutex;
extern std::list<std::string> g_nuiFocusStack;
#endif

namespace nui
{
#ifndef USE_NUI_ROOTLESS
	__declspec(dllexport) CefBrowser* GetBrowser()
	{
		auto rootWindow = Instance<NUIWindowManager>::Get()->GetRootWindow();

		if (!rootWindow.GetRef())
		{
			return nullptr;
		}

		return rootWindow->GetBrowser();
	}
#endif

	bool HasMainUI()
	{
		return g_mainUIFlag;
	}

	std::unordered_map<std::string, fwRefContainer<NUIWindow>> windowList;
	std::shared_mutex windowListMutex;
	
	fwRefContainer<NUIWindow> FindNUIWindow(fwString windowName)
	{
		{
			std::unique_lock<std::shared_mutex> lock(windowListMutex);
			auto windowIt = windowList.find(windowName);

			if (windowIt == windowList.end())
			{
				return nullptr;
			}

			return windowIt->second;
		}
	}

#ifndef USE_NUI_ROOTLESS
	__declspec(dllexport) void ExecuteRootScript(const std::string& scriptBit)
	{
		auto rootWindow = Instance<NUIWindowManager>::Get()->GetRootWindow();

		if (rootWindow.GetRef() && rootWindow->GetBrowser() && rootWindow->GetBrowser()->GetMainFrame())
		{
			rootWindow->GetBrowser()->GetMainFrame()->ExecuteJavaScript(scriptBit, "internal", 1);
		}
	}

	static void PostJSEvent(const std::string& type, const std::vector<std::reference_wrapper<const std::string>>& args)
	{
		auto rootWindow = Instance<NUIWindowManager>::Get()->GetRootWindow();

		if (rootWindow.GetRef() && rootWindow->GetBrowser() && rootWindow->GetBrowser()->GetMainFrame())
		{
			auto processMessage = CefProcessMessage::Create("pushEvent");
			auto argumentList = processMessage->GetArgumentList();

			argumentList->SetString(0, type);

			int argIdx = 1;

			for (auto arg : args)
			{
				argumentList->SetString(argIdx, arg.get());

				argIdx++;
			}

			rootWindow->GetBrowser()->GetMainFrame()->SendProcessMessage(PID_RENDERER, processMessage);
		}
	}

	__declspec(dllexport) void PostRootMessage(const std::string& jsonData)
	{
		PostJSEvent("rootCall", { jsonData });
	}

	__declspec(dllexport) void PostFrameMessage(const std::string& frame, const std::string& jsonData)
	{
		PostJSEvent("frameCall", { frame, jsonData });
	}
#else
	__declspec(dllexport) void PostFrameMessage(const std::string& frame, const std::string& jsonData)
	{
		auto rootWindow = FindNUIWindow(fmt::sprintf("nui_%s", frame));

		if (rootWindow.GetRef())
		{
			bool passed = false;

			auto sendMessage = [frame, jsonData]()
			{
				auto rootWindow = FindNUIWindow(fmt::sprintf("nui_%s", frame));
				rootWindow->TouchMessage();

				auto processMessage = CefProcessMessage::Create("pushEvent");
				auto argumentList = processMessage->GetArgumentList();

				argumentList->SetString(0, "frameCall");
				argumentList->SetString(1, jsonData);

				rootWindow->GetBrowser()->SendProcessMessage(PID_RENDERER, processMessage);
			};

			if (rootWindow->GetBrowser() && rootWindow->GetBrowser()->GetMainFrame())
			{
				if (rootWindow->GetBrowser()->GetHost())
				{
					auto client = rootWindow->GetBrowser()->GetHost()->GetClient();

					if (client)
					{
						auto nuiClient = (NUIClient*)client.get();

						if (nuiClient->HasLoadedMainFrame())
						{
							sendMessage();
							passed = true;
						}
					}
				}
			}

			if (!rootWindow->GetBrowser())
			{
				rootWindow->DeferredCreate();
			}
			
			if (!passed)
			{
				rootWindow->PushLoadQueue(std::move(sendMessage));
			}
		}
	}
#endif

#ifndef USE_NUI_ROOTLESS
	__declspec(dllexport) void ReloadNUI()
	{
		auto rootWindow = Instance<NUIWindowManager>::Get()->GetRootWindow();

		rootWindow->GetBrowser()->ReloadIgnoreCache();
	}
#endif

	__declspec(dllexport) void SetMainUI(bool enable)
	{
		g_mainUIFlag = enable;
		nui::GiveFocus("mpMenu", enable);
	}

	DLL_EXPORT void SetHideCursor(bool hide)
	{
		g_shouldHideCursor = hide;
	}

	fwRefContainer<NUIWindow> CreateNUIWindow(fwString windowName, int width, int height, fwString windowURL, bool rawBlit/* = false*/, bool instant)
	{
		auto window = NUIWindow::Create(rawBlit, width, height, windowURL, instant);

		std::unique_lock<std::shared_mutex> lock(windowListMutex);
		windowList[windowName] = window;

		return window;
	}

	DLL_EXPORT fwRefContainer<NUIWindow> CreateNUIWindow(fwString windowName, int width, int height, fwString windowURL, bool rawBlit)
	{
		return CreateNUIWindow(windowName, width, height, windowURL, rawBlit, true);
	}

	__declspec(dllexport) void DestroyNUIWindow(fwString windowName)
	{
		std::unique_lock<std::shared_mutex> lock(windowListMutex);

		auto windowIt = windowList.find(windowName);

		if (windowIt != windowList.end())
		{
			Instance<NUIWindowManager>::Get()->RemoveWindow(windowIt->second.GetRef());

			windowList.erase(windowName);
		}
	}

	__declspec(dllexport) CefBrowser* GetNUIWindowBrowser(fwString windowName)
	{
		auto window = FindNUIWindow(windowName);

		if (window.GetRef())
		{
			return window->GetBrowser();
		}

		return nullptr;
	}

	__declspec(dllexport) void SetNUIWindowURL(fwString windowName, fwString url)
	{
		fwRefContainer<NUIWindow> window = FindNUIWindow(windowName);

		if (!window.GetRef())
		{
			return;
		}

		if (window->GetBrowser() && window->GetBrowser()->GetMainFrame())
		{
			window->GetBrowser()->GetMainFrame()->LoadURL(url);
		}
	}

	OVERLAY_DECL fwRefContainer<nui::GITexture> GetWindowTexture(fwString windowName)
	{
		fwRefContainer<NUIWindow> window = FindNUIWindow(windowName);

		if (!window.GetRef())
		{
			return nullptr;
		}

		return window->GetTexture();
	}

	__declspec(dllexport) void ExecuteWindowScript(const std::string& windowName, const std::string& scriptBit)
	{
		fwRefContainer<NUIWindow> window = FindNUIWindow(windowName);

		if (!window.GetRef())
		{
			return;
		}

		if (window.GetRef() && window->GetBrowser() && window->GetBrowser()->GetMainFrame())
		{
			window->GetBrowser()->GetMainFrame()->ExecuteJavaScript(scriptBit, "internal", 1);
		}
	}

	static std::unordered_map<std::string, std::string> frameList;
	static std::shared_mutex frameListMutex;

	static bool rootWindowTerminated;

	static void CreateFrame(fwString frameName, fwString frameURL, bool instant)
	{
		if (frameName == "mpMenu" && launch::IsSDKGuest())
		{
			return;
		}

#ifdef IS_LAUNCHER
#ifndef USE_NUI_ROOTLESS
		if (rootWindowTerminated)
		{
			g_shouldCreateRootWindow = true;
			rootWindowTerminated = false;
			return;
		}
#endif
#endif

		bool exists = false;

		{
			std::shared_lock<std::shared_mutex> lock(frameListMutex);
			exists = frameList.find(frameName) != frameList.end();
		}

		if (!exists)
		{
#ifndef USE_NUI_ROOTLESS
			auto rootWindow = Instance<NUIWindowManager>::Get()->GetRootWindow();
			auto browser = rootWindow->GetBrowser();

			if (browser)
			{
				auto procMessage = CefProcessMessage::Create("createFrame");
				auto argumentList = procMessage->GetArgumentList();

				argumentList->SetSize(2);
				argumentList->SetString(0, frameName.c_str());
				argumentList->SetString(1, frameURL.c_str());

				browser->GetMainFrame()->SendProcessMessage(PID_RENDERER, procMessage);
			}
#else
			int resX, resY;
			g_nuiGi->GetGameResolution(&resX, &resY);

			auto winName = fmt::sprintf("nui_%s", frameName);

			auto window = CreateNUIWindow(winName, resX, resY, frameURL, true, instant);
			window->SetPaintType(NUIPaintTypePostRender);
			window->SetName(winName);

			{
				std::unique_lock<std::shared_mutex> lock(g_nuiFocusStackMutex);
				g_nuiFocusStack.push_back(winName);
			}
#endif

			std::unique_lock<std::shared_mutex> lock(frameListMutex);
			frameList.insert({ frameName, frameURL });
		}
	}

	DLL_EXPORT void CreateFrame(fwString frameName, fwString frameURL)
	{
		CreateFrame(frameName, frameURL, true);
	}

	DLL_EXPORT void PrepareFrame(fwString frameName, fwString frameURL)
	{
		CreateFrame(frameName, frameURL, false);
	}

	__declspec(dllexport) void DestroyFrame(fwString frameName)
	{
#ifndef USE_NUI_ROOTLESS
		auto procMessage = CefProcessMessage::Create("destroyFrame");
		auto argumentList = procMessage->GetArgumentList();

		argumentList->SetSize(1);
		argumentList->SetString(0, frameName.c_str());

		auto rootWindow = Instance<NUIWindowManager>::Get()->GetRootWindow();

		if (rootWindow.GetRef())
		{
			auto browser = rootWindow->GetBrowser();

			if (browser)
			{
				browser->GetMainFrame()->SendProcessMessage(PID_RENDERER, procMessage);
			}

			std::unique_lock<std::shared_mutex> lock(frameListMutex);
			frameList.erase(frameName);

#ifdef IS_LAUNCHER
			if (frameList.empty())
			{
				rootWindowTerminated = true;

				browser->GetHost()->CloseBrowser(true);
				Instance<NUIWindowManager>::Get()->RemoveWindow(rootWindow.GetRef());
				Instance<NUIWindowManager>::Get()->SetRootWindow({});
			}
#endif
		}
#else
		auto winName = fmt::sprintf("nui_%s", frameName);

		{
			std::unique_lock<std::shared_mutex> lock(g_nuiFocusStackMutex);

			for (auto it = g_nuiFocusStack.begin(); it != g_nuiFocusStack.end(); )
			{
				if (*it == winName)
				{
					it = g_nuiFocusStack.erase(it);
				}
				else
				{
					++it;
				}
			}
		}

		DestroyNUIWindow(winName);

		std::unique_lock<std::shared_mutex> lock(frameListMutex);
		frameList.erase(frameName);
#endif
	}

	bool HasFrame(const std::string& frameName)
	{
		std::shared_lock<std::shared_mutex> lock(frameListMutex);
		return frameList.find(frameName) != frameList.end();
	}

	void RecreateFrames()
	{
		std::vector<std::pair<std::string, std::string>> frameData;

		{
			std::shared_lock<std::shared_mutex> lock(frameListMutex);

			for (auto& frame : frameList)
			{
				frameData.push_back(frame);
			}
		}

		{
			std::unique_lock<std::shared_mutex> lock(frameListMutex);
			frameList.clear();
		}

		for (auto& frame : frameData)
		{
			CreateFrame(frame.first, frame.second);
		}
	}

	__declspec(dllexport) void SignalPoll(fwString frameName)
	{
#ifndef USE_NUI_ROOTLESS
		auto rootWindow = Instance<NUIWindowManager>::Get()->GetRootWindow();
		rootWindow->SignalPoll(std::string(frameName.c_str()));
#else
		auto frameWindow = FindNUIWindow(fmt::sprintf("nui_%s", frameName));
		frameWindow->SignalPoll(std::string(frameName.c_str()));
#endif
	}
}
