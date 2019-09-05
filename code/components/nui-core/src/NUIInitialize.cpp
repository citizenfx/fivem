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

#include <HostSharedData.h>

#include <Error.h>

#include <MinHook.h>

#include "memdbgon.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <VFSManager.h>
#include <VFSZipFile.h>

void FinalizeInitNUI();

struct GameRenderData
{
	HANDLE handle;
	int width;
	int height;
	bool requested;

	GameRenderData()
		: requested(false)
	{

	}
};

static void(*g_origglTexParameterf)(GLenum target, GLenum pname, GLfloat param);

static void glTexParameterfHook(GLenum target, GLenum pname, GLfloat param)
{
	// 'secret' activation sequence
	static int stage = 0;

	if (target == GL_TEXTURE_2D && pname == GL_TEXTURE_WRAP_T)
	{
		switch (stage)
		{
		case 0:
			if (param == GL_CLAMP_TO_EDGE)
			{
				stage = 1;
			}

			break;
		case 1:
			if (param == GL_MIRRORED_REPEAT)
			{
				stage = 2;
			}
			else
			{
				stage = 0;
			}

			break;
		case 2:
			if (param == GL_REPEAT)
			{
				stage = 3;
			}
			else
			{
				stage = 0;
			}

			break;
		}
	}
	else
	{
		stage = 0;
	}

	if (stage == 3)
	{
		auto _eglGetCurrentDisplay = (decltype(&eglGetCurrentDisplay))(GetProcAddress(GetModuleHandle(L"libEGL.dll"), "eglGetCurrentDisplay"));
		auto _eglChooseConfig = (decltype(&eglChooseConfig))(GetProcAddress(GetModuleHandle(L"libEGL.dll"), "eglChooseConfig"));
		auto _eglGetConfigs = (decltype(&eglGetConfigs))(GetProcAddress(GetModuleHandle(L"libEGL.dll"), "eglGetConfigs"));
		auto _eglCreatePbufferFromClientBuffer = (decltype(&eglCreatePbufferFromClientBuffer))(GetProcAddress(GetModuleHandle(L"libEGL.dll"), "eglCreatePbufferFromClientBuffer"));
		auto _eglBindTexImage = (decltype(&eglBindTexImage))(GetProcAddress(GetModuleHandle(L"libEGL.dll"), "eglBindTexImage"));
		auto _eglGetError = (decltype(&eglGetError))(GetProcAddress(GetModuleHandle(L"libEGL.dll"), "eglGetError"));

		auto m_display = _eglGetCurrentDisplay();

		static HostSharedData<GameRenderData> handleData("CfxGameRenderHandle");

		EGLint pbuffer_attributes[] =
		{
			EGL_WIDTH,  handleData->width,
			EGL_HEIGHT,  handleData->height,
			EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
			EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA,
			EGL_NONE
		};

		EGLConfig configs;
		EGLint numConfigs = 0;

		EGLint config_attributes[] =
		{
			EGL_RED_SIZE,			8,
			EGL_GREEN_SIZE,			8,
			EGL_BLUE_SIZE,			8,
			EGL_ALPHA_SIZE,			8,
			EGL_NONE,				EGL_NONE,
		};

		_eglChooseConfig(m_display, config_attributes, &configs, 1, &numConfigs);

		if (numConfigs == 0)
		{
			_eglGetConfigs(m_display, &configs, 1, &numConfigs);
		}

		EGLSurface pbuffer = _eglCreatePbufferFromClientBuffer(
			m_display,
			EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE,
			(EGLClientBuffer)handleData->handle,
			configs,
			pbuffer_attributes);

		handleData->requested = true;

		auto err = _eglGetError();

		_eglBindTexImage(m_display, pbuffer, EGL_BACK_BUFFER);

		stage = 0;
	}
	else if (stage <= 1)
	{
		g_origglTexParameterf(target, pname, param);
	}
}

void HookLibGL(HMODULE libGL)
{
	MH_Initialize();
	MH_CreateHook(GetProcAddress(libGL, "glTexParameterf"), glTexParameterfHook, (void**)&g_origglTexParameterf);
	MH_EnableHook(MH_ALL_HOOKS);
}

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
	HMODULE libGL = LoadLibrary(MakeRelativeCitPath(L"bin/libGLESv2.dll").c_str());

	// hook libGLESv2 for Cfx purposes
	HookLibGL(libGL);

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

void CreateRootWindow()
{
	int resX, resY;
	GetGameResolution(resX, resY);

	auto rootWindow = NUIWindow::Create(true, resX, resY, "nui://game/ui/root.html");
	rootWindow->SetPaintType(NUIPaintTypePostRender);

	Instance<NUIWindowManager>::Get()->SetRootWindow(rootWindow);
}

bool g_shouldCreateRootWindow;

void FinalizeInitNUI()
{
    if (getenv("CitizenFX_ToolMode"))
    {
        return;
    }

	std::wstring cachePath = MakeRelativeCitPath(L"cache\\browser\\");
	CreateDirectory(cachePath.c_str(), nullptr);

	// delete any old CEF logs
	DeleteFile(MakeRelativeCitPath(L"cef.log").c_str());

	auto selfApp = Instance<NUIApp>::Get();

	CefMainArgs args(GetModuleHandle(NULL));
	CefRefPtr<CefApp> app(selfApp);

	CefSettings cSettings;
		
	cSettings.multi_threaded_message_loop = true;
	cSettings.remote_debugging_port = 13172;
	cSettings.pack_loading_disabled = false; // true;
	cSettings.windowless_rendering_enabled = true;
	cSettings.log_severity = LOGSEVERITY_DEFAULT;
	cSettings.background_color = 0;
	
	CefString(&cSettings.log_file).FromWString(MakeRelativeCitPath(L"cef.log"));
	
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

	CefRegisterSchemeHandlerFactory("https", "nui-game-internal", Instance<NUISchemeHandlerFactory>::Get());
	CefAddCrossOriginWhitelistEntry("https://nui-game-internal", "https", "", true);
	CefAddCrossOriginWhitelistEntry("https://nui-game-internal", "http", "", true);
	CefAddCrossOriginWhitelistEntry("https://nui-game-internal", "nui", "", true);

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

	OnGrcCreateDevice.Connect([]()
	{
		CreateRootWindow();
	});

	OnPostFrontendRender.Connect([]()
	{
		if (g_shouldCreateRootWindow)
		{
			Instance<NUIWindowManager>::Get()->RemoveWindow(Instance<NUIWindowManager>::Get()->GetRootWindow().GetRef());
			Instance<NUIWindowManager>::Get()->SetRootWindow({});

			CreateRootWindow();

			g_shouldCreateRootWindow = false;
		}
	});

	rage::fiDevice::OnInitialMount.Connect([]()
	{
		auto zips = { "citizen:/ui.zip", "citizen:/ui-big.zip" };

		for (auto zip : zips)
		{
			fwRefContainer<vfs::ZipFile> file = new vfs::ZipFile();

			if (file->OpenArchive(zip))
			{
				vfs::Mount(file, "citizen:/ui/");
			}
		}
	}, 100);
}
