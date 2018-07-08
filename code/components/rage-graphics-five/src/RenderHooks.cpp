#include "StdInc.h"

//#include <ETWProviders/etwprof.h>

#include <mutex>

#include <dxgi1_4.h>
#include <dxgi1_5.h>
#include <wrl.h>

#include "DrawCommands.h"
#include "Hooking.h"

#include <MinHook.h>

#include <Error.h>

#include <CoreConsole.h>

namespace WRL = Microsoft::WRL;

fwEvent<> OnGrcCreateDevice;
fwEvent<> OnPostFrontendRender;

static void(*g_origCreateCB)(const char*);

static void InvokeCreateCB(const char* arg)
{
	g_origCreateCB(arg);

	//OnGrcCreateDevice();
}

static void InvokeRender()
{
	static std::once_flag of;

	std::call_once(of, [] ()
	{
		OnGrcCreateDevice();
	});

	OnPostFrontendRender();
}

#pragma comment(lib, "d3d11.lib")

static IDXGISwapChain1* g_swapChain1;
static DWORD g_swapChainFlags;
static ID3D11DeviceContext* g_dc;

static bool g_allowTearing;
static bool g_disableRendering;

void MakeDummyDevice(ID3D11Device** device, ID3D11DeviceContext** context, const DXGI_SWAP_CHAIN_DESC* desc, IDXGISwapChain** swapChain);

static HRESULT CreateD3D11DeviceWrap(_In_opt_ IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, _In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, _In_opt_ CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, _Out_opt_ IDXGISwapChain** ppSwapChain, _Out_opt_ ID3D11Device** ppDevice, _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel, _Out_opt_ ID3D11DeviceContext** ppImmediateContext)
{
	if (g_disableRendering)
	{
		*pFeatureLevel = D3D_FEATURE_LEVEL_11_0;

		MakeDummyDevice(ppDevice, ppImmediateContext, pSwapChainDesc, ppSwapChain);

		return S_OK;
	}

	if (!IsWindows10OrGreater())
	{
		return D3D11CreateDeviceAndSwapChain(/*pAdapter*/nullptr, /*DriverType*/ D3D_DRIVER_TYPE_HARDWARE, Software, Flags | D3D11_CREATE_DEVICE_BGRA_SUPPORT, pFeatureLevels, FeatureLevels/*nullptr, 0*/, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
	}

	HRESULT hr = D3D11CreateDevice(/*pAdapter*/nullptr, /*DriverType*/ D3D_DRIVER_TYPE_HARDWARE, Software, Flags | D3D11_CREATE_DEVICE_BGRA_SUPPORT/* | D3D11_CREATE_DEVICE_DEBUG*/, pFeatureLevels, FeatureLevels/*nullptr, 0*/, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);

	WRL::ComPtr<IDXGIFactory2> dxgiFactory;

	WRL::ComPtr<IDXGIDevice> dxgiDevice;
	WRL::ComPtr<IDXGIAdapter> dxgiAdapter;

	if (SUCCEEDED(hr))
	{
		//hr = CreateDXGIFactory2(0, __uuidof(IDXGIFactory2), &factory);
		(*ppDevice)->QueryInterface(__uuidof(IDXGIDevice), &dxgiDevice);
		dxgiDevice->GetAdapter(&dxgiAdapter);
		dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), &dxgiFactory);
	}

	WRL::ComPtr<IDXGISwapChain1> swapChain1;

	if (SUCCEEDED(hr))
	{
		DXGI_SWAP_CHAIN_DESC1 scDesc1 = { 0 };
		scDesc1.Width = pSwapChainDesc->BufferDesc.Width;
		scDesc1.Height = pSwapChainDesc->BufferDesc.Height;
		scDesc1.Format = pSwapChainDesc->BufferDesc.Format;
		scDesc1.BufferCount = 2;
		scDesc1.BufferUsage = pSwapChainDesc->BufferUsage;
		scDesc1.Flags = pSwapChainDesc->Flags | DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		scDesc1.SampleDesc = pSwapChainDesc->SampleDesc;
		scDesc1.SwapEffect = pSwapChainDesc->SwapEffect;

		/*
		// probe if DXGI 1.5 is available (Win10 RS1+)
		WRL::ComPtr<IDXGIFactory5> factory5;

		if (SUCCEEDED(dxgiFactory.As(&factory5)))
		{
			scDesc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			scDesc1.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
			g_allowTearing = true;
		}
		*/

		g_swapChainFlags = scDesc1.Flags;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsDesc = { 0 };
		fsDesc.RefreshRate = pSwapChainDesc->BufferDesc.RefreshRate;
		fsDesc.Scaling = pSwapChainDesc->BufferDesc.Scaling;
		fsDesc.ScanlineOrdering = pSwapChainDesc->BufferDesc.ScanlineOrdering;
		fsDesc.Windowed = pSwapChainDesc->Windowed;

		dxgiFactory->CreateSwapChainForHwnd(*ppDevice, pSwapChainDesc->OutputWindow, &scDesc1, &fsDesc, nullptr, &swapChain1);

		swapChain1->QueryInterface(__uuidof(IDXGISwapChain), (void**)ppSwapChain);
	}

	// patch stuff here as only now do we know swapchain flags
	auto pattern = hook::pattern("C7 44 24 28 02 00 00 00 89 44 24 20 41").count(2);

	hook::put<uint32_t>(pattern.get(0).get<void>(4), g_swapChainFlags | 2);
	hook::put<uint32_t>(pattern.get(1).get<void>(4), g_swapChainFlags | 2);

	// we assume all users will stop using the object by the time it is dereferenced
	g_swapChain1 = swapChain1.Get();

	g_dc = *ppImmediateContext;

	return hr;
}

