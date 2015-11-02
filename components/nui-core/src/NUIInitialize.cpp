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
	// CEF keeps loading/unloading this - load it ourselves to make the refcount always 1
	LoadLibrary(L"bluetoothapis.dll");

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
		
	cSettings.multi_threaded_message_loop = true;
	cSettings.remote_debugging_port = 13172;
	cSettings.pack_loading_disabled = false; // true;
	cSettings.windowless_rendering_enabled = false; // true;
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

#ifndef GTA_FIVE
	rage::fiDevice::OnInitialMount.Connect([] ()
	{
		std::wstring emojiPack = MakeRelativeCitPath(L"citizen/emoji.rpf");

		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
		std::string emojiPath = converter.to_bytes(emojiPack);

		rage::fiPackfile* packFile = new rage::fiPackfile();
		packFile->OpenPackfile(emojiPath.c_str(), true, false, 0);
		packFile->Mount("citizen:/ui/img/emoji/");
	}, 100);
#endif

	return;
}, 50);