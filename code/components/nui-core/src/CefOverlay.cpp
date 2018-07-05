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

		Instance<NUIWindowManager>::Get()->AddWindow(window.GetRef());

		std::unique_lock<std::shared_mutex> lock(windowListMutex);
		windowList[windowName] = window;
	}

	__declspec(dllexport) void DestroyNUIWindow(fwString windowName)
	{
		std::unique_lock<std::shared_mutex> lock(windowListMutex);
		windowList.erase(windowName);
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

	OVERLAY_DECL rage::grcTexture* GetWindowTexture(fwString windowName)
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

	static std::unordered_set<std::string> frameList;
	static std::shared_mutex frameListMutex;

	__declspec(dllexport) void CreateFrame(fwString frameName, fwString frameURL)
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
		frameList.insert(frameName);
	}

	__declspec(dllexport) void DestroyFrame(fwString frameName)
	{
		auto procMessage = CefProcessMessage::Create("destroyFrame");
		auto argumentList = procMessage->GetArgumentList();

		argumentList->SetSize(1);
		argumentList->SetString(0, frameName.c_str());

		auto rootWindow = Instance<NUIWindowManager>::Get()->GetRootWindow();
		auto browser = rootWindow->GetBrowser();
		browser->SendProcessMessage(PID_RENDERER, procMessage);

		std::unique_lock<std::shared_mutex> lock(frameListMutex);
		frameList.erase(frameName);
	}

	bool HasFrame(const std::string& frameName)
	{
		std::shared_lock<std::shared_mutex> lock(frameListMutex);
		return frameList.find(frameName) != frameList.end();
	}

	__declspec(dllexport) void SignalPoll(fwString frameName)
	{
		auto rootWindow = Instance<NUIWindowManager>::Get()->GetRootWindow();
		rootWindow->SignalPoll(std::string(frameName.c_str()));
	}
}
