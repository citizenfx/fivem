/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIApp.h"
#include "NUISchemeHandlerFactory.h"
#include "NUIWindowManager.h"

#include <DrawCommands.h>
#include <fiDevice.h>

#include <delayimp.h>

#include <include/cef_origin_whitelist.h>

#include "memdbgon.h"

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

	auto schemeHandlerFactory = new NUISchemeHandlerFactory();
	Instance<NUISchemeHandlerFactory>::Set(schemeHandlerFactory);

	// 2014-06-30: sandbox disabled as it breaks scheme handler factories (results in blank page being loaded)
	CefInitialize(args, cSettings, app.get(), /*cefSandbox*/ nullptr);
	CefRegisterSchemeHandlerFactory("nui", "", schemeHandlerFactory);
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
		Instance<NUIWindowManager>::Get()->ForAllWindows([] (fwRefContainer<NUIWindow> window)
		{
			window->UpdateFrame();
		});
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

		auto rootWindow = NUIWindow::Create(true, resX, resY, "nui://game/ui/root.html");
		rootWindow->SetPaintType(NUIPaintTypePostRender);

		Instance<NUIWindowManager>::Get()->SetRootWindow(rootWindow);
	});

	rage::fiDevice::OnInitialMount.Connect([] ()
	{
		std::wstring emojiPack = MakeRelativeCitPath(L"citizen/emoji.rpf");
		char emojiPath[MAX_PATH];

		wcstombs(emojiPath, emojiPack.c_str(), sizeof(emojiPath));

		rage::fiPackfile* packFile = new rage::fiPackfile();
		packFile->openArchive(emojiPath, true, false, 0);
		packFile->mount("citizen:/ui/img/emoji/");
	}, 100);

	return;
}, 50);