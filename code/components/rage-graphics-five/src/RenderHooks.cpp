#include "StdInc.h"

#include <mutex>

#include <d3d11_1.h>

#include <dxgi1_4.h>
#include <dxgi1_5.h>
#include <wrl.h>

#include "DrawCommands.h"
#include "Hooking.h"

#include <MinHook.h>

#include <Error.h>

#include <CrossBuildRuntime.h>
#include <LaunchMode.h>
#include <CL2LaunchMode.h>

#include <CoreConsole.h>
#include <ICoreGameInit.h>

#include <HostSharedData.h>

namespace WRL = Microsoft::WRL;

void WakeWindowThreadFor(std::function<void()>&& func);

fwEvent<> OnGrcCreateDevice;
fwEvent<> OnPostFrontendRender;

DLL_EXPORT fwEvent<bool*> OnFlipModelHook;

fwEvent<bool*> OnRequestInternalScreenshot;
fwEvent<const uint8_t*, int, int> OnInternalScreenshot;

struct WaitableTimer {
	WaitableTimer(
		_In_opt_ LPSECURITY_ATTRIBUTES lpTimerAttributes,
		_In_ BOOL bManualReset,
		_In_opt_ LPCWSTR lpTimerName
	) : handle(CreateWaitableTimer(lpTimerAttributes, bManualReset, lpTimerName)) {}
	~WaitableTimer() { CloseHandle(handle); }

	template<class Rep, class Period = std::ratio<1>>
	BOOL Wait(std::chrono::duration<Rep, Period> dueTime)
	{
		LARGE_INTEGER liDueTime;

		liDueTime.QuadPart = -(LONGLONG)(std::chrono::duration_cast<std::chrono::microseconds>(dueTime).count() * 10);

		if (handle && SetWaitableTimer(handle, &liDueTime, 0, NULL, NULL, 0))
		{
			WaitForSingleObject(handle, INFINITE);
			return TRUE;
		}

		return FALSE;
	}
private:
	HANDLE handle;
};

static void* g_lastBackbufTexture;
static bool g_useFlipModel = false;

static bool g_overrideVsync;

static void CaptureBufferOutput();
static void CaptureInternalScreenshot();

static hook::cdecl_stub<void()> flushRenderStates([]()
{
	return hook::get_pattern("F6 C2 01 74 30 8B", -10);
});

static hook::cdecl_stub<void(bool)> _grcLighting([]()
{
	return hook::get_call(hook::get_pattern("48 83 EC 38 B1 01 E8 ? ? ? ? 48 8B", 6));
});

static hook::cdecl_stub<void(rage::grcTexture*)> _grcBindTexture([]()
{
	return hook::get_call(hook::get_pattern("48 8B D9 33 C9 E8 ? ? ? ? E8 ? ? ? ? 33 C9 E8", 5));
});

static hook::cdecl_stub<void()> _grcWorldIdentity([]()
{
	return hook::get_call(hook::get_pattern("48 8B D9 33 C9 E8 ? ? ? ? E8 ? ? ? ? 33 C9 E8", 10));
});

namespace rage
{
void grcLighting(bool enable)
{
	return _grcLighting(enable);
}

void grcBindTexture(rage::grcTexture* texture)
{
	return _grcBindTexture(texture);
}

void grcWorldIdentity()
{
	return _grcWorldIdentity();
}

struct grcViewport
{
};

struct spdViewport : grcViewport
{
	static spdViewport* GetCurrent();
};
}

static rage::spdViewport** rage__spdViewport__sm_Current;

rage::spdViewport* rage::spdViewport::GetCurrent()
{
	return *rage__spdViewport__sm_Current;
}

static HookFunction hookFunctionSafe([]()
{
	rage__spdViewport__sm_Current = hook::get_address<rage::spdViewport**>(hook::get_pattern("48 8B 3D ? ? ? ? 40 8A F2 48 8B D9 75 14", 3));
});

static void InvokeRender()
{
	static std::once_flag of;

	std::call_once(of, [] ()
	{
		OnGrcCreateDevice();
	});

	SetBlendState(GetStockStateIdentifier(BlendStateDefault));
	SetDepthStencilState(GetStockStateIdentifier(DepthStencilStateNoDepth));
	SetRasterizerState(GetStockStateIdentifier(RasterizerStateNoCulling));

	rage::grcLighting(false);

	if (rage::spdViewport::GetCurrent())
	{
		rage::grcWorldIdentity();
	}

	rage::grcBindTexture(nullptr);

	flushRenderStates();

	// run a no-op draw call to flush other grcore states
	rage::grcBegin(3, 3);
	rage::grcVertex(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0.0f, 0.0f);
	rage::grcVertex(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0.0f, 0.0f);
	rage::grcVertex(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0, 0.0f, 0.0f);
	rage::grcEnd();

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

#include <HostSharedData.h>
#include <ReverseGameData.h>
#include <CfxState.h>

#include <dcomp.h>

#pragma comment(lib, "dcomp.lib")

#include <mmsystem.h>

static auto LimitFrameTime(WaitableTimer& timer, size_t fpsLimit)
{
	using namespace std::chrono_literals;

	static auto getNowUs = []()
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch());
	};
	static std::chrono::microseconds lastFrameTime{ 0 };

	struct ReturnToken
	{
		~ReturnToken()
		{
			lastFrameTime = getNowUs();
		}

		ReturnToken(const ReturnToken&) = delete;
	};

	if (fpsLimit > 0)
	{
		std::chrono::microseconds fpsLimitUs{ 1000000 / fpsLimit };

		auto timeLeft = std::chrono::duration_cast<std::chrono::microseconds>(fpsLimitUs - (getNowUs() - lastFrameTime));

		if (timeLeft > 0us)
		{
			timer.Wait(timeLeft);
		}
	}

	return ReturnToken{};
}

