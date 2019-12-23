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

#include <CefOverlay.h>

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

nui::GameInterface* g_nuiGi;

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

static HRESULT(*g_origD3D11CreateDevice)(_In_opt_ IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, _In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, _COM_Outptr_opt_ ID3D11Device** ppDevice, _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel, _COM_Outptr_opt_ ID3D11DeviceContext** ppImmediateContext);

static HRESULT (*g_origCreateTexture2D)(
	ID3D11Device* device,
	const D3D11_TEXTURE2D_DESC* pDesc,
	const D3D11_SUBRESOURCE_DATA* pInitialData,
	ID3D11Texture2D** ppTexture2D
);

#include <unordered_set>

#include <wrl.h>
#include <dxgi1_3.h>

#include <HostSharedData.h>
#include <CfxState.h>

namespace WRL = Microsoft::WRL;

MIDL_INTERFACE("035f3ab4-482e-4e50-b41f-8a7f8bd8960b")
IDXGIResourceHack : public IDXGIDeviceSubObject
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetSharedHandle(
		/* [annotation][out] */
		_Out_  HANDLE * pSharedHandle) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetUsage(
		/* [out] */ DXGI_USAGE* pUsage) = 0;

	virtual HRESULT STDMETHODCALLTYPE SetEvictionPriority2(
		/* [in] */ UINT EvictionPriority) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetEvictionPriority(
		/* [annotation][retval][out] */
		_Out_  UINT* pEvictionPriority) = 0;

};

MIDL_INTERFACE("EF93CE51-2C32-488A-ADC1-3453BC0D1664")
IMyTexture : public IUnknown
{
	virtual void GetOriginal(ID3D11Resource** dst) = 0;
};

class Texture2DWrap : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IDXGIResourceHack, ID3D11Texture2D, IMyTexture>
{
private:
	WRL::ComPtr<ID3D11Texture2D> m_texture;
	WRL::ComPtr<IDXGIResource> m_resource;
	WRL::ComPtr<IDXGIResource1> m_resource1;

public:
	Texture2DWrap(ID3D11Texture2D* res)
	{
		m_texture.Attach(res);
		m_texture.As(&m_resource);
		m_texture.As(&m_resource1);
	}

	// Inherited via RuntimeClass
	virtual HRESULT __stdcall SetPrivateData(REFGUID Name, UINT DataSize, const void* pData) override
	{
		return m_texture->SetPrivateData(Name, DataSize, pData);
	}

	virtual HRESULT __stdcall SetPrivateDataInterface(REFGUID Name, const IUnknown* pUnknown) override
	{
		return m_texture->SetPrivateDataInterface(Name, pUnknown);
	}

	virtual HRESULT __stdcall GetPrivateData(REFGUID Name, UINT* pDataSize, void* pData) override
	{
		return m_texture->GetPrivateData(Name, pDataSize, pData);
	}

	virtual HRESULT __stdcall GetParent(REFIID riid, void** ppParent) override
	{
		return m_resource->GetParent(riid, ppParent);
	}

	virtual HRESULT __stdcall GetDevice(REFIID riid, void** ppDevice) override
	{
		return m_resource->GetDevice(riid, ppDevice);
	}

	virtual HRESULT __stdcall GetSharedHandle(HANDLE* pSharedHandle) override
	{
		HANDLE handle;
		HRESULT hr = m_resource1->CreateSharedHandle(NULL, DXGI_SHARED_RESOURCE_READ, NULL, &handle);

		static HostSharedData<CfxState> initState("CfxInitState");

		if (SUCCEEDED(hr))
		{
			auto proc = OpenProcess(PROCESS_DUP_HANDLE, FALSE, initState->GetInitialPid());
			DuplicateHandle(GetCurrentProcess(), handle, proc, pSharedHandle, 0, FALSE, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);
			CloseHandle(proc);
		}

		return hr;
	}

	virtual HRESULT __stdcall GetUsage(DXGI_USAGE* pUsage) override
	{
		return m_resource->GetUsage(pUsage);
	}

	virtual HRESULT __stdcall SetEvictionPriority2(UINT EvictionPriority) override
	{
		return m_resource->SetEvictionPriority(EvictionPriority);
	}

	virtual void __stdcall SetEvictionPriority(UINT EvictionPriority) override
	{
		m_resource->SetEvictionPriority(EvictionPriority);
	}

	virtual void __stdcall GetDevice(ID3D11Device** ppDevice) override
	{
		return m_texture->GetDevice(ppDevice);
	}

	virtual void __stdcall GetType(D3D11_RESOURCE_DIMENSION* pResourceDimension) override
	{
		return m_texture->GetType(pResourceDimension);
	}

	virtual HRESULT __stdcall GetEvictionPriority(UINT*) override
	{
		return S_OK;
	}

	virtual UINT __stdcall GetEvictionPriority(void) override
	{
		return 0;
	}

	virtual void __stdcall GetDesc(D3D11_TEXTURE2D_DESC* pDesc) override
	{
		return m_texture->GetDesc(pDesc);
	}

