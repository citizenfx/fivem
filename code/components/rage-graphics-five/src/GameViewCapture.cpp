#include "StdInc.h"

#include <array>
#include <mutex>

#include <Hooking.h>

#include <d3d11_1.h>
#include <dxgi1_4.h>
#include <dxgi1_5.h>
#include <wrl.h>

#include <DrawCommands.h>
#include <ProfilerShaders.h>
#include <HostSharedData.h>
#include <Error.h>

namespace WRL = Microsoft::WRL;

extern std::vector<ID3D11Resource**> g_resources;
extern void* g_lastBackbufTexture;

struct GameRenderData
{
	HANDLE handle = NULL;
	int width = 0;
	int height = 0;
	bool requested = false;

	// For backwards compatability. The flipped texture is slower and more prone to flickering/stuttering.
	bool requestedFlipped;
	HANDLE flippedHandle;
};

static auto GetInvariantD3D11Device()
{
	WRL::ComPtr<IDXGIDevice> realDeviceDxgi;
	WRL::ComPtr<ID3D11Device> realDevice = nullptr;

	GetD3D11Device()->QueryInterface(IID_PPV_ARGS(&realDeviceDxgi));
	if (realDeviceDxgi)
	{
		realDeviceDxgi.As(&realDevice);
	}

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

static rage::grcRenderTargetDX11** g_backBuffer;

static auto GetBackbuf()
{
	return *g_backBuffer;
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

	if (!backBuf || !backBuf->texture)
	{
		return;
	}

	((ID3D11Texture2D*)backBuf->texture)->GetDesc(&resDesc);

	WRL::ComPtr<IUnknown> realSrvUnk;
	WRL::ComPtr<ID3D11ShaderResourceView> realSrv;

	backBuf->m_srv2->QueryInterface(IID_PPV_ARGS(&realSrvUnk));
	realSrvUnk.As(&realSrv);

	auto realDevice = GetInvariantD3D11Device();
	auto realDeviceContext = GetInvariantD3D11DeviceContext();
	if (!realDevice)
	{
		return;
	}

	auto m_width = resDesc.Width;
	auto m_height = resDesc.Height;

	WRL::ComPtr<ID3DUserDefinedAnnotation> pPerf = NULL;
	realDeviceContext->QueryInterface(IID_PPV_ARGS(&pPerf));

	if (pPerf)
	{
		pPerf->BeginEvent(L"DrawRenderTexture");
	}

	WRL::ComPtr<ID3D11Resource> srcRes;
	backBuf->m_srv2->GetResource(srcRes.GetAddressOf());
	if (!srcRes)
	{
		return;
	}

	WRL::ComPtr<ID3D11Texture2D> srcTex;
	if (FAILED(srcRes.As(&srcTex)))
	{
		return;
	}

	WRL::ComPtr<ID3D11Resource> dstRes;
	rtv->GetResource(&dstRes);
	if (!dstRes)
	{
		return;
	}

	WRL::ComPtr<ID3D11Texture2D> dstTex;
	if (FAILED(dstRes.As(&dstTex)))
	{
		return;
	}

	D3D11_TEXTURE2D_DESC srcDesc, dstDesc;
	srcTex->GetDesc(&srcDesc);
	dstTex->GetDesc(&dstDesc);

	if (srcDesc.Format != dstDesc.Format)
	{
		return;
	}

	if (srcDesc.Width != dstDesc.Width || srcDesc.Height != dstDesc.Height || srcDesc.Format != dstDesc.Format)
	{
		return;
	}

	realDeviceContext->CopyResource(dstTex.Get(), srcTex.Get());

	static ID3D11Query* copyQuery = nullptr;
	if (!copyQuery)
	{
		D3D11_QUERY_DESC qd{};
		qd.Query = D3D11_QUERY_EVENT;
		realDevice->CreateQuery(&qd, &copyQuery);
	}

	if (copyQuery)
	{
		realDeviceContext->End(copyQuery);
	}

	if (pPerf)
	{
		pPerf->EndEvent();
	}
}

// For use with the profiler and compatability with the old game-view capture implementations
static void RenderBufferToBufferFlipped(ID3D11RenderTargetView* rtv, int width = 0, int height = 0)
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

	if (!backBuf)
	{
		return;
	}

	WRL::ComPtr<IUnknown> realSrvUnk;
	WRL::ComPtr<ID3D11ShaderResourceView> realSrv;

	backBuf->m_srv2->QueryInterface(IID_PPV_ARGS(&realSrvUnk));
	realSrvUnk.As(&realSrv);

	auto realDevice = GetInvariantD3D11Device();
	auto realDeviceContext = GetInvariantD3D11DeviceContext();
	if (!realDevice)
	{
		return;
	}

	auto m_width = resDesc.Width;
	auto m_height = resDesc.Height;

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

		realDevice->CreateVertexShader(fx::shaders::quadVS, sizeof(fx::shaders::quadVS), nullptr, &vs);
		realDevice->CreatePixelShader(fx::shaders::quadPS, sizeof(fx::shaders::quadPS), nullptr, &ps);
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

	ID3D11ShaderResourceView* srvs[] = {
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

			WRL::ComPtr<ID3D11Device> device = GetInvariantD3D11Device();
			if (!device)
			{
				return;
			}

			WRL::ComPtr<ID3D11Texture2D> d3dTex;
			HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, &d3dTex);
			if FAILED (hr)
			{
				return;
			}

			D3D11_RENDER_TARGET_VIEW_DESC rtDesc = CD3D11_RENDER_TARGET_VIEW_DESC(d3dTex.Get(), D3D11_RTV_DIMENSION_TEXTURE2D);
			device->CreateRenderTargetView(d3dTex.Get(), &rtDesc, &rtv);

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
			if FAILED (hr)
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

	RenderBufferToBufferFlipped(rtv, resDesc.Width / 4, resDesc.Height / 4);

	GetInvariantD3D11DeviceContext()->CopyResource(myStagingTexture, myTexture);

	D3D11_MAPPED_SUBRESOURCE msr;

	if (SUCCEEDED(GetInvariantD3D11DeviceContext()->Map(myStagingTexture, 0, D3D11_MAP_READ, 0, &msr)))
	{
		size_t blen = (static_cast<size_t>(resDesc.Height / 4)) * msr.RowPitch;
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
	static ID3D11Texture2D* gameViewTex;
	static ID3D11RenderTargetView* gameViewRTV;

	static ID3D11Texture2D* flippedTex;
	static ID3D11RenderTargetView* flippedRTV;

	if (lastWidth != handleData->width || lastHeight != handleData->height || g_lastBackbufTexture != backBuf->texture)
	{
		lastWidth = handleData->width;
		lastHeight = handleData->height;
		g_lastBackbufTexture = backBuf->texture;

		if (flippedRTV)
		{
			flippedRTV->Release();
			flippedRTV = NULL;
		}

		if (gameViewRTV)
		{
			gameViewRTV->Release();
			gameViewRTV = NULL;
		}

		if (flippedTex)
		{
			flippedTex->Release();
			flippedTex = NULL;
		}

		if (gameViewTex)
		{
			gameViewTex->Release();
			gameViewTex = NULL;
		}

		change = true;
	}

	if (change)
	{
		auto createTexture = [](ID3D11Texture2D** outTex, ID3D11RenderTargetView** outRtv) -> HANDLE
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

			WRL::ComPtr<ID3D11Device> device = GetInvariantD3D11Device();
			if (!device)
			{
				return NULL;
			}

			WRL::ComPtr<ID3D11Texture2D> d3dTexture;
			HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, &d3dTexture);
			if (FAILED(hr))
			{
				return NULL;
			}

			D3D11_RENDER_TARGET_VIEW_DESC rtDesc = CD3D11_RENDER_TARGET_VIEW_DESC(d3dTexture.Get(), D3D11_RTV_DIMENSION_TEXTURE2D);
			device->CreateRenderTargetView(d3dTexture.Get(), &rtDesc, outRtv);

			d3dTexture.CopyTo(outTex);

		    g_resources.push_back((ID3D11Resource**)outTex);
			g_resources.push_back((ID3D11Resource**)outRtv);

			WRL::ComPtr<IDXGIResource> dxgiResource;
			HANDLE sharedHandle;
			hr = d3dTexture.As(&dxgiResource);
			if (FAILED(hr))
			{
				return NULL;
			}

			hr = dxgiResource->GetSharedHandle(&sharedHandle);
			if (FAILED(hr))
			{
				trace("Unable to create shared handle for game-view texture %x\n", hr);
				return NULL;
			}

			return sharedHandle;
		};

		handleData->handle = createTexture(&gameViewTex, &gameViewRTV);
		handleData->flippedHandle = createTexture(&flippedTex, &flippedRTV);
	}

	if (handleData->requested && gameViewRTV && gameViewTex)
	{
		RenderBufferToBuffer(gameViewRTV);
	}

	if (handleData->requestedFlipped && flippedRTV && flippedTex)
	{
		RenderBufferToBufferFlipped(flippedRTV);
	}
}

static HookFunction hookFunction([]()
{
	g_backBuffer = hook::get_address<decltype(g_backBuffer)>(hook::get_pattern("48 8B D0 48 89 05 ? ? ? ? EB 07 48 8B 15", 6));

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
	},
	INT32_MIN + 5);
});
