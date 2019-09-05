#include "StdInc.h"

//#include <ETWProviders/etwprof.h>

#include <mutex>

#include <d3d11_1.h>

#include <dxgi1_4.h>
#include <dxgi1_5.h>
#include <wrl.h>

#include "DrawCommands.h"
#include "Hooking.h"

#include <MinHook.h>

#include <Error.h>

#include <CoreConsole.h>

#include <HostSharedData.h>

namespace WRL = Microsoft::WRL;

fwEvent<> OnGrcCreateDevice;
fwEvent<> OnPostFrontendRender;

static bool g_overrideVsync;

static void(*g_origCreateCB)(const char*);

static void InvokeCreateCB(const char* arg)
{
	g_origCreateCB(arg);

	//OnGrcCreateDevice();
}

static void CaptureBufferOutput();

static void InvokeRender()
{
	static std::once_flag of;

	std::call_once(of, [] ()
	{
		OnGrcCreateDevice();
	});

	uintptr_t a1;
	uintptr_t a2;

	EnqueueGenericDrawCommand([](uintptr_t, uintptr_t)
	{
		CaptureBufferOutput();
	}, &a1, &a2);

	OnPostFrontendRender();
}

#pragma comment(lib, "d3d11.lib")

static IDXGISwapChain1* g_swapChain1;
static DWORD g_swapChainFlags;
static ID3D11DeviceContext* g_dc;

static bool g_allowTearing;
static bool g_disableRendering;

void MakeDummyDevice(ID3D11Device** device, ID3D11DeviceContext** context, const DXGI_SWAP_CHAIN_DESC* desc, IDXGISwapChain** swapChain);

fwEvent<IDXGIFactory2*, ID3D11Device*, HWND, DXGI_SWAP_CHAIN_DESC1*, DXGI_SWAP_CHAIN_FULLSCREEN_DESC*, IDXGISwapChain1**> OnTryCreateSwapChain;

static HRESULT CreateD3D11DeviceWrap(_In_opt_ IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, _In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, _In_opt_ CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, _Out_opt_ IDXGISwapChain** ppSwapChain, _Out_opt_ ID3D11Device** ppDevice, _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel, _Out_opt_ ID3D11DeviceContext** ppImmediateContext)
{
	auto uiExitEvent = CreateEvent(NULL, FALSE, FALSE, L"CitizenFX_PreUIExit");
	auto uiDoneEvent = CreateEvent(NULL, FALSE, FALSE, L"CitizenFX_PreUIDone");

	if (uiExitEvent)
	{
		SetEvent(uiExitEvent);
	}

	if (uiDoneEvent)
	{
		WaitForSingleObject(uiDoneEvent, INFINITE);
	}

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

		OnTryCreateSwapChain(dxgiFactory.Get(), *ppDevice, pSwapChainDesc->OutputWindow, &scDesc1, &fsDesc, &swapChain1);

		if (!swapChain1)
		{
			dxgiFactory->CreateSwapChainForHwnd(*ppDevice, pSwapChainDesc->OutputWindow, &scDesc1, &fsDesc, nullptr, &swapChain1);
		}

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
		ID3D11ShaderResourceView* m_srv2;
		void* m_pad2;
		ID3D11Resource* m_resource2;
		atArray<ID3D11ShaderResourceView*> m_rtvs;
	};
}

static bool(*g_resetVideoMode)(VideoModeInfo*);

static ID3D11RenderTargetView* g_rtv;
static ID3D11Texture2D* g_myTexture;

void(*g_origCreateBackbuffer)(void*);

void WrapCreateBackbuffer(void* tf)
{
	trace("Creating backbuffer.\n");

	g_origCreateBackbuffer(tf);

	trace("Done creating backbuffer.\n");
}