	virtual void GetOriginal(ID3D11Resource** dst) override
	{
		*dst = m_texture.Get();
	}
};

static HRESULT CreateTexture2DHook(
	ID3D11Device* device,
	D3D11_TEXTURE2D_DESC* pDesc,
	const D3D11_SUBRESOURCE_DATA* pInitialData,
	ID3D11Texture2D** ppTexture2D
)
{
	auto& desc = *pDesc;

	bool wrapped = false;

	if (IsWindows8OrGreater())
	{
		if ((desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED) || (desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX))
		{
			desc.MiscFlags |= D3D11_RESOURCE_MISC_SHARED_NTHANDLE;
			wrapped = true;
		}
	}

	auto rv = g_origCreateTexture2D(device, &desc, pInitialData, ppTexture2D);

	if (wrapped)
	{
		auto retVal = WRL::Make<Texture2DWrap>(*ppTexture2D);
		retVal.CopyTo(ppTexture2D);
	}

	return rv;
}

static HRESULT(*g_origCopyResource)(ID3D11DeviceContext* cxt, ID3D11Resource* dst, ID3D11Resource* src);

static HRESULT CopyResourceHook(ID3D11DeviceContext* cxt, ID3D11Resource* dst, ID3D11Resource* src)
{
	WRL::ComPtr<ID3D11Resource> dstRef(dst);
	WRL::ComPtr<IMyTexture> mt;

	if (SUCCEEDED(dstRef.As(&mt)))
	{
		mt->GetOriginal(&dst);
	}

	WRL::ComPtr<ID3D11Resource> srcRef(src);

	if (SUCCEEDED(srcRef.As(&mt)))
	{
		mt->GetOriginal(&src);
	}

	return g_origCopyResource(cxt, dst, src);
}

#include <dxgi1_6.h>

#pragma comment(lib, "dxgi.lib")

static HRESULT D3D11CreateDeviceHook(_In_opt_ IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, _In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, _COM_Outptr_opt_ ID3D11Device** ppDevice, _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel, _COM_Outptr_opt_ ID3D11DeviceContext** ppImmediateContext)
{
	if (!pAdapter)
	{
		WRL::ComPtr<IDXGIFactory> dxgiFactory;
		CreateDXGIFactory(IID_IDXGIFactory, &dxgiFactory);

		WRL::ComPtr<IDXGIAdapter1> adapter;
		WRL::ComPtr<IDXGIFactory6> factory6;
		HRESULT hr = dxgiFactory.As(&factory6);
		if (SUCCEEDED(hr))
		{
			for (UINT adapterIndex = 0;
				DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(
					adapterIndex,
					DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
					IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()));
				adapterIndex++)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter.
					continue;
				}

				adapter.CopyTo(&pAdapter);

				break;
			}
		}
	}

	auto hr = g_origD3D11CreateDevice(pAdapter, pAdapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);

#if defined(IS_RDR3)
	if (SUCCEEDED(hr) && ppDevice && ppImmediateContext)
	{
		auto vtbl = **(intptr_t***)ppDevice;
		auto vtblCxt = **(intptr_t***)ppImmediateContext;
		MH_CreateHook((void*)vtbl[5], CreateTexture2DHook, (void**)&g_origCreateTexture2D);
		MH_CreateHook((void*)vtblCxt[47], CopyResourceHook, (void**)&g_origCopyResource);
		MH_EnableHook(MH_ALL_HOOKS);
	}
#endif

	return hr;
}

void HookLibGL(HMODULE libGL)
{
	MH_Initialize();
	MH_CreateHook(GetProcAddress(libGL, "glTexParameterf"), glTexParameterfHook, (void**)&g_origglTexParameterf);
	MH_CreateHook(GetProcAddress(LoadLibraryW(L"d3d11.dll"), "D3D11CreateDevice"), D3D11CreateDeviceHook, (void**)&g_origD3D11CreateDevice);

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
	g_nuiGi->GetGameResolution(&resX, &resY);

	auto rootWindow = NUIWindow::Create(true, resX, resY, "nui://game/ui/root.html");
	rootWindow->SetPaintType(NUIPaintTypePostRender);

	Instance<NUIWindowManager>::Get()->SetRootWindow(rootWindow);
}

bool g_shouldCreateRootWindow;

namespace nui
{
void Initialize(nui::GameInterface* gi)
{
	g_nuiGi = gi;

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

	g_nuiGi->OnInitRenderer.Connect([]()
	{
		CreateRootWindow();
	});

	g_nuiGi->OnRender.Connect([]()
	{
		if (g_shouldCreateRootWindow)
		{
			{
				auto rw = Instance<NUIWindowManager>::Get()->GetRootWindow().GetRef();

				if (rw)
				{
					Instance<NUIWindowManager>::Get()->RemoveWindow(rw);
					Instance<NUIWindowManager>::Get()->SetRootWindow({});
				}
			}

			CreateRootWindow();

			g_shouldCreateRootWindow = false;
		}
	});

	g_nuiGi->OnInitVfs.Connect([]()
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
	});
}
}