struct VideoModeInfo
{
	int width;
	int height;
	int refreshRateNumerator;
	int refreshRateDenominator;
	bool fullscreen;
};

static bool(*g_origVideoModeChange)(VideoModeInfo* info);

IDXGISwapChain** g_dxgiSwapChain;

#include <atArray.h>
#include <grcTexture.h>

namespace rage
{
	class grcRenderTargetDX11 : public grcTexture
	{
	public:
		char m_pad[136 - sizeof(grcTexture)];
		ID3D11RenderTargetView* m_rtv;
		void* m_pad2;
		ID3D11Resource* m_resource2;
		atArray<ID3D11ShaderResourceView*> m_srvs;
	};
}

static bool(*g_resetVideoMode)(VideoModeInfo*);

bool WrapVideoModeChange(VideoModeInfo* info)
{
	trace("Changing video mode.\n");

	bool success = g_origVideoModeChange(info);

	trace("Changing video mode success: %d.\n", success);

	if (success)
	{
		g_resetVideoMode(info);
	}

#if 0
	IUnknown** g_backbuffer = (IUnknown**)0x14299D640;
	rage::grcRenderTargetDX11** g_backBufferRT = (rage::grcRenderTargetDX11**)0x14299DC50;

	//delete (*g_backBufferRT);

	//ULONG refCnt = (*g_backbuffer)->Release();

	ULONG refCnt = (*g_backBufferRT)->m_rtv->Release();
	(*g_backBufferRT)->m_rtv = nullptr;

	trace("refcnt %d\n", refCnt);

	if ((*g_backBufferRT)->m_resource2)
	{
		(*g_backBufferRT)->m_resource2->Release();
		(*g_backBufferRT)->m_resource2 = nullptr;
	}

	for (auto& srv : (*g_backBufferRT)->m_srvs)
	{
		srv->Release();
	}

	(*g_backBufferRT)->m_srvs.Clear();

	hook::put<uint8_t>(0x141299035, 0xC3);
	hook::put<uint8_t>(0x141298F60, 0x48);

	((void(*)(void*))0x141298F60)(rage::grcTextureFactory::getInstance());

	hook::put<uint8_t>(0x141298F60, 0xC3);

	DXGI_SWAP_CHAIN_DESC desc = {};
	(*g_dxgiSwapChain)->GetDesc(&desc);

	trace("flags now: %d\n", desc.Flags);

	HRESULT hr = (*g_dxgiSwapChain)->ResizeBuffers(0, info->width, info->height, DXGI_FORMAT_UNKNOWN, desc.Flags);// | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);

	if (FAILED(hr))
	{
		TerminateProcess(GetCurrentProcess(), 1);
	}

	if (FAILED(hr))
	{
		trace("Changing video mode: buffers failed. HR: %08x\n", hr);
		// TODO: fail on D3D error
	}

	trace("Changing video mode: meheeeeeh.\n");

	((void(*)(void*))0x14129D1C8)(rage::grcTextureFactory::getInstance());

	trace("Woo! %016x\n", (uintptr_t)*g_backBufferRT);
#endif

	return success;
}

