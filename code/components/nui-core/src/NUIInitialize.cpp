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

#include "ResumeComponent.h"
#include "HookFunction.h"

#include <CfxSubProcess.h>

#include <Error.h>

#include "memdbgon.h"

void FinalizeInitNUI();

void Component_RunPreInit()
{
#ifdef _M_AMD64
	// again, a Win7 SP1 check (Chromium x64 isn't supported below this operating level)
	if (!IsWindows7SP1OrGreater())
	{
		FatalError("CitizenFX requires Windows 7 SP1 or higher. Please upgrade to this operating system version to run CitizenFX.");
	}
#endif

	// CEF keeps loading/unloading this - load it ourselves to make the refcount always 1
	LoadLibrary(L"bluetoothapis.dll");

	// load Chrome dependencies ourselves so that the system won't try loading from other paths
	LoadLibrary(MakeRelativeCitPath(L"bin/chrome_elf.dll").c_str());
	LoadLibrary(MakeRelativeCitPath(L"bin/libEGL.dll").c_str());
	LoadLibrary(MakeRelativeCitPath(L"bin/libGLESv2.dll").c_str());

	// load the CEF library
	HMODULE libcef = LoadLibraryW(MakeRelativeCitPath(L"bin/libcef.dll").c_str());

	if (!libcef)
	{
		MessageBoxW(NULL, L"Could not load bin/libcef.dll.", L"CitizenFX", MB_ICONSTOP | MB_OK);

		ExitProcess(0);
	}

	__HrLoadAllImportsForDll("libcef.dll");

	Instance<NUIApp>::Set(new NUIApp());

	// instantiate a NUIApp
	auto selfApp = Instance<NUIApp>::Get();

	CefMainArgs args(GetModuleHandle(NULL));
	static CefRefPtr<CefApp> app(selfApp);

    auto schemeHandlerFactory = new NUISchemeHandlerFactory();
    schemeHandlerFactory->AddRef();
    Instance<NUISchemeHandlerFactory>::Set(schemeHandlerFactory);

    InitFunctionBase::RunAll();

    OnResumeGame.Connect([] ()
    {
        FinalizeInitNUI();
    });

	// try to execute as a CEF process
	int exitCode = CefExecuteProcess(args, app, nullptr);

	// and exit if we did
	if (exitCode >= 0)
	{
		ExitProcess(0);
	}
}

void FinalizeInitNUI()
{
    if (getenv("CitizenFX_ToolMode"))
    {
        return;
    }

	std::wstring cachePath = MakeRelativeCitPath(L"cache\\browser\\");
	CreateDirectory(cachePath.c_str(), nullptr);

	// delete any old CEF logs
	DeleteFile(MakeRelativeCitPath(L"debug.log").c_str());

	auto selfApp = Instance<NUIApp>::Get();

	CefMainArgs args(GetModuleHandle(NULL));
	CefRefPtr<CefApp> app(selfApp);

	CefSettings cSettings;
		
	cSettings.multi_threaded_message_loop = true;
	cSettings.remote_debugging_port = 13172;
	cSettings.pack_loading_disabled = false; // true;
	cSettings.windowless_rendering_enabled = false; // true;
	cSettings.log_severity = LOGSEVERITY_DISABLE;
	
	CefString(&cSettings.browser_subprocess_path).FromWString(MakeCfxSubProcess(L"ChromeBrowser"));

	CefString(&cSettings.locale).FromASCII("en-US");

	std::wstring resPath = MakeRelativeCitPath(L"bin/cef/");

	CefString(&cSettings.resources_dir_path).FromWString(resPath);
	CefString(&cSettings.locales_dir_path).FromWString(resPath);
	CefString(&cSettings.cache_path).FromWString(cachePath);

	// 2014-06-30: sandbox disabled as it breaks scheme handler factories (results in blank page being loaded)
	CefInitialize(args, cSettings, app.get(), /*cefSandbox*/ nullptr);
	CefRegisterSchemeHandlerFactory("nui", "", Instance<NUISchemeHandlerFactory>::Get());
	CefAddCrossOriginWhitelistEntry("nui://game", "https", "", true);
	CefAddCrossOriginWhitelistEntry("nui://game", "http", "", true);
	CefAddCrossOriginWhitelistEntry("nui://game", "nui", "", true);

    HookFunctionBase::RunAll();

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
}
