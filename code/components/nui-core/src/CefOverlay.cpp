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
	}

	__declspec(dllexport) void SignalPoll(fwString frameName)
	{
		auto rootWindow = Instance<NUIWindowManager>::Get()->GetRootWindow();
		rootWindow->SignalPoll(std::string(frameName.c_str()));
	}
}