static bool(*g_origRunGame)();

static int RunGameWrap()
{
	return g_origRunGame();
}

void D3DPresent(int syncInterval, int flags)
{
	if (syncInterval == 0)
	{
		BOOL fullscreen;

		if (SUCCEEDED((*g_dxgiSwapChain)->GetFullscreenState(&fullscreen, nullptr)) && !fullscreen)
		{
			if (g_allowTearing)
			{
				flags |= DXGI_PRESENT_ALLOW_TEARING;
			}
		}
	}

	HRESULT hr = (*g_dxgiSwapChain)->Present(syncInterval, flags);

	if (FAILED(hr))
	{
		trace("IDXGISwapChain::Present failed: %08x\n", hr);
	}
}

static int Return1()
{
	return 1;
}

#include "dxerr.h"

static void DisplayD3DCrashMessage(HRESULT hr)
{
	wchar_t errorBuffer[16384];
	DXGetErrorDescriptionW(hr, errorBuffer, _countof(errorBuffer));

	FatalError("DirectX encountered an unrecoverable error: %s - %s", ToNarrow(DXGetErrorStringW(hr)), ToNarrow(errorBuffer));
}

static HRESULT D3DGetData(ID3D11DeviceContext* dc, ID3D11Asynchronous* async, void* data, UINT dataSize, UINT flags)
{
	*(int*)data = 1;

	return S_OK;
}

static void(*g_origPresent)();

void RagePresentWrap()
{
	InvokeRender();

	return g_origPresent();
}

static SRWLOCK g_textureOverridesLock = SRWLOCK_INIT;
static std::unordered_map<rage::grcTexture*, rage::grcTexture*> g_textureOverrides;

static void(*g_origSetTexture)(void* a1, void* a2, int index, rage::grcTexture* texture);

static void SetTextureHook(void* a1, void* a2, int index, rage::grcTexture* texture)
{
	if (texture)
	{
		if (!g_textureOverrides.empty())
		{
			AcquireSRWLockShared(&g_textureOverridesLock);

			auto it = g_textureOverrides.find(texture);

			if (it != g_textureOverrides.end())
			{
				texture = it->second;
			}

			ReleaseSRWLockShared(&g_textureOverridesLock);
		}
	}

	g_origSetTexture(a1, a2, index, texture);
}

static void(*g_origGrcTextureDtor)(void*);

static void grcTextureDtorHook(rage::grcTexture* self)
{
	RemoveTextureOverride(self);

	g_origGrcTextureDtor(self);
}

void AddTextureOverride(rage::grcTexture* orig, rage::grcTexture* repl)
{
	AcquireSRWLockExclusive(&g_textureOverridesLock);
	g_textureOverrides[orig] = repl;
	ReleaseSRWLockExclusive(&g_textureOverridesLock);
}

void RemoveTextureOverride(rage::grcTexture* orig)
{
	AcquireSRWLockExclusive(&g_textureOverridesLock);
	g_textureOverrides.erase(orig);
	ReleaseSRWLockExclusive(&g_textureOverridesLock);
}