class BufferBackedDXGISwapChain : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IDXGISwapChain>
{
public:
	// Inherited via RuntimeClass
	virtual HRESULT SetPrivateData(REFGUID Name, UINT DataSize, const void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT SetPrivateDataInterface(REFGUID Name, const IUnknown * pUnknown) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT GetPrivateData(REFGUID Name, UINT * pDataSize, void * pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT GetParent(REFIID riid, void ** ppParent) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT GetDevice(REFIID riid, void ** ppDevice) override
	{
		trace("GetDevice\n");

		return E_NOTIMPL;
	}
	virtual HRESULT Present(UINT SyncInterval, UINT Flags) override
	{
		if (Flags & DXGI_PRESENT_TEST)
		{
			// TODO: request more information from the device/host game
			return S_OK;
		}

		static HostSharedData<ReverseGameData> rgd("CfxReverseGameData");

		GetD3D11DeviceContext()->Flush();

		auto _ = LimitFrameTime(m_fpsLimitTimer, rgd->fpsLimit);

		int idx = rgd->GetNextSurface(INFINITE);

		if (idx >= 0)
		{
			auto surface = m_frontTextures[idx];
			WRL::ComPtr<IDXGIKeyedMutex> mutex;

			GetD3D11DeviceContext()->CopyResource(surface.Get(), m_texture.Get());
			rgd->SubmitSurface();
		}
		else
		{
			trace("frame dropped - presenter was busy?\n");
		}

		return S_OK;
	}
	virtual HRESULT GetBuffer(UINT Buffer, REFIID riid, void ** ppSurface) override
	{
		//trace("GetBuffer %d\n", Buffer);

		if (Buffer == 0)
		{
			return m_texture.CopyTo(riid, ppSurface);
		}

		return E_NOTIMPL;
	}
	virtual HRESULT SetFullscreenState(BOOL Fullscreen, IDXGIOutput * pTarget) override
	{
		//trace("SetFullscreenState %d\n", Fullscreen);

		return S_OK;
	}
	virtual HRESULT GetFullscreenState(BOOL * pFullscreen, IDXGIOutput ** ppTarget) override
	{
		//trace("GetFullscreenState\n");

		*pFullscreen = FALSE;
		return S_OK;
	}

private:
	WRL::ComPtr<ID3D11Texture2D> m_texture;
	WRL::ComPtr<ID3D11Texture2D> m_frontTextures[4];
	WaitableTimer m_fpsLimitTimer;

public:
	DXGI_SWAP_CHAIN_DESC desc;
	ID3D11Device* device;

	BufferBackedDXGISwapChain(ID3D11Device* device, DXGI_SWAP_CHAIN_DESC desc)
		: desc(desc), device(device), m_fpsLimitTimer(NULL, TRUE, NULL)
	{
		RecreateFromDesc();
	}

private:
	void RecreateFromDesc()
	{
		D3D11_TEXTURE2D_DESC tdesc = CD3D11_TEXTURE2D_DESC(desc.BufferDesc.Format, desc.BufferDesc.Width, desc.BufferDesc.Height, 1, 1, D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT, 0, 1, 0, /*D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX*/ D3D11_RESOURCE_MISC_SHARED);

		static HostSharedData<ReverseGameData> rgd("CfxReverseGameData");

		if (rgd->twidth != desc.BufferDesc.Width || rgd->theight != desc.BufferDesc.Height)
		{
			rgd->twidth = desc.BufferDesc.Width;
			rgd->theight = desc.BufferDesc.Height;

			rgd->editWidth = true;
		}

		for (int i = 0; i < std::size(m_frontTextures); i++)
		{
			WRL::ComPtr<ID3D11Texture2D> tex;
			device->CreateTexture2D(&tdesc, nullptr, tex.ReleaseAndGetAddressOf());

			m_frontTextures[i] = tex;

			WRL::ComPtr<IDXGIResource> res;

			if (SUCCEEDED(tex.As(&res)))
			{
				HANDLE hdl;
				res->GetSharedHandle(&hdl);

				rgd->surfaces[i] = hdl;
			}
		}

		tdesc.MiscFlags &= ~D3D11_RESOURCE_MISC_SHARED;
		tdesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
		device->CreateTexture2D(&tdesc, nullptr, m_texture.ReleaseAndGetAddressOf());

		if (rgd->inited)
		{
			rgd->createHandles = true;
		}

		rgd->inited = true;
	}

public:

	virtual HRESULT GetDesc(DXGI_SWAP_CHAIN_DESC * pDesc) override
	{
		*pDesc = desc;
		return S_OK;
	}
	virtual HRESULT ResizeBuffers(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) override
	{
		desc.BufferCount = BufferCount;
		desc.BufferDesc.Width = Width;
		desc.BufferDesc.Height = Height;
		desc.BufferDesc.Format = NewFormat;
		desc.Flags = SwapChainFlags;

		RecreateFromDesc();

		return S_OK;
	}
	virtual HRESULT ResizeTarget(const DXGI_MODE_DESC * pNewTargetParameters) override
	{
		return S_OK;
	}
	virtual HRESULT GetContainingOutput(IDXGIOutput ** ppOutput) override
	{
		return S_OK;
	}
	virtual HRESULT GetFrameStatistics(DXGI_FRAME_STATISTICS * pStats) override
	{
		return S_OK;
	}
	virtual HRESULT GetLastPresentCount(UINT * pLastPresentCount) override
	{
		return S_OK;
	}
};

class DeferredFullscreenSwapChain : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IDXGISwapChain>
{
	WRL::ComPtr<IDXGISwapChain> m_orig;
	bool m_defer = true;

public:
	DeferredFullscreenSwapChain(WRL::ComPtr<IDXGISwapChain> swapChain)
		: m_orig(swapChain)
	{
	
	}

	virtual HRESULT __stdcall SetPrivateData(REFGUID Name, UINT DataSize, const void* pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall SetPrivateDataInterface(REFGUID Name, const IUnknown* pUnknown) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetPrivateData(REFGUID Name, UINT* pDataSize, void* pData) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetParent(REFIID riid, void** ppParent) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetDevice(REFIID riid, void** ppDevice) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall Present(UINT SyncInterval, UINT Flags) override
	{
		return m_orig->Present(SyncInterval, Flags);
	}
	virtual HRESULT __stdcall GetBuffer(UINT Buffer, REFIID riid, void** ppSurface) override
	{
		return m_orig->GetBuffer(Buffer, riid, ppSurface);
	}
	virtual HRESULT __stdcall SetFullscreenState(BOOL Fullscreen, IDXGIOutput* pTarget) override
	{
		if (m_defer && !Fullscreen)
		{
			m_defer = false;
		}

		return m_orig->SetFullscreenState(Fullscreen, pTarget);
	}
	virtual HRESULT __stdcall GetFullscreenState(BOOL* pFullscreen, IDXGIOutput** ppTarget) override
	{
		auto rv = m_orig->GetFullscreenState(pFullscreen, ppTarget);

		if (SUCCEEDED(rv))
		{
			if (m_defer)
			{
				if (!*pFullscreen)
				{
					WRL::ComPtr<IDXGIOutput> output;
					
					if (SUCCEEDED(m_orig->GetContainingOutput(&output)) && output.Get())
					{
						m_orig->SetFullscreenState(TRUE, output.Get());

						if (ppTarget)
						{
							output.CopyTo(ppTarget);
						}

						*pFullscreen = true;
					}
				}
				else
				{
					m_defer = false;
				}
			}
		}

		return rv;
	}
	virtual HRESULT __stdcall GetDesc(DXGI_SWAP_CHAIN_DESC* pDesc) override
	{
		return m_orig->GetDesc(pDesc);
	}
	virtual HRESULT __stdcall ResizeBuffers(UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) override
	{
		return m_orig->ResizeBuffers(BufferCount, Width, Height, NewFormat, SwapChainFlags);
	}
	virtual HRESULT __stdcall ResizeTarget(const DXGI_MODE_DESC* pNewTargetParameters) override
	{
		return m_orig->ResizeTarget(pNewTargetParameters);
	}
	virtual HRESULT __stdcall GetContainingOutput(IDXGIOutput** ppOutput) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetFrameStatistics(DXGI_FRAME_STATISTICS* pStats) override
	{
		return E_NOTIMPL;
	}
	virtual HRESULT __stdcall GetLastPresentCount(UINT* pLastPresentCount) override
	{
		return E_NOTIMPL;
	}
};

#include <dxgi1_6.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

static HANDLE g_gameWindowEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

void DLL_EXPORT UiDone()
{
	static HostSharedData<CfxState> initState("CfxInitState");
	WaitForSingleObject(g_gameWindowEvent, INFINITE);

	auto uiExitEvent = CreateEventW(NULL, TRUE, FALSE, va(L"CitizenFX_PreUIExit%s", IsCL2() ? L"CL2" : L""));
	auto uiDoneEvent = CreateEventW(NULL, FALSE, FALSE, va(L"CitizenFX_PreUIDone%s", IsCL2() ? L"CL2" : L""));

	if (uiExitEvent)
	{
		SetEvent(uiExitEvent);
	}

	if (uiDoneEvent && !g_disableRendering)
	{
		WaitForSingleObject(uiDoneEvent, INFINITE);
	}
}

// graphics mods don't usually block calls from Windows DLLs to themselves in APIs they've hooked
// and therefore break EnsureChildDevice API
//
// as such, we'll block our forced DXGI use if we found any such library in the game folder
static bool IsSafeToUseDXGI()
{
	if (GetFileAttributesW(MakeRelativeGamePath(L"d3d11.dll").c_str()) != INVALID_FILE_ATTRIBUTES ||
		GetFileAttributesW(MakeRelativeGamePath(L"dxgi.dll").c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}

	return true;
}

extern HRESULT RootD3D11CreateDevice(_In_opt_ IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, _In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, _COM_Outptr_opt_ ID3D11Device** ppDevice, _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel, _COM_Outptr_opt_ ID3D11DeviceContext** ppImmediateContext);

static void GoGetAdapter(IDXGIAdapter** ppAdapter)
{
	{
		WRL::ComPtr<IDXGIFactory1> dxgiFactory;
		CreateDXGIFactory1(IID_IDXGIFactory1, &dxgiFactory);

		WRL::ComPtr<IDXGIAdapter1> adapter;
		WRL::ComPtr<IDXGIFactory6> factory6;
		HRESULT hr = dxgiFactory.As(&factory6);
		if (SUCCEEDED(hr))
		{
			for (UINT adapterIndex = 0;
				 DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf()));
				 adapterIndex++)
			{
				DXGI_ADAPTER_DESC1 desc;
				adapter->GetDesc1(&desc);

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				{
					// Don't select the Basic Render Driver adapter.
					continue;
				}

				static auto _ = ([&desc]
				{
					AddCrashometry("gpu_name", "%s", ToNarrow(desc.Description));
					AddCrashometry("gpu_id", "%04x:%04x", desc.VendorId, desc.DeviceId);
					return true;
				})();

				adapter.CopyTo(ppAdapter);
				break;
			}
		}
	}
}

static HRESULT CreateD3D11DeviceWrapOrig(_In_opt_ IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, _In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, _In_opt_ CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, _Out_opt_ IDXGISwapChain** ppSwapChain, _Out_opt_ ID3D11Device** ppDevice, _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel, _Out_opt_ ID3D11DeviceContext** ppImmediateContext)
{
	GoGetAdapter(&pAdapter);

	SetEvent(g_gameWindowEvent);

	if (g_disableRendering)
	{
		*pFeatureLevel = D3D_FEATURE_LEVEL_11_0;

		MakeDummyDevice(ppDevice, ppImmediateContext, pSwapChainDesc, ppSwapChain);

		return S_OK;
	}

	if (!IsWindows10OrGreater())
	{
		return D3D11CreateDeviceAndSwapChain(/*pAdapter*/ pAdapter, /*DriverType*/ pAdapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE, Software, Flags | D3D11_CREATE_DEVICE_BGRA_SUPPORT, pFeatureLevels, FeatureLevels /*nullptr, 0*/, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);
	}

	OnFlipModelHook(&g_useFlipModel);

	HRESULT hr = RootD3D11CreateDevice(pAdapter, /*DriverType*/ pAdapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE, Software, Flags | D3D11_CREATE_DEVICE_BGRA_SUPPORT/* | D3D11_CREATE_DEVICE_DEBUG*/, pFeatureLevels, FeatureLevels/*nullptr, 0*/, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);

	WRL::ComPtr<IDXGIFactory2> dxgiFactory;

	WRL::ComPtr<IDXGIDevice> dxgiDevice;
	WRL::ComPtr<IDXGIAdapter> dxgiAdapter;

	static HostSharedData<CfxState> initState("CfxInitState");

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

		if (g_useFlipModel)
		{
			// probe if DXGI 1.5 is available (Win10 RS1+)
			WRL::ComPtr<IDXGIFactory5> factory5;

			if (SUCCEEDED(dxgiFactory.As(&factory5)))
			{
				scDesc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
				scDesc1.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
				g_allowTearing = true;
			}
		}

		g_swapChainFlags = scDesc1.Flags;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsDesc = { 0 };
		fsDesc.RefreshRate = pSwapChainDesc->BufferDesc.RefreshRate;
		fsDesc.Scaling = pSwapChainDesc->BufferDesc.Scaling;
		fsDesc.ScanlineOrdering = pSwapChainDesc->BufferDesc.ScanlineOrdering;
		fsDesc.Windowed = pSwapChainDesc->Windowed;

		if (!fsDesc.Windowed)
		{
			fsDesc.Windowed = true;
		}

		OnTryCreateSwapChain(dxgiFactory.Get(), *ppDevice, pSwapChainDesc->OutputWindow, &scDesc1, &fsDesc, &swapChain1);

		if (!swapChain1)
		{
			if (!initState->isReverseGame)
			{
				dxgiFactory->CreateSwapChainForHwnd(*ppDevice, pSwapChainDesc->OutputWindow, &scDesc1, &fsDesc, nullptr, &swapChain1);
				swapChain1->QueryInterface(__uuidof(IDXGISwapChain), (void**)ppSwapChain);
			}
		}

		if (initState->isReverseGame)
		{
			auto sc = WRL::Make<BufferBackedDXGISwapChain>(*ppDevice, *pSwapChainDesc);
			sc.CopyTo(ppSwapChain);
		}
		else if (!pSwapChainDesc->Windowed && IsSafeToUseDXGI())
		{
			auto sc = WRL::Make<DeferredFullscreenSwapChain>(*ppSwapChain);

			// release the original swapchain
			(*ppSwapChain)->Release();
			*ppSwapChain = NULL;

			sc.CopyTo(ppSwapChain);
		}
	}

	// patch stuff here as only now do we know swapchain flags
	auto pattern = hook::pattern("C7 44 24 28 02 00 00 00 89 44 24 20 41").count(2);

	hook::put<uint32_t>(pattern.get(0).get<void>(4), g_swapChainFlags | 2);
	hook::put<uint32_t>(pattern.get(1).get<void>(4), g_swapChainFlags | 2);

	// we assume all users will stop using the object by the time it is dereferenced
	if (!initState->isReverseGame)
	{
		g_swapChain1 = swapChain1.Get();
	}

	g_dc = *ppImmediateContext;

	return hr;
}

static HRESULT CreateD3D11DeviceWrap(_In_opt_ IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType, HMODULE Software, UINT Flags, _In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels, UINT FeatureLevels, UINT SDKVersion, _In_opt_ CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc, _Out_opt_ IDXGISwapChain** ppSwapChain, _Out_opt_ ID3D11Device** ppDevice, _Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel, _Out_opt_ ID3D11DeviceContext** ppImmediateContext)
{
	HANDLE hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	HRESULT hresult = E_FAIL;

	WakeWindowThreadFor([&]()
	{
		hresult = CreateD3D11DeviceWrapOrig(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, pSwapChainDesc, ppSwapChain, ppDevice, pFeatureLevel, ppImmediateContext);

		SetEvent(hEvent);
	});

	WaitForSingleObject(hEvent, INFINITE);
	CloseHandle(hEvent);

	return hresult;
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

static std::vector<ID3D11Resource**> g_resources;

void(*g_origCreateBackbuffer)(void*);

void WrapCreateBackbuffer(void* tf)
{
	trace("Creating backbuffer.\n");

	g_lastBackbufTexture = NULL;

	g_origCreateBackbuffer(tf);

	trace("Done creating backbuffer.\n");
}

bool WrapVideoModeChange(VideoModeInfo* info)
{
	trace("Changing video mode.\n");

	for (auto& res : g_resources)
	{
		if (*res)
		{
			(*res)->Release();
		}

		*res = nullptr;
	}

	g_resources = {};

	bool success = g_origVideoModeChange(info);

	trace("Changing video mode success: %d.\n", success);

	return success;
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
	HANDLE handle = NULL;
	int width = 0;
	int height = 0;
	bool requested = false;
};

static rage::grcRenderTargetDX11** g_backBuffer;

static auto GetBackbuf()
{
	return *g_backBuffer;
}

static auto GetInvariantD3D11Device()
{
	WRL::ComPtr<IDXGIDevice> realDeviceDxgi;
	WRL::ComPtr<ID3D11Device> realDevice;

	GetD3D11Device()->QueryInterface(IID_PPV_ARGS(&realDeviceDxgi));
	realDeviceDxgi.As(&realDevice);

	return realDevice;
}

static auto GetInvariantD3D11DeviceContext()
{
	WRL::ComPtr<IUnknown> realDeviceContextUnk;
	WRL::ComPtr<ID3D11DeviceContext> realDeviceContext;

	GetD3D11DeviceContext()->QueryInterface(IID_PPV_ARGS(&realDeviceContextUnk));
	realDeviceContextUnk.As(&realDeviceContext);

	return realDeviceContext;
}

void RenderBufferToBuffer(ID3D11RenderTargetView* rtv, int width = 0, int height = 0)
{
	static auto didCallCrashometry = ([]()
	{
		AddCrashometry("did_render_backbuf", "true");

		return true;
	})();

	D3D11_TEXTURE2D_DESC resDesc = { 0 };
	auto backBuf = GetBackbuf();

	if (backBuf)
	{
		if (backBuf->texture)
		{
			((ID3D11Texture2D*)backBuf->texture)->GetDesc(&resDesc);
		}
	}

	// guess what we can't just CopyResource, so time for copy/pasted D3D11 garbage
	if (backBuf)
	{
		WRL::ComPtr<IUnknown> realSrvUnk;
		WRL::ComPtr<ID3D11ShaderResourceView> realSrv;

		backBuf->m_srv2->QueryInterface(IID_PPV_ARGS(&realSrvUnk));
		realSrvUnk.As(&realSrv);

		auto realDevice = GetInvariantD3D11Device();
		auto realDeviceContext = GetInvariantD3D11DeviceContext();

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
		std::call_once(of, [&realDevice]()
		{
			D3D11_SAMPLER_DESC sd = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
			realDevice->CreateSamplerState(&sd, &ss);

			D3D11_BLEND_DESC bd = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
			bd.RenderTarget[0].BlendEnable = FALSE;

			realDevice->CreateBlendState(&bd, &bs);

			realDevice->CreateVertexShader(quadVS, sizeof(quadVS), nullptr, &vs);
			realDevice->CreatePixelShader(quadPS, sizeof(quadPS), nullptr, &ps);
		});

		WRL::ComPtr<ID3DUserDefinedAnnotation> pPerf = NULL;
		realDeviceContext->QueryInterface(IID_PPV_ARGS(&pPerf));

		if (pPerf)
		{
			pPerf->BeginEvent(L"DrawRenderTexture");
		}

		auto deviceContext = realDeviceContext;
		
		WRL::ComPtr<ID3D11RenderTargetView> oldRtv;
		WRL::ComPtr<ID3D11DepthStencilView> oldDsv;
		deviceContext->OMGetRenderTargets(1, &oldRtv, &oldDsv);

		WRL::ComPtr<ID3D11SamplerState> oldSs;
		WRL::ComPtr<ID3D11BlendState> oldBs;
		WRL::ComPtr<ID3D11PixelShader> oldPs;
		WRL::ComPtr<ID3D11VertexShader> oldVs;
		WRL::ComPtr<ID3D11ShaderResourceView> oldSrv;

		D3D11_VIEWPORT oldVp;
		UINT numVPs = 1;

		deviceContext->RSGetViewports(&numVPs, &oldVp);

		CD3D11_VIEWPORT vp = CD3D11_VIEWPORT(0.0f, 0.0f, width ? width : m_width, height ? height : m_height);
		deviceContext->RSSetViewports(1, &vp);

		deviceContext->OMGetBlendState(&oldBs, nullptr, nullptr);

		deviceContext->PSGetShader(&oldPs, nullptr, nullptr);
		deviceContext->PSGetSamplers(0, 1, &oldSs);
		deviceContext->PSGetShaderResources(0, 1, &oldSrv);

		deviceContext->VSGetShader(&oldVs, nullptr, nullptr);

		deviceContext->OMSetRenderTargets(1, &rtv, nullptr);
		deviceContext->OMSetBlendState(bs, nullptr, 0xffffffff);

		ID3D11ShaderResourceView* srvs[] =
		{
			realSrv.Get()
		};

		deviceContext->PSSetShader(ps, nullptr, 0);
		deviceContext->PSSetSamplers(0, 1, &ss);
		deviceContext->PSSetShaderResources(0, 1, srvs);

		deviceContext->VSSetShader(vs, nullptr, 0);

		D3D11_PRIMITIVE_TOPOLOGY oldTopo;
		deviceContext->IAGetPrimitiveTopology(&oldTopo);

		ID3D11InputLayout* oldLayout;
		deviceContext->IAGetInputLayout(&oldLayout);

		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		deviceContext->IASetInputLayout(nullptr);

		FLOAT blank[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		deviceContext->ClearRenderTargetView(rtv, blank);

		deviceContext->Draw(4, 0);

		deviceContext->OMSetRenderTargets(1, oldRtv.GetAddressOf(), oldDsv.Get());

		deviceContext->IASetPrimitiveTopology(oldTopo);
		deviceContext->IASetInputLayout(oldLayout);

		deviceContext->VSSetShader(oldVs.Get(), nullptr, 0);
		deviceContext->PSSetShader(oldPs.Get(), nullptr, 0);
		deviceContext->PSSetSamplers(0, 1, oldSs.GetAddressOf());
		deviceContext->PSSetShaderResources(0, 1, oldSrv.GetAddressOf());
		deviceContext->OMSetBlendState(oldBs.Get(), nullptr, 0xffffffff);
		deviceContext->RSSetViewports(1, &oldVp);

		if (pPerf)
		{
			pPerf->EndEvent();
		}
	}
}

void CaptureInternalScreenshot()
{
	static D3D11_TEXTURE2D_DESC resDesc;

	auto backBuf = GetBackbuf();

	static int intWidth;
	static int intHeight;

	if (backBuf)
	{
		if (backBuf->texture)
		{
			((ID3D11Texture2D*)backBuf->texture)->GetDesc(&resDesc);

			intWidth = resDesc.Width;
			intHeight = resDesc.Height;
		}
	}

	static ID3D11Texture2D* myTexture;
	static ID3D11Texture2D* myStagingTexture;
	static ID3D11RenderTargetView* rtv;

	if (!myTexture)
	{
		{
			D3D11_TEXTURE2D_DESC texDesc = { 0 };
			texDesc.Width = resDesc.Width / 4;
			texDesc.Height = resDesc.Height / 4;
			texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			texDesc.Usage = D3D11_USAGE_DEFAULT;
			texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
			texDesc.CPUAccessFlags = 0;
			texDesc.MiscFlags = 0;

			WRL::ComPtr<ID3D11Texture2D> d3dTex;
			HRESULT hr = GetInvariantD3D11Device()->CreateTexture2D(&texDesc, nullptr, &d3dTex);
			if FAILED(hr)
			{
				return;
			}

			D3D11_RENDER_TARGET_VIEW_DESC rtDesc = CD3D11_RENDER_TARGET_VIEW_DESC(d3dTex.Get(), D3D11_RTV_DIMENSION_TEXTURE2D);
			GetInvariantD3D11Device()->CreateRenderTargetView(d3dTex.Get(), &rtDesc, &rtv);

			d3dTex.CopyTo(&myTexture);
		}

		{
			D3D11_TEXTURE2D_DESC texDesc = { 0 };
			texDesc.Width = resDesc.Width / 4;
			texDesc.Height = resDesc.Height / 4;
			texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			texDesc.MipLevels = 1;
			texDesc.ArraySize = 1;
			texDesc.SampleDesc.Count = 1;
			texDesc.SampleDesc.Quality = 0;
			texDesc.Usage = D3D11_USAGE_STAGING;
			texDesc.BindFlags = 0;
			texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			texDesc.MiscFlags = 0;

			WRL::ComPtr<ID3D11Texture2D> d3dTex;
			HRESULT hr = GetInvariantD3D11Device()->CreateTexture2D(&texDesc, nullptr, &d3dTex);
			if FAILED(hr)
			{
				return;
			}

			d3dTex.CopyTo(&myStagingTexture);
		}

		g_resources.push_back((ID3D11Resource**)&myTexture);
		g_resources.push_back((ID3D11Resource**)&myStagingTexture);
		g_resources.push_back((ID3D11Resource**)&rtv);
	}

	bool should = false;
	OnRequestInternalScreenshot(&should);

	if (!should)
	{
		return;
	}

	RenderBufferToBuffer(rtv, resDesc.Width / 4, resDesc.Height / 4);

	GetInvariantD3D11DeviceContext()->CopyResource(myStagingTexture, myTexture);

	D3D11_MAPPED_SUBRESOURCE msr;
	
	if (SUCCEEDED(GetInvariantD3D11DeviceContext()->Map(myStagingTexture, 0, D3D11_MAP_READ, 0, &msr)))
	{
		size_t blen = (resDesc.Height / 4) * msr.RowPitch;
		std::unique_ptr<uint8_t[]> data(new uint8_t[blen]);
		memcpy(data.get(), msr.pData, blen);

		GetInvariantD3D11DeviceContext()->Unmap(myStagingTexture, 0);

		// convert RGBA to RGB
		int w = (resDesc.Width / 4);
		int h = (resDesc.Height / 4);

		int rgbPitch = (w * 3);

		std::unique_ptr<uint8_t[]> outData(new uint8_t[h * rgbPitch]);

		for (int y = 0; y < h; y++)
		{
			int rgbaStart = (msr.RowPitch * y);
			int rgbStart = (rgbPitch * (h - y - 1));

			for (int x = 0; x < w; x++)
			{
				outData[rgbStart + 2] = data[rgbaStart];
				outData[rgbStart + 1] = data[rgbaStart + 1];
				outData[rgbStart] = data[rgbaStart + 2];

				rgbaStart += 4;
				rgbStart += 3;
			}
		}

		OnInternalScreenshot(outData.get(), resDesc.Width / 4, resDesc.Height / 4);
	}
}

void CaptureBufferOutput()
{
	static HostSharedData<GameRenderData> handleData("CfxGameRenderHandle");

	static D3D11_TEXTURE2D_DESC resDesc;
	static int lastWidth, lastHeight;

	auto backBuf = GetBackbuf();

	if (backBuf)
	{
		if (backBuf->texture)
		{
			((ID3D11Texture2D*)backBuf->texture)->GetDesc(&resDesc);

			handleData->width = resDesc.Width;
			handleData->height = resDesc.Height;
		}
	}
	else
	{
		return;
	}

	bool change = false;
	static ID3D11Texture2D* myTexture;
	static ID3D11RenderTargetView* rtv;

	if (lastWidth != handleData->width || lastHeight != handleData->height || g_lastBackbufTexture != backBuf->texture)
	{
		lastWidth = handleData->width;
		lastHeight = handleData->height;
		g_lastBackbufTexture = backBuf->texture;

		if (rtv)
		{
			rtv->Release();
			rtv = NULL;
		}

		if (myTexture)
		{
			myTexture->Release();
			myTexture = NULL;
		}

		change = true;
	}

	if (change)
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
		HRESULT hr = GetInvariantD3D11Device()->CreateTexture2D(&texDesc, nullptr, &d3dTex);
		if (FAILED(hr))
		{
			return;
		}

		D3D11_RENDER_TARGET_VIEW_DESC rtDesc = CD3D11_RENDER_TARGET_VIEW_DESC(d3dTex.Get(), D3D11_RTV_DIMENSION_TEXTURE2D);
		GetInvariantD3D11Device()->CreateRenderTargetView(d3dTex.Get(), &rtDesc, &rtv);

		d3dTex.CopyTo(&myTexture);

		WRL::ComPtr<IDXGIResource> dxgiResource;
		HANDLE sharedHandle;
		hr = d3dTex.As(&dxgiResource);
		if (FAILED(hr))
		{
			return;
		}

		hr = dxgiResource->GetSharedHandle(&sharedHandle);
		if (FAILED(hr))
		{
			// no error handling for safety
		}

		handleData->handle = sharedHandle;

		g_resources.push_back((ID3D11Resource**)&myTexture);
		g_resources.push_back((ID3D11Resource**)&rtv);
	}

	if (!rtv || !myTexture)
	{
		return;
	}

	if (!handleData->requested)
	{
		return;
	}

	RenderBufferToBuffer(rtv);
}

extern void RootCheckPresented(int& flags);
extern void RootSetPresented();

void D3DPresent(int syncInterval, int flags)
{
#if __has_include(<ENBApi.h>)
	static auto beforePresent = GetENBProcedure<TPresentHook>("API_BeforePresent");
	static auto afterPresent = GetENBProcedure<TPresentHook>("API_AfterPresent");

	if (beforePresent)
	{
		beforePresent();
	}
#endif

	if (g_overrideVsync)
	{
		syncInterval = 1;
	}

	RootCheckPresented(flags);

	if (IsWindows10OrGreater())
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
	else
	{
		(*g_dxgiSwapChain)->Present(syncInterval, flags);
	}

	static auto icgi = Instance<ICoreGameInit>::Get();
	if (icgi->HasVariable("gameMinimized"))
	{
		static WaitableTimer fpsTimer{NULL, TRUE, NULL};
		LimitFrameTime(fpsTimer, 30);
	}

	RootSetPresented();

#if __has_include(<ENBApi.h>)
	if (afterPresent)
	{
		afterPresent();
	}
#endif
}

static int Return1()
{
	return 1;
}

#include "dxerr.h"

static void __declspec(noinline) DisplayD3DCrashMessageGeneric(const std::string& errorBody)
{
	FatalError("DirectX encountered an unrecoverable error: %s", errorBody);
}

static void __declspec(noinline) DisplayD3DCrashMessageReShadeENBSeries(const std::string& errorBody)
{
	FatalError("DirectX encountered an unrecoverable error (R+E): %s", errorBody);
}

static void __declspec(noinline) DisplayD3DCrashMessageReShade(const std::string& errorBody)
{
	FatalError("DirectX encountered an unrecoverable error (R): %s", errorBody);
}

static void __declspec(noinline) DisplayD3DCrashMessageENBSeries(const std::string& errorBody)
{
	FatalError("DirectX encountered an unrecoverable error (E): %s", errorBody);
}

static void __declspec(noinline) DisplayD3DCrashMessageGraphicsMods(const std::string& errorBody)
{
	FatalError("DirectX encountered an unrecoverable error (M): %s", errorBody);
}

static std::string GetGraphicsModDetails(const std::string& fileName)
{
	std::wstring path = MakeRelativeGamePath(ToWide(fileName));
	DWORD versionInfoSize = GetFileVersionInfoSize(path.c_str(), nullptr);

	if (versionInfoSize)
	{
		std::vector<uint8_t> versionInfo(versionInfoSize);

		if (GetFileVersionInfo(path.c_str(), 0, versionInfo.size(), &versionInfo[0]))
		{
			struct LANGANDCODEPAGE
			{
				WORD wLanguage;
				WORD wCodePage;
			} * lpTranslate;

			UINT cbTranslate = 0;

			// Read the list of languages and code pages.

			VerQueryValue(&versionInfo[0],
			TEXT("\\VarFileInfo\\Translation"),
			(LPVOID*)&lpTranslate,
			&cbTranslate);

			if (cbTranslate > 0)
			{
				void* productNameBuffer;
				UINT productNameSize = 0;

				VerQueryValue(&versionInfo[0],
				va(L"\\StringFileInfo\\%04x%04x\\ProductName", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage),
				&productNameBuffer,
				&productNameSize);

				void* fixedInfoBuffer;
				UINT fixedInfoSize = 0;

				VerQueryValue(&versionInfo[0], L"\\", &fixedInfoBuffer, &fixedInfoSize);

				VS_FIXEDFILEINFO* fixedInfo = reinterpret_cast<VS_FIXEDFILEINFO*>(fixedInfoBuffer);

				if (productNameSize > 0 && fixedInfoSize > 0)
				{
					return fmt::sprintf("%s %d.%d.%d.%d",
						ToNarrow((wchar_t*)productNameBuffer),
						fixedInfo->dwProductVersionMS >> 16,
						fixedInfo->dwProductVersionMS & 0xFFFF,
						fixedInfo->dwProductVersionLS >> 16,
						fixedInfo->dwProductVersionLS & 0xFFFF);
				}
			}
		}
	}

	return "";
}

static void DisplayD3DCrashMessageWrap(HRESULT hr)
{
	constexpr auto errorBufferCount = 8192;
	auto errorBuffer = std::unique_ptr<wchar_t[]>(new wchar_t[errorBufferCount]);
	memset(errorBuffer.get(), 0, errorBufferCount * sizeof(wchar_t));
	DXGetErrorDescriptionW(hr, errorBuffer.get(), errorBufferCount);

	auto errorString = DXGetErrorStringW(hr);

	if (!errorString)
	{
		errorString = va(L"0x%08x", hr);
	}

	std::string removedError;

	if (hr == DXGI_ERROR_DEVICE_REMOVED)
	{
		HRESULT removedReason = GetD3D11Device()->GetDeviceRemovedReason();

		auto errorBuffer = std::unique_ptr<wchar_t[]>(new wchar_t[errorBufferCount]);
		memset(errorBuffer.get(), 0, errorBufferCount * sizeof(wchar_t));
		DXGetErrorDescriptionW(removedReason, errorBuffer.get(), errorBufferCount);

		auto removedString = DXGetErrorStringW(removedReason);

		if (!removedString)
		{
			removedString = va(L"0x%08x", hr);
		}

		removedError = ToNarrow(fmt::sprintf(L"\nGetDeviceRemovedReason returned %s - %s", removedString, errorBuffer.get()));
	}

	auto errorBody = fmt::sprintf("%s - %s%s", ToNarrow(errorString), ToNarrow(errorBuffer.get()), removedError);

	std::set<std::string> mods;
	auto getMod = [&mods](const std::string& filename)
	{
		if (auto details = GetGraphicsModDetails(filename); !details.empty())
		{
			mods.insert(details);
		}
	};

	getMod("d3d11.dll");
	getMod("dxgi.dll");
	getMod("d3d10.dll");

	if (!mods.empty())
	{
		errorBody += "\n\nThe following graphics mods were found in your GTA V installation:\n";

		for (const auto& mod : mods)
		{
			errorBody += fmt::sprintf("- %s\n", mod);
		}

		errorBody += "\nPlease contact the author of these mods to see if the issue may be related to them.";

		bool reshade = std::find_if(mods.begin(), mods.end(), [](const auto& str)
					   {
						   return str.find("ReShade") == 0;
					   })
					   != mods.end();

		bool enbseries = std::find_if(mods.begin(), mods.end(), [](const auto& str)
						 {
							 return str.find("ENBSeries") == 0;
						 })
						 != mods.end();

		if (reshade && enbseries)
		{
			DisplayD3DCrashMessageReShadeENBSeries(errorBody);
		}
		else if (reshade)
		{
			DisplayD3DCrashMessageReShade(errorBody);
		}
		else if (enbseries)
		{
			DisplayD3DCrashMessageENBSeries(errorBody);
		}
		else
		{
			DisplayD3DCrashMessageGraphicsMods(errorBody);
		}
	}
	else
	{
		DisplayD3DCrashMessageGeneric(errorBody);
	}
}

static void(*g_origPresent)();

void RagePresentWrap()
{
	InvokeRender();

	return g_origPresent();
}

static SRWLOCK g_textureOverridesLock = SRWLOCK_INIT;
static std::unordered_map<rage::grcTexture*, rage::grcTexture*> g_textureOverrides;

static void(*g_origSetTexture)(int a1, int index, rage::grcTexture* texture);

static void SetTextureHook(int a1, int index, rage::grcTexture* texture)
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

	g_origSetTexture(a1, index, texture);
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

static HWND g_gtaWindow;
static decltype(&CreateWindowExW) g_origCreateWindowExW;

static HWND WINAPI HookCreateWindowExW(_In_ DWORD dwExStyle, _In_opt_ LPCWSTR lpClassName, _In_opt_ LPCWSTR lpWindowName, _In_ DWORD dwStyle, _In_ int X, _In_ int Y, _In_ int nWidth, _In_ int nHeight, _In_opt_ HWND hWndParent, _In_opt_ HMENU hMenu, _In_opt_ HINSTANCE hInstance, _In_opt_ LPVOID lpParam)
{
	static HostSharedData<CfxState> initState("CfxInitState");
	HWND w;

	auto wndName = (CfxIsSinglePlayer()) ? L"Grand Theft Auto V (FiveM SP)" : L"FiveM® by Cfx.re";

	if (initState->isReverseGame)
	{
		static HostSharedData<ReverseGameData> rgd("CfxReverseGameData");

		w = g_origCreateWindowExW(dwExStyle, lpClassName, wndName, WS_POPUP | WS_CLIPSIBLINGS, 0, 0, rgd->width, rgd->height, NULL, hMenu, hInstance, lpParam);
	}
	else
	{
		w = g_origCreateWindowExW(dwExStyle, lpClassName, wndName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	}

	if (lpClassName && wcscmp(lpClassName, L"grcWindow") == 0)
	{
		CoreSetGameWindow(w);
	}
	
	g_gtaWindow = w;

	return w;
}

static HWND WINAPI HookGetForegroundWindow()
{
	auto orig = GetForegroundWindow();

	static HostSharedData<ReverseGameData> rgd("CfxReverseGameData");

	if (rgd->mainWindowHandle == orig)
	{
		orig = g_gtaWindow;
	}

	return orig;
}

static BOOL WINAPI HookShowWindow(HWND, int)
{
	return TRUE;
}

static HWND HookSetFocus(_In_opt_ HWND hWnd)
{
	return hWnd;
}

static HWND HookSetCapture(_In_opt_ HWND hWnd)
{
	return NULL;
}

static INT HookShowCursor(BOOL show)
{
	return (show) ? 0 : -1;
}

static BOOL HookClipCursor(const LPRECT rect)
{
	return TRUE;
}

static BOOL HookGetCursorPos(LPPOINT point)
{
	point->x = 24;
	point->y = 24;

	ClientToScreen(g_gtaWindow, point);

	return TRUE;
}

static BOOL HookAdjustWindowRect(_Inout_ LPRECT lpRect, _In_ DWORD dwStyle, _In_ BOOL bMenu)
{
	return TRUE;
}

static LONG_PTR SetWindowLongPtrAHook(HWND hWnd,
	int  nIndex,
	LONG_PTR dwNewLong)
{
	if (nIndex == GWL_STYLE)
	{
		dwNewLong &= ~WS_OVERLAPPEDWINDOW;
		dwNewLong = WS_POPUP;
	}

	return SetWindowLongPtrA(hWnd, nIndex, dwNewLong);
}

#include <concurrent_unordered_map.h>

static concurrency::concurrent_unordered_map<void*, bool> g_queriesSetUp;
static bool (*g_isQueryPending)(void*);

static std::atomic<int> g_isInRenderQuery;

DLL_EXPORT bool IsInRenderQuery()
{
	return g_isInRenderQuery > 0;
}

static void WaitForQueryHook(void* query)
{
	if (g_queriesSetUp[query])
	{
		++g_isInRenderQuery;

		while (g_isQueryPending(query))
		{
			Sleep(1);
		}

		--g_isInRenderQuery;
	}

	g_queriesSetUp[query] = false;
}

static void(*g_origSetupQuery)(void*);

static void SetupQueryHook(void* query)
{
	// vsync override means we want to live in real time
	if (g_overrideVsync)
	{
		return;
	}

	g_origSetupQuery(query);
	g_queriesSetUp[query] = true;
}

class FakeDXGIOutput : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IDXGIOutput>
{
	// Inherited via RuntimeClass
	virtual HRESULT __stdcall SetPrivateData(REFGUID Name, UINT DataSize, const void* pData) override
	{
		return S_OK;
	}
	virtual HRESULT __stdcall SetPrivateDataInterface(REFGUID Name, const IUnknown* pUnknown) override
	{
		return S_OK;
	}
	virtual HRESULT __stdcall GetPrivateData(REFGUID Name, UINT* pDataSize, void* pData) override
	{
		return S_OK;
	}
	virtual HRESULT __stdcall GetParent(REFIID riid, void** ppParent) override
	{
		return S_OK;
	}
	virtual HRESULT __stdcall GetDesc(DXGI_OUTPUT_DESC* pDesc) override
	{
		pDesc->AttachedToDesktop = TRUE;
		pDesc->DesktopCoordinates = { 0, 0, 1280, 720 };
		wcscpy(pDesc->DeviceName, L"DummyDevice");
		pDesc->Monitor = MonitorFromPoint({ 0, 0 }, 0);
		pDesc->Rotation = DXGI_MODE_ROTATION_IDENTITY;

		return S_OK;
	}
	virtual HRESULT __stdcall GetDisplayModeList(DXGI_FORMAT EnumFormat, UINT Flags, UINT* pNumModes, DXGI_MODE_DESC* pDesc) override
	{
		*pNumModes = 1;

		return S_OK;
	}
	virtual HRESULT __stdcall FindClosestMatchingMode(const DXGI_MODE_DESC* pModeToMatch, DXGI_MODE_DESC* pClosestMatch, IUnknown* pConcernedDevice) override
	{
		return S_OK;
	}
	virtual HRESULT __stdcall WaitForVBlank(void) override
	{
		return S_OK;
	}
	virtual HRESULT __stdcall TakeOwnership(IUnknown* pDevice, BOOL Exclusive) override
	{
		return S_OK;
	}
	virtual void __stdcall ReleaseOwnership(void) override
	{
	}
	virtual HRESULT __stdcall GetGammaControlCapabilities(DXGI_GAMMA_CONTROL_CAPABILITIES* pGammaCaps) override
	{
		return S_OK;
	}
	virtual HRESULT __stdcall SetGammaControl(const DXGI_GAMMA_CONTROL* pArray) override
	{
		return S_OK;
	}
	virtual HRESULT __stdcall GetGammaControl(DXGI_GAMMA_CONTROL* pArray) override
	{
		return S_OK;
	}
	virtual HRESULT __stdcall SetDisplaySurface(IDXGISurface* pScanoutSurface) override
	{
		return S_OK;
	}
	virtual HRESULT __stdcall GetDisplaySurfaceData(IDXGISurface* pDestination) override
	{
		return S_OK;
	}
	virtual HRESULT __stdcall GetFrameStatistics(DXGI_FRAME_STATISTICS* pStats) override
	{
		return S_OK;
	}
};

static HRESULT FakeOutput(IDXGIAdapter* adap, UINT idx, IDXGIOutput** out)
{
	if (idx >= 1)
	{
		return DXGI_ERROR_NOT_FOUND;
	}

	auto output = WRL::Make<FakeDXGIOutput>();
	output.CopyTo(out);

	return S_OK;
}

static HookFunction hookFunction([] ()
{
	static ConVar<bool> disableRenderingCvar("r_disableRendering", ConVar_None, false, &g_disableRendering);

	g_backBuffer = hook::get_address<decltype(g_backBuffer)>(hook::get_pattern("48 8B D0 48 89 05 ? ? ? ? EB 07 48 8B 15", 6));

	// end scene
	{
		auto location = hook::get_pattern("48 0F 45 C8 48 83 C4 40 5B E9 00 00 00 00", 9);
		hook::set_call(&g_origPresent, location);
		hook::jump(location, RagePresentWrap);
	}

	// present hook function
	hook::put(hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? 48 85 C0 74 0C 8B 4D 50 8B", 3)), D3DPresent);

	char* fnStart = hook::get_pattern<char>("8B 03 41 BE 01 00 00 00 89 05", -0x47);	
	g_dxgiSwapChain = hook::get_address<IDXGISwapChain**>(fnStart + 0x127);

	MH_CreateHook(fnStart, WrapVideoModeChange, (void**)&g_origVideoModeChange);

	g_resetVideoMode = hook::get_pattern<std::remove_pointer_t<decltype(g_resetVideoMode)>>("8B 44 24 50 4C 8B 17 44 8B 4E 04 44 8B 06", -0x61);

	// wrap video mode changing
	MH_CreateHook(hook::get_pattern("57 48 83 EC 20 49 83 63 08 00", -0xB), WrapCreateBackbuffer, (void**)&g_origCreateBackbuffer);

	MH_EnableHook(MH_ALL_HOOKS);

	if (g_disableRendering)
	{
		hook::jump(hook::get_pattern("84 D2 0F 45 C7 8A D9 89 05", -0x1F), Return1);
	}

	// force at least one DXGI output when disabled rendering
	if (g_disableRendering)
	{
		uint8_t mov[] = { 0x4C, 0x8D, 0x44, 0x24, 0x40 };
		auto location = hook::get_pattern<char>("8B D6 48 8B 01 4C 8D 44 24 ? FF", 2);

		hook::nop(location, 11);
		memcpy(location, mov, 5);
		hook::call(location + 5, FakeOutput);
	}

	// add D3D11_CREATE_DEVICE_BGRA_SUPPORT flag
	if (xbr::IsGameBuildOrGreater<2802>())
	{
		void* createDeviceLoc = hook::pattern("48 8D 44 24 78 89 74 24 30 89 7C 24 28").count(1).get(0).get<void>(18);
		hook::nop(createDeviceLoc, 6);
		hook::call(createDeviceLoc, CreateD3D11DeviceWrap);
	}
	else
	{
		void* createDeviceLoc = hook::pattern("48 8D 45 90 C7 44 24 30 07 00 00 00").count(1).get(0).get<void>(21);
		hook::nop(createDeviceLoc, 6);
		hook::call(createDeviceLoc, CreateD3D11DeviceWrap);
	}

	// don't crash on ID3D11DeviceContext::GetData call failures
	// these somehow are caused by NVIDIA driver settings?
	hook::nop(hook::get_pattern("EB 0C 8B C8 E8 ? ? ? ? B8 01", 4), 5);

	// ERR_GFX_D3D_INIT: display valid reasons
	auto loc = hook::get_pattern<char>("75 0A B9 06 BD F7 9C E8");
	hook::nop(loc + 2, 5);
	hook::call(loc + 7, DisplayD3DCrashMessageWrap);

	// remove infinite loop before grcResourceCache D3D failure
	{
		if (auto pattern = hook::pattern("EB FE 8B CF").count_hint(1); pattern.size() > 0)
		{
			hook::nop(pattern.get(0).get<void>(), 2);
		}
	}

	// texture overrides
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("48 8B CE 48 8B 74 24 38 48 6B C9 2A 48 03 CF", -0x45), SetTextureHook, (void**)&g_origSetTexture);
	MH_CreateHook(hook::get_pattern("48 8B D9 48 89 01 48 8B 49 28 E8 ? ? ? ? 48 8D", -0xD), grcTextureDtorHook, (void**)&g_origGrcTextureDtor);
	MH_EnableHook(MH_ALL_HOOKS);

	g_origCreateWindowExW = hook::iat("user32.dll", HookCreateWindowExW, "CreateWindowExW");

	static HostSharedData<CfxState> initState("CfxInitState");

	if (initState->isReverseGame)
	{
		// make window a child
		hook::iat("user32.dll", HookShowWindow, "ShowWindow");
		hook::iat("user32.dll", HookGetForegroundWindow, "GetForegroundWindow");
		hook::iat("user32.dll", HookSetFocus, "SetFocus");
		hook::iat("user32.dll", HookGetCursorPos, "GetCursorPos");
		hook::iat("user32.dll", HookSetCapture, "SetCapture");
		hook::iat("user32.dll", HookShowCursor, "ShowCursor");
		hook::iat("user32.dll", HookClipCursor, "ClipCursor");
		//hook::iat("user32.dll", HookAdjustWindowRect, "AdjustWindowRect");
		hook::iat("user32.dll", SetWindowLongPtrAHook, "SetWindowLongPtrA");
	}

	// some changes for timing (remove OS yields)

	// present sleeper
	hook::return_function(hook::get_pattern("0F 2F F0 76 0B  0F 2F F8 76 06 F3 0F 5E F7", -0x8F));

	// disable render queries if in load screen thread
	MH_Initialize();

	{
		auto location = hook::get_pattern<char>("84 C0 75 E8 48 83 C4 20 5B C3", -0x1F);
		hook::set_call(&g_isQueryPending, location + 0x1A);
		MH_CreateHook(location, WaitForQueryHook, NULL);
	}

	MH_CreateHook(hook::get_pattern("41 3B C3 74 30 4C 63 CB 44", -0x1F), SetupQueryHook, (void**)&g_origSetupQuery);
	MH_EnableHook(MH_ALL_HOOKS);

	// allow 5 slots for pre-buffer drawing
	OnPostFrontendRender.Connect([]()
	{
		uintptr_t a1;
		uintptr_t a2;

		EnqueueGenericDrawCommand([](uintptr_t, uintptr_t)
		{
			CaptureBufferOutput();
			CaptureInternalScreenshot();
		},
		&a1, &a2);
	}, INT32_MIN + 5);

	// prevent the render thread from knowing present is occluded
	hook::nop(hook::get_pattern("3D 01 00 7A 08 0F 94"), 5 + 7);

	// dumb render thread sleep
	*hook::get_address<bool*>(hook::get_pattern("75 08 8D 48 61 E8", -0x18)) = false;

	// don't try to resize on minimize
	hook::put<uint8_t>(hook::get_pattern("38 1D ? ? ? ? 89 0D ? ? ? ? 75", 12), 0xEB);

	// don't handle that flag either
	hook::put<uint8_t>(hook::get_pattern("38 05 ? ? ? ? 8A 0D ? ? ? ? 8A 15 ? ? ? ? 74 08", 18), 0xEB);

	// still set up BeginFrame/BeginDraw when occluded
	hook::put<int32_t>(hook::get_pattern("85 C0 74 2D 8B C8 E8", 0x2D), 0x81);

	// and when minimized
	hook::nop(hook::get_pattern("74 0C 84 C9 75 08 84 C0 0F", 8), 6);
});

// load the UMD early by having a 'dummy' D3D11 device so this won't slow down due to scanning
static InitFunction initFunctionEarlyUMD([]
{
	{
		auto state = CfxState::Get();
		if (!state->IsGameProcess())
		{
			return;
		}
	}

	std::thread([]()
	{
		IDXGIAdapter* adapter = nullptr;
		GoGetAdapter(&adapter);

		WRL::ComPtr<ID3D11Device> device;
		HRESULT hr = D3D11CreateDevice(adapter, D3D_DRIVER_TYPE_UNKNOWN, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &device, NULL, NULL);

		// release
		if (adapter)
		{
			adapter->Release();
		}

		Sleep(20000);
	}).detach();
});
