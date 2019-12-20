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

#include <shared_mutex>
#include <unordered_set>

#include "memdbgon.h"

bool g_mainUIFlag = true;

fwEvent<const wchar_t*, const wchar_t*> nui::OnInvokeNative;
fwEvent<bool> nui::OnDrawBackground;

extern bool g_shouldCreateRootWindow;

namespace nui
{
	__declspec(dllexport) CefBrowser* GetBrowser()
	{
		auto rootWindow = Instance<NUIWindowManager>::Get()->GetRootWindow();

		if (!rootWindow.GetRef())
		{
			return nullptr;
		}

		return rootWindow->GetBrowser();
	}

	bool HasMainUI()
	{
		return g_mainUIFlag;
	}

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

			rootWindow->GetBrowser()->SendProcessMessage(PID_RENDERER, processMessage);
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

	__declspec(dllexport) void ReloadNUI()
	{
		auto rootWindow = Instance<NUIWindowManager>::Get()->GetRootWindow();

		rootWindow->GetBrowser()->ReloadIgnoreCache();
	}

	__declspec(dllexport) void SetMainUI(bool enable)
	{
		g_mainUIFlag = enable;
		nui::GiveFocus(enable);
	}

	static std::unordered_map<std::string, fwRefContainer<NUIWindow>> windowList;
	static std::shared_mutex windowListMutex;

	__declspec(dllexport) void CreateNUIWindow(fwString windowName, int width, int height, fwString windowURL)
	{
		auto window = NUIWindow::Create(false, width, height, windowURL);

		std::unique_lock<std::shared_mutex> lock(windowListMutex);
		windowList[windowName] = window;
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

	static fwRefContainer<NUIWindow> FindNUIWindow(fwString windowName)
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

	OVERLAY_DECL nui::GITexture* GetWindowTexture(fwString windowName)
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

	__declspec(dllexport) void CreateFrame(fwString frameName, fwString frameURL)
	{
#ifdef IS_LAUNCHER
		if (rootWindowTerminated)
		{
			g_shouldCreateRootWindow = true;
			rootWindowTerminated = false;
			return;
		}
#endif

		bool exists = false;

		{
			std::shared_lock<std::shared_mutex> lock(frameListMutex);
			exists = frameList.find(frameName) != frameList.end();
		}

		if (!exists)
		{
			auto procMessage = CefProcessMessage::Create("createFrame");
			auto argumentList = procMessage->GetArgumentList();

			argumentList->SetSize(2);
			argumentList->SetString(0, frameName.c_str());
			argumentList->SetString(1, frameURL.c_str());

			auto rootWindow = Instance<NUIWindowManager>::Get()->GetRootWindow();
			auto browser = rootWindow->GetBrowser();
			browser->SendProcessMessage(PID_RENDERER, procMessage);

			std::unique_lock<std::shared_mutex> lock(frameListMutex);
			frameList.insert({ frameName, frameURL });
		}
	}

	__declspec(dllexport) void DestroyFrame(fwString frameName)
	{
		auto procMessage = CefProcessMessage::Create("destroyFrame");
		auto argumentList = procMessage->GetArgumentList();

		argumentList->SetSize(1);
		argumentList->SetString(0, frameName.c_str());

		auto rootWindow = Instance<NUIWindowManager>::Get()->GetRootWindow();

		if (rootWindow.GetRef())
		{
			auto browser = rootWindow->GetBrowser();
			browser->SendProcessMessage(PID_RENDERER, procMessage);

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
		auto rootWindow = Instance<NUIWindowManager>::Get()->GetRootWindow();
		rootWindow->SignalPoll(std::string(frameName.c_str()));
	}
}