static HookFunction hookFunction([] ()
{
	static ConVar<bool> disableRenderingCvar("r_disableRendering", ConVar_None, false, &g_disableRendering);

	// device creation
	void* ptrFunc = hook::pattern("E8 ? ? ? ? 84 C0 75 ? B2 01 B9 2F A9 C2 F4").count(1).get(0).get<void>(33);

	hook::set_call(&g_origCreateCB, ptrFunc);
	hook::call(ptrFunc, InvokeCreateCB);

	// end scene
	{
		auto location = hook::get_pattern("48 0F 45 C8 48 83 C4 40 5B E9 00 00 00 00", 9);
		hook::set_call(&g_origPresent, location);
		hook::jump(location, RagePresentWrap);
	}

	//ptrFunc = hook::pattern("83 64 24 28 00 41 B0 01 C6 44 24 20 01 41 8A C8").count(1).get(0).get<void>(-30);
	//hook::set_call(&g_origEndScene, ptrFunc);
	//hook::call(ptrFunc, EndSceneWrap);

	ptrFunc = hook::get_pattern("41 83 F9 01 75 34 48 83 C4 28 E9", 10);
	hook::set_call(&g_origRunGame, ptrFunc);
	hook::jump(ptrFunc, RunGameWrap);

	// set the present hook
	if (IsWindows10OrGreater())
	{
		// present hook function
		hook::put(hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? 48 85 C0 74 0C 8B 4D 50 8B", 3)), D3DPresent);

		// wrap video mode changing
		char* fnStart = hook::get_pattern<char>("8B 03 41 BE 01 00 00 00 89 05", -0x47);

		MH_CreateHook(fnStart, WrapVideoModeChange, (void**)&g_origVideoModeChange);
		MH_EnableHook(MH_ALL_HOOKS);

		g_dxgiSwapChain = hook::get_address<IDXGISwapChain**>(fnStart + 0x127);

		g_resetVideoMode = hook::get_pattern<std::remove_pointer_t<decltype(g_resetVideoMode)>>("8B 44 24 50 4C 8B 17 44 8B 4E 04 44 8B 06", -0x61);

		// remove render thread semaphore checks from buffer resizing
		hook::nop((char*)g_resetVideoMode + 0x48, 5);
		hook::nop((char*)g_resetVideoMode + 0x163, 5);
	}

	if (g_disableRendering)
	{
		hook::jump(hook::get_pattern("84 D2 0F 45 C7 8A D9 89 05", -0x1F), Return1);
	}

	// ignore frozen render device (for PIX and such)
	//hook::put<uint32_t>(hook::pattern("8B 8E C0 0F 00 00 68 88 13 00 00 51 E8").count(2).get(0).get<void>(7), INFINITE);

	/*ptrFunc = hook::pattern("8B 8E F0 09 00 00 E8 ? ? ? ? 68").count(1).get(0).get<void>(12);

	hook::put(ptrFunc, InvokePostFrontendRender);*/

	// frontend render phase
	//hook::put(0xE9F1AC, InvokeFrontendCBStub);

	// in-menu check for renderphasefrontend
	//*(BYTE*)0x43AF21 = 0xEB;

	// temp: d3d debug layer
	//static void* gFunc = D3D11CreateDeviceAndSwapChain2;
	//hook::put(0xF107CE, &gFunc);

	// add D3D11_CREATE_DEVICE_BGRA_SUPPORT flag
	void* createDeviceLoc = hook::pattern("48 8D 45 90 C7 44 24 30 07 00 00 00").count(1).get(0).get<void>(21);
	hook::nop(createDeviceLoc, 6);
	hook::call(createDeviceLoc, CreateD3D11DeviceWrap);

	// don't crash on ID3D11DeviceContext::GetData call failures
	// these somehow are caused by NVIDIA driver settings?
	hook::nop(hook::get_pattern("EB 0C 8B C8 E8 ? ? ? ? B8 01", 4), 5);

	// ERR_GFX_D3D_INIT: display valid reasons
	auto loc = hook::get_pattern<char>("75 0A B9 06 BD F7 9C E8");
	hook::nop(loc + 2, 5);
	hook::call(loc + 7, DisplayD3DCrashMessage);

	// texture overrides
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("C8 08 74 05 4C 89 4C C8 08 65 48 8B 0C 25", -0x15), SetTextureHook, (void**)&g_origSetTexture);
	MH_CreateHook(hook::get_pattern("48 8B D9 48 89 01 48 8B 49 28 E8 ? ? ? ? 48 8D", -0xD), grcTextureDtorHook, (void**)&g_origGrcTextureDtor);
	MH_EnableHook(MH_ALL_HOOKS);

	// query GetData, always return 1 (why even wait for presentation with a really weird Sleep loop?)
	{
		//auto location = hook::get_pattern("48 8B 01 8B FE FF 90 E8 00 00 00", 5);
		//hook::nop(location, 6);
		//hook::call(location, D3DGetData);
	}
});