bool WrapVideoModeChange(VideoModeInfo* info)
{
	trace("Changing video mode.\n");

	if (g_rtv)
	{
		g_rtv->Release();
		g_rtv = nullptr;
	}

	if (g_myTexture)
	{
		g_myTexture->Release();
		g_myTexture = nullptr;
	}

	bool success = g_origVideoModeChange(info);

	trace("Changing video mode success: %d.\n", success);

	if (success)
	{
		//g_resetVideoMode(info);
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

#pragma region shaders
const BYTE quadPS[] =
{
	 68,  88,  66,  67, 189,  87,
	  5, 130, 168, 148, 229, 231,
	171,  37, 224,   4, 165,  41,
	 28,  80,   1,   0,   0,   0,
	 84,   1,   0,   0,   3,   0,
	  0,   0,  44,   0,   0,   0,
	132,   0,   0,   0, 184,   0,
	  0,   0,  73,  83,  71,  78,
	 80,   0,   0,   0,   2,   0,
	  0,   0,   8,   0,   0,   0,
	 56,   0,   0,   0,   0,   0,
	  0,   0,   1,   0,   0,   0,
	  3,   0,   0,   0,   0,   0,
	  0,   0,  15,   0,   0,   0,
	 68,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,
	  3,   0,   0,   0,   1,   0,
	  0,   0,   3,   3,   0,   0,
	 83,  86,  95,  80,  79,  83,
	 73,  84,  73,  79,  78,   0,
	 84,  69,  88,  67,  79,  79,
	 82,  68,   0, 171, 171, 171,
	 79,  83,  71,  78,  44,   0,
	  0,   0,   1,   0,   0,   0,
	  8,   0,   0,   0,  32,   0,
	  0,   0,   0,   0,   0,   0,
	  0,   0,   0,   0,   3,   0,
	  0,   0,   0,   0,   0,   0,
	 15,   0,   0,   0,  83,  86,
	 95,  84,  65,  82,  71,  69,
	 84,   0, 171, 171,  83,  72,
	 68,  82, 148,   0,   0,   0,
	 64,   0,   0,   0,  37,   0,
	  0,   0,  90,   0,   0,   3,
	  0,  96,  16,   0,   0,   0,
	  0,   0,  88,  24,   0,   4,
	  0, 112,  16,   0,   0,   0,
	  0,   0,  85,  85,   0,   0,
	 98,  16,   0,   3,  50,  16,
	 16,   0,   1,   0,   0,   0,
	101,   0,   0,   3, 242,  32,
	 16,   0,   0,   0,   0,   0,
	104,   0,   0,   2,   1,   0,
	  0,   0,  69,   0,   0,   9,
	242,   0,  16,   0,   0,   0,
	  0,   0,  70,  16,  16,   0,
	  1,   0,   0,   0,  70, 126,
	 16,   0,   0,   0,   0,   0,
	  0,  96,  16,   0,   0,   0,
	  0,   0,  54,   0,   0,   5,
	114,  32,  16,   0,   0,   0,
	  0,   0,  70,   2,  16,   0,
	  0,   0,   0,   0,  54,   0,
	  0,   5, 130,  32,  16,   0,
	  0,   0,   0,   0,   1,  64,
	  0,   0,   0,   0, 128,  63,
	 62,   0,   0,   1
};
const BYTE quadVS[] =
{
	68,  88,  66,  67, 203, 141,
	78, 146,   5, 246, 239, 246,
	166,  36, 242, 232,  80,   1,
	231, 115,   1,   0,   0,   0,
	208,   2,   0,   0,   5,   0,
	0,   0,  52,   0,   0,   0,
	128,   0,   0,   0, 180,   0,
	0,   0,  12,   1,   0,   0,
	84,   2,   0,   0,  82,  68,
	69,  70,  68,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	28,   0,   0,   0,   0,   4,
	254, 255,   0,   1,   0,   0,
	28,   0,   0,   0,  77, 105,
	99, 114, 111, 115, 111, 102,
	116,  32,  40,  82,  41,  32,
	72,  76,  83,  76,  32,  83,
	104,  97, 100, 101, 114,  32,
	67, 111, 109, 112, 105, 108,
	101, 114,  32,  49,  48,  46,
	49,   0,  73,  83,  71,  78,
	44,   0,   0,   0,   1,   0,
	0,   0,   8,   0,   0,   0,
	32,   0,   0,   0,   0,   0,
	0,   0,   6,   0,   0,   0,
	1,   0,   0,   0,   0,   0,
	0,   0,   1,   1,   0,   0,
	83,  86,  95,  86,  69,  82,
	84,  69,  88,  73,  68,   0,
	79,  83,  71,  78,  80,   0,
	0,   0,   2,   0,   0,   0,
	8,   0,   0,   0,  56,   0,
	0,   0,   0,   0,   0,   0,
	1,   0,   0,   0,   3,   0,
	0,   0,   0,   0,   0,   0,
	15,   0,   0,   0,  68,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   3,   0,
	0,   0,   1,   0,   0,   0,
	3,  12,   0,   0,  83,  86,
	95,  80,  79,  83,  73,  84,
	73,  79,  78,   0,  84,  69,
	88,  67,  79,  79,  82,  68,
	0, 171, 171, 171,  83,  72,
	68,  82,  64,   1,   0,   0,
	64,   0,   1,   0,  80,   0,
	0,   0,  96,   0,   0,   4,
	18,  16,  16,   0,   0,   0,
	0,   0,   6,   0,   0,   0,
	103,   0,   0,   4, 242,  32,
	16,   0,   0,   0,   0,   0,
	1,   0,   0,   0, 101,   0,
	0,   3,  50,  32,  16,   0,
	1,   0,   0,   0, 104,   0,
	0,   2,   2,   0,   0,   0,
	54,   0,   0,   8, 194,  32,
	16,   0,   0,   0,   0,   0,
	2,  64,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	128,  63,   1,   0,   0,   7,
	18,   0,  16,   0,   0,   0,
	0,   0,  10,  16,  16,   0,
	0,   0,   0,   0,   1,  64,
	0,   0,   1,   0,   0,   0,
	85,   0,   0,   7, 130,   0,
	16,   0,   0,   0,   0,   0,
	10,  16,  16,   0,   0,   0,
	0,   0,   1,  64,   0,   0,
	1,   0,   0,   0,  86,   0,
	0,   5,  50,   0,  16,   0,
	0,   0,   0,   0, 198,   0,
	16,   0,   0,   0,   0,   0,
	0,   0,   0,  10,  50,   0,
	16,   0,   1,   0,   0,   0,
	70,   0,  16,   0,   0,   0,
	0,   0,   2,  64,   0,   0,
	0,   0,   0, 191,   0,   0,
	0, 191,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   8,  66,   0,  16,   0,
	0,   0,   0,   0,  26,   0,
	16, 128,  65,   0,   0,   0,
	0,   0,   0,   0,   1,  64,
	0,   0,   0,   0, 128,  63,
	54,   0,   0,   5,  50,  32,
	16,   0,   1,   0,   0,   0,
	134,   0,  16,   0,   0,   0,
	0,   0,   0,   0,   0,   7,
	18,  32,  16,   0,   0,   0,
	0,   0,  10,   0,  16,   0,
	1,   0,   0,   0,  10,   0,
	16,   0,   1,   0,   0,   0,
	56,   0,   0,   7,  34,  32,
	16,   0,   0,   0,   0,   0,
	26,   0,  16,   0,   1,   0,
	0,   0,   1,  64,   0,   0,
	0,   0,   0, 192,  62,   0,
	0,   1,  83,  84,  65,  84,
	116,   0,   0,   0,  10,   0,
	0,   0,   2,   0,   0,   0,
	0,   0,   0,   0,   3,   0,
	0,   0,   4,   0,   0,   0,
	0,   0,   0,   0,   2,   0,
	0,   0,   1,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   2,   0,   0,   0,
	0,   0,   0,   0,   1,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0
};
#pragma endregion

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

void CaptureBufferOutput()
{
	static HostSharedData<GameRenderData> handleData("CfxGameRenderHandle");

	static D3D11_TEXTURE2D_DESC resDesc;

	// 1365
	// 1604
	rage::grcRenderTargetDX11* backBuf = *(rage::grcRenderTargetDX11**)hook::get_adjusted(0x142AD7A88);

	if (backBuf)
	{
		if (backBuf->texture)
		{
			((ID3D11Texture2D*)backBuf->texture)->GetDesc(&resDesc);

			handleData->width = resDesc.Width;
			handleData->height = resDesc.Height;
		}
	}

	if (!g_myTexture)
	{
		D3D11_TEXTURE2D_DESC texDesc = { 0 };
		texDesc.Width = resDesc.Width;
		texDesc.Height = resDesc.Height;
		texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

		WRL::ComPtr<ID3D11Texture2D> d3dTex;
		HRESULT hr = GetD3D11Device()->CreateTexture2D(&texDesc, nullptr, &d3dTex);
		if FAILED(hr)
		{
			// error handling code
		}

		D3D11_RENDER_TARGET_VIEW_DESC rtDesc = CD3D11_RENDER_TARGET_VIEW_DESC(d3dTex.Get(), D3D11_RTV_DIMENSION_TEXTURE2D);
		GetD3D11Device()->CreateRenderTargetView(d3dTex.Get(), &rtDesc, &g_rtv);

		d3dTex.CopyTo(&g_myTexture);

		WRL::ComPtr<IDXGIResource> dxgiResource;
		HANDLE sharedHandle;
		hr = d3dTex.As(&dxgiResource);
		if (FAILED(hr))
		{
			// error handling code
			return;
		}

		hr = dxgiResource->GetSharedHandle(&sharedHandle);
		if FAILED(hr)
		{
			// error handling code
		}

		handleData->handle = sharedHandle;
	}

	if (!handleData->requested)
	{
		return;
	}

	// guess what we can't just CopyResource, so time for copy/pasted D3D11 garbage
	{
		auto m_width = resDesc.Width;
		auto m_height = resDesc.Height;

		//
		// LOTS of D3D11 garbage to flip a texture...
		//
		static ID3D11BlendState* bs;
		static ID3D11SamplerState* ss;
		static ID3D11VertexShader* vs;
		static ID3D11PixelShader* ps;

		static std::once_flag of;
		std::call_once(of, []()
		{
			D3D11_SAMPLER_DESC sd = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
			GetD3D11Device()->CreateSamplerState(&sd, &ss);

			D3D11_BLEND_DESC bd = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
			bd.RenderTarget[0].BlendEnable = FALSE;

			GetD3D11Device()->CreateBlendState(&bd, &bs);

			GetD3D11Device()->CreateVertexShader(quadVS, sizeof(quadVS), nullptr, &vs);
			GetD3D11Device()->CreatePixelShader(quadPS, sizeof(quadPS), nullptr, &ps);
		});

		ID3DUserDefinedAnnotation* pPerf;
		GetD3D11DeviceContext()->QueryInterface(__uuidof(pPerf), reinterpret_cast<void**>(&pPerf));

		pPerf->BeginEvent(L"DrawRenderTexture");

		auto deviceContext = GetD3D11DeviceContext();

		ID3D11RenderTargetView* oldRtv = nullptr;
		ID3D11DepthStencilView* oldDsv = nullptr;
		deviceContext->OMGetRenderTargets(1, &oldRtv, &oldDsv);

		ID3D11SamplerState* oldSs;
		ID3D11BlendState* oldBs;
		ID3D11PixelShader* oldPs;
		ID3D11VertexShader* oldVs;
		ID3D11ShaderResourceView* oldSrv;

		D3D11_VIEWPORT oldVp;
		UINT numVPs = 1;

		deviceContext->RSGetViewports(&numVPs, &oldVp);

		CD3D11_VIEWPORT vp = CD3D11_VIEWPORT(0.0f, 0.0f, m_width, m_height);
		deviceContext->RSSetViewports(1, &vp);

		deviceContext->OMGetBlendState(&oldBs, nullptr, nullptr);

		deviceContext->PSGetShader(&oldPs, nullptr, nullptr);
		deviceContext->PSGetSamplers(0, 1, &oldSs);
		deviceContext->PSGetShaderResources(0, 1, &oldSrv);

		deviceContext->VSGetShader(&oldVs, nullptr, nullptr);

		deviceContext->OMSetRenderTargets(1, &g_rtv, nullptr);
		deviceContext->OMSetBlendState(bs, nullptr, 0xffffffff);

		deviceContext->PSSetShader(ps, nullptr, 0);
		deviceContext->PSSetSamplers(0, 1, &ss);
		deviceContext->PSSetShaderResources(0, 1, &backBuf->m_srv2);

		deviceContext->VSSetShader(vs, nullptr, 0);

		D3D11_PRIMITIVE_TOPOLOGY oldTopo;
		deviceContext->IAGetPrimitiveTopology(&oldTopo);

		ID3D11InputLayout* oldLayout;
		deviceContext->IAGetInputLayout(&oldLayout);

		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		deviceContext->IASetInputLayout(nullptr);

		FLOAT blank[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		deviceContext->ClearRenderTargetView(g_rtv, blank);

		deviceContext->Draw(4, 0);

		deviceContext->OMSetRenderTargets(1, &oldRtv, oldDsv);

		deviceContext->IASetPrimitiveTopology(oldTopo);
		deviceContext->IASetInputLayout(oldLayout);

		deviceContext->VSSetShader(oldVs, nullptr, 0);
		deviceContext->PSSetShader(oldPs, nullptr, 0);
		deviceContext->PSSetSamplers(0, 1, &oldSs);
		deviceContext->PSSetShaderResources(0, 1, &oldSrv);
		deviceContext->OMSetBlendState(oldBs, nullptr, 0xffffffff);
		deviceContext->RSSetViewports(1, &oldVp);

		if (oldVs)
		{
			oldVs->Release();
		}

		if (oldPs)
		{
			oldPs->Release();
		}

		if (oldBs)
		{
			oldBs->Release();
		}

		if (oldSs)
		{
			oldSs->Release();
		}

		if (oldSrv)
		{
			oldSrv->Release();
		}

		if (oldRtv)
		{
			oldRtv->Release();
		}

		if (oldDsv)
		{
			oldDsv->Release();
		}

		if (oldLayout)
		{
			oldLayout->Release();
		}

		pPerf->EndEvent();

		pPerf->Release();
	}
}

void D3DPresent(int syncInterval, int flags)
{
	if (g_overrideVsync)
	{
		syncInterval = 1;
	}

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

void GfxForceVsync(bool enabled)
{
	g_overrideVsync = enabled;
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
		MH_CreateHook(hook::get_pattern("57 48 83 EC 20 49 83 63 08 00", -0xB), WrapCreateBackbuffer, (void**)&g_origCreateBackbuffer);
		MH_EnableHook(MH_ALL_HOOKS);

		g_dxgiSwapChain = hook::get_address<IDXGISwapChain**>(fnStart + 0x127);

		g_resetVideoMode = hook::get_pattern<std::remove_pointer_t<decltype(g_resetVideoMode)>>("8B 44 24 50 4C 8B 17 44 8B 4E 04 44 8B 06", -0x61);

		// remove render thread semaphore checks from buffer resizing
		/*hook::nop((char*)g_resetVideoMode + 0x48, 5);
		hook::nop((char*)g_resetVideoMode + 0x163, 5);*/
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
