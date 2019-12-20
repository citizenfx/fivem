/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIWindow.h"

#include "NUIClient.h"
#include "NUIWindowManager.h"

#include <Error.h>

#include <LaunchMode.h>
#include <CoreConsole.h>

#include <include/base/cef_bind.h>
#include <include/wrapper/cef_closure_task.h>

#include <CefOverlay.h>

extern nui::GameInterface* g_nuiGi;

#include "memdbgon.h"

NUIWindow::NUIWindow(bool primary, int width, int height)
	: m_primary(primary), m_width(width), m_height(height), m_renderBuffer(nullptr), m_dirtyFlag(0), m_onClientCreated(nullptr), m_nuiTexture(nullptr), m_popupTexture(nullptr), m_swapTexture(nullptr),
	  m_swapRtv(nullptr), m_swapSrv(nullptr), m_dereferencedNuiTexture(false)
{
	memset(&m_lastDirtyRect, 0, sizeof(m_lastDirtyRect));

	Instance<NUIWindowManager>::Get()->AddWindow(this);
}

static void CloseBrowser(CefRefPtr<CefBrowser> browser)
{
	browser->GetHost()->CloseBrowser(true);
}

NUIWindow::~NUIWindow()
{
	auto nuiClient = ((NUIClient*)m_client.get());

	if (nuiClient)
	{
		auto& mutex = nuiClient->GetWindowLock();

		mutex.lock();
		nuiClient->SetWindowValid(false);

		if (nuiClient->GetBrowser() && nuiClient->GetBrowser()->GetHost())
		{
			if (!CefCurrentlyOn(TID_UI))
			{
				CefPostTask(TID_UI, base::Bind(&::CloseBrowser, nuiClient->GetBrowser()));
			}
			else
			{
				nuiClient->GetBrowser()->GetHost()->CloseBrowser(true);
			}
		}

		mutex.unlock();
	}

	if (m_popupTexture != m_parentTextures[CefRenderHandler::PaintElementType::PET_POPUP])
	{
		delete m_popupTexture;
	}

	if (m_nuiTexture != m_parentTextures[CefRenderHandler::PaintElementType::PET_VIEW])
	{
		delete m_popupTexture;
	}

	for (auto& texPair : m_parentTextures)
	{
		auto tex = texPair.second;

		if (tex)
		{
			delete tex;
		}
	}

	if (m_swapTexture)
	{
		m_swapTexture->Release();
	}

	if (m_swapRtv)
	{
		m_swapRtv->Release();
	}

	if (m_swapSrv)
	{
		m_swapSrv->Release();
	}

	if (m_renderBuffer)
	{
		delete[] m_renderBuffer;
	}

	Instance<NUIWindowManager>::Get()->RemoveWindow(this);
}

fwRefContainer<NUIWindow> NUIWindow::Create(bool primary, int width, int height, CefString url)
{
	auto window = new NUIWindow(primary, width, height);
	window->Initialize(url);

	return window;
}

#pragma region shaders
const BYTE quadPS[] =
{
	68,  88,  66,  67,  58,  78,
	234,  91, 133,  80, 171, 186,
	78,  67, 133,  59, 192,  44,
	182,  57,   1,   0,   0,   0,
	60,   2,   0,   0,   5,   0,
	0,   0,  52,   0,   0,   0,
	200,   0,   0,   0,  32,   1,
	0,   0,  84,   1,   0,   0,
	192,   1,   0,   0,  82,  68,
	69,  70, 140,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   2,   0,   0,   0,
	28,   0,   0,   0,   0,   4,
	255, 255,   0,   1,   0,   0,
	99,   0,   0,   0,  92,   0,
	0,   0,   3,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   1,   0,
	0,   0,   0,   0,   0,   0,
	96,   0,   0,   0,   2,   0,
	0,   0,   5,   0,   0,   0,
	4,   0,   0,   0, 255, 255,
	255, 255,   0,   0,   0,   0,
	1,   0,   0,   0,  12,   0,
	0,   0, 115, 109, 112,   0,
	116, 120,   0,  77, 105,  99,
	114, 111, 115, 111, 102, 116,
	32,  40,  82,  41,  32,  72,
	76,  83,  76,  32,  83, 104,
	97, 100, 101, 114,  32,  67,
	111, 109, 112, 105, 108, 101,
	114,  32,  49,  48,  46,  49,
	0, 171,  73,  83,  71,  78,
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
	68,  82, 100,   0,   0,   0,
	64,   0,   0,   0,  25,   0,
	0,   0,  90,   0,   0,   3,
	0,  96,  16,   0,   0,   0,
	0,   0,  88,  24,   0,   4,
	0, 112,  16,   0,   0,   0,
	0,   0,  85,  85,   0,   0,
	98,  16,   0,   3,  50,  16,
	16,   0,   1,   0,   0,   0,
	101,   0,   0,   3, 242,  32,
	16,   0,   0,   0,   0,   0,
	69,   0,   0,   9, 242,  32,
	16,   0,   0,   0,   0,   0,
	70,  16,  16,   0,   1,   0,
	0,   0,  70, 126,  16,   0,
	0,   0,   0,   0,   0,  96,
	16,   0,   0,   0,   0,   0,
	62,   0,   0,   1,  83,  84,
	65,  84, 116,   0,   0,   0,
	2,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	2,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   1,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   1,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0,   0,   0,   0,   0,
	0,   0
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

void NUIWindow::Initialize(CefString url)
{
	static bool nuiSharedResourcesEnabled = true;
	static ConVar<bool> nuiSharedResources("nui_useSharedResources", ConVar_Archive, true, &nuiSharedResourcesEnabled);

	InitializeCriticalSection(&m_renderBufferLock);

	if (m_renderBuffer)
	{
		delete[] m_renderBuffer;
	}

	// rounding function
	auto roundUp = [] (int x, int y)
	{
		return x + (y - (x % y));
	};

	// create the temporary backing store
	m_roundedHeight = roundUp(m_height, 16);
	m_roundedWidth = roundUp(m_width, 16);

	// create the backing texture
	m_nuiTexture = g_nuiGi->CreateTextureBacking(m_width, m_height, nui::GITextureFormat::ARGB);
	
	if (!m_primary)
	{
		D3D11_TEXTURE2D_DESC tgtDesc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_B8G8R8A8_UNORM, m_width, m_height, 1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);

		auto d3d = g_nuiGi->GetD3D11Device();
		d3d->CreateTexture2D(&tgtDesc, nullptr, &m_swapTexture);

		D3D11_RENDER_TARGET_VIEW_DESC rtDesc = CD3D11_RENDER_TARGET_VIEW_DESC(m_swapTexture, D3D11_RTV_DIMENSION_TEXTURE2D);

		d3d->CreateRenderTargetView(m_swapTexture, &rtDesc, &m_swapRtv);
	}

	// create the client/browser instance
	m_client = new NUIClient(this);

	CefWindowInfo info;
	info.SetAsWindowless(NULL);
	info.shared_texture_enabled = (!CfxIsWine() && nuiSharedResourcesEnabled);
	info.external_begin_frame_enabled = true;
	info.width = m_width;
	info.height = m_height;

	CefBrowserSettings settings;
	settings.javascript_close_windows = STATE_DISABLED;
	settings.web_security = STATE_DISABLED;
	settings.windowless_frame_rate = 240;
	CefString(&settings.default_encoding).FromString("utf-8");

	CefRefPtr<CefRequestContext> rc = CefRequestContext::GetGlobalContext();
	CefBrowserHost::CreateBrowser(info, m_client, url, settings, rc);

	if (!info.shared_texture_enabled)
	{
		m_renderBuffer = new char[4 * m_roundedWidth * m_roundedHeight];
	}
}

void NUIWindow::AddDirtyRect(const CefRect& rect)
{
	EnterCriticalSection(&m_renderBufferLock);
	m_dirtyRects.push(rect);
	LeaveCriticalSection(&m_renderBufferLock);
}

CefBrowser* NUIWindow::GetBrowser()
{
	return ((NUIClient*)m_client.get())->GetBrowser();
}

void NUIWindow::UpdateSharedResource(void* sharedHandle, uint64_t syncKey, const CefRenderHandler::RectList& rects, CefRenderHandler::PaintElementType type)
{
	if (!m_primary && type != CefRenderHandler::PaintElementType::PET_VIEW)
	{
		return;
	}

	HANDLE parentHandle = (void*)sharedHandle;
	m_syncKey = syncKey;
	
	{
		if (m_lastParentHandle[type] != parentHandle)
		{
			if (type == CefRenderHandler::PaintElementType::PET_VIEW)
			{
				trace("Changing NUI shared resource...\n");
			}

			m_lastParentHandle[type] = parentHandle;

			auto& texRef = (type == CefRenderHandler::PaintElementType::PET_VIEW) ? m_nuiTexture : m_popupTexture;

			int w, h;

			if (type == CefRenderHandler::PaintElementType::PET_VIEW)
			{
				w = m_width;
				h = m_height;
			}
			else
			{
				w = m_popupRect.width;
				h = m_popupRect.height;
			}

			if (!m_primary)
			{
				auto oldRef = m_parentTextures[type];

				auto fakeTexRef = g_nuiGi->CreateTextureFromShareHandle(parentHandle, w, h);
				SetParentTexture(type, fakeTexRef);

				if (oldRef)
				{
					delete oldRef;
				}

				auto oldSrv = m_swapSrv;

				struct
				{
					void* vtbl;
					ID3D11Device* rawDevice;
				}*deviceStuff = (decltype(deviceStuff))g_nuiGi->GetD3D11Device();

				deviceStuff->rawDevice->CreateShaderResourceView((ID3D11Resource*)GetParentTexture(CefRenderHandler::PaintElementType::PET_VIEW)->GetNativeTexture(), nullptr, &m_swapSrv);

				if (oldSrv)
				{
					oldSrv->Release();
				}
			}
			else
			{
				nui::GITexture* oldRef = nullptr;

				if (texRef)
				{
					oldRef = texRef;
				}

				texRef = g_nuiGi->CreateTextureFromShareHandle(parentHandle, w, h);
				SetParentTexture(type, texRef);

				if (oldRef)
				{
					delete oldRef;
					oldRef = nullptr;
				}
			}
		}
	}

	for (auto& rect : rects)
	{
		int x = rect.x;
		int y = rect.y;
		int width = rect.width;
		int height = rect.height;

		RECT newRect;
		newRect.left = x;
		newRect.right = x + width;
		newRect.top = GetHeight() - y - height;
		newRect.bottom = GetHeight() - y;
		//newRect.top = y;
		//newRect.bottom = y + height;
		
		RECT oldRect = m_lastDirtyRect;

		UnionRect(&m_lastDirtyRect, &newRect, &oldRect);
	}

	MarkRenderBufferDirty();
}

#include <d3d11_1.h>

void NUIWindow::UpdateFrame()
{
	if (m_client)
	{
		auto client = ((NUIClient*)m_client.get());

		auto browser = client->GetBrowser();
		
		if (browser)
		{
			auto host = browser->GetHost();

			if (host)
			{
				host->SendExternalBeginFrame();
			}
		}
	}

	if (!m_nuiTexture)
	{
		return;
	}

	if (m_primary)
	{
		int resX, resY;
		g_nuiGi->GetGameResolution(&resX, &resY);

		if (m_width != resX || m_height != resY)
		{
			m_width = resX;
			m_height = resY;

			((NUIClient*)m_client.get())->GetBrowser()->GetHost()->WasResized();

			// make a new texture
			delete m_nuiTexture;

			m_nuiTexture = g_nuiGi->CreateTextureBacking(m_width, m_height, nui::GITextureFormat::ARGB);
		}

		for (auto& item : m_pollQueue)
		{
			NUIClient* client = static_cast<NUIClient*>(m_client.get());
			auto browser = client->GetBrowser();

			auto message = CefProcessMessage::Create("doPoll");
			auto argList = message->GetArgumentList();

			argList->SetSize(1);
			argList->SetString(0, item);

			browser->SendProcessMessage(PID_RENDERER, message);
		}
	}

	m_pollQueue.clear();

	NUIWindowManager* wm = Instance<NUIWindowManager>::Get();
	auto texture = GetParentTexture(CefRenderHandler::PaintElementType::PET_VIEW);

	if (texture)
	{
		//
		// dirty flag checking and CopySubresourceRegion are disabled here due to some issue
		// with scheduling frame copies from the OnAcceleratedPaint handler.
		//
		// this'll use a bunch of additional GPU memory bandwidth, but this should not be an
		// issue on any modern GPU.
		//
		//if (InterlockedExchange(&m_dirtyFlag, 0) > 0)
		if (!m_primary)
		{
			HRESULT hr = S_OK;

			if (hr == S_OK)
			{
				ID3D11Device* device = g_nuiGi->GetD3D11Device();

				if (device)
				{
					ID3D11DeviceContext* deviceContext = g_nuiGi->GetD3D11DeviceContext();
					assert(deviceContext);

					D3D11_BOX box = CD3D11_BOX(m_lastDirtyRect.left,
											   m_lastDirtyRect.top,
											   0,
											   m_lastDirtyRect.right,
											   m_lastDirtyRect.bottom,
											   1);

					if (m_primary)
					{
						//deviceContext->CopyResource(m_nuiTexture->texture, texture);
						g_nuiGi->BlitTexture(m_nuiTexture, texture);
					}
					else
					{
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
							g_nuiGi->GetD3D11Device()->CreateSamplerState(&sd, &ss);

							D3D11_BLEND_DESC bd = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
							g_nuiGi->GetD3D11Device()->CreateBlendState(&bd, &bs);

							g_nuiGi->GetD3D11Device()->CreateVertexShader(quadVS, sizeof(quadVS), nullptr, &vs);
							g_nuiGi->GetD3D11Device()->CreatePixelShader(quadPS, sizeof(quadPS), nullptr, &ps);
						});

						ID3DUserDefinedAnnotation* pPerf;
						deviceContext->QueryInterface(__uuidof(pPerf), reinterpret_cast<void**>(&pPerf));

						pPerf->BeginEvent(L"DRAWSHIT");

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

						deviceContext->OMSetRenderTargets(1, &m_swapRtv, nullptr);
						deviceContext->OMSetBlendState(bs, nullptr, 0xffffffff);

						deviceContext->PSSetShader(ps, nullptr, 0);
						deviceContext->PSSetSamplers(0, 1, &ss);
						deviceContext->PSSetShaderResources(0, 1, &m_swapSrv);

						deviceContext->VSSetShader(vs, nullptr, 0);

						D3D11_PRIMITIVE_TOPOLOGY oldTopo;
						deviceContext->IAGetPrimitiveTopology(&oldTopo);

						ID3D11InputLayout* oldLayout;
						deviceContext->IAGetInputLayout(&oldLayout);

						deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
						deviceContext->IASetInputLayout(nullptr);

						deviceContext->Draw(4, 0);

						deviceContext->CopyResource((ID3D11Resource*)m_nuiTexture->GetNativeTexture(), m_swapTexture);

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

					memset(&m_lastDirtyRect, 0, sizeof(m_lastDirtyRect));
				}
			}
			else
			{
				MarkRenderBufferDirty();
			}
		}
	}
	else if (m_renderBuffer)
	{
		if (InterlockedExchange(&m_dirtyFlag, 0) > 0)
		{
			void* pBits = nullptr;
			int pitch;
			bool discarded = false;

			nui::GILockedTexture lockedTexture;

			if (m_nuiTexture->Map(0, 0, &lockedTexture, nui::GILockFlags::Write))
			{
				pBits = lockedTexture.pBits;
				pitch = lockedTexture.pitch;
			}
			else if (m_nuiTexture->Map(0, 0, &lockedTexture, nui::GILockFlags::WriteDiscard))
			{
				pBits = lockedTexture.pBits;
				pitch = lockedTexture.pitch;

				discarded = true;
			}

			if (pBits)
			{
				if (!discarded)
				{
					while (!m_dirtyRects.empty())
					{
						EnterCriticalSection(&m_renderBufferLock);
						CefRect rect = m_dirtyRects.front();
						m_dirtyRects.pop();
						LeaveCriticalSection(&m_renderBufferLock);

						int height = m_height;

						for (int y = rect.y; y < (rect.y + rect.height); y++)
						{
							int dy = height - y;

							int* src = &((int*)(m_renderBuffer))[(y * m_roundedWidth) + rect.x];
							int* dest = &((int*)(pBits))[(dy * (pitch / 4)) + rect.x];

							memcpy(dest, src, (rect.width * 4));
						}
					}
				}
				else
				{
					EnterCriticalSection(&m_renderBufferLock);
					m_dirtyRects = std::queue<CefRect>();
					LeaveCriticalSection(&m_renderBufferLock);

					memcpy(pBits, m_renderBuffer, m_height * pitch);
				}

				m_nuiTexture->Unmap(&lockedTexture);
			}
		}
	}
}

void NUIWindow::SignalPoll(std::string& argument)
{
	if (m_pollQueue.find(argument) == m_pollQueue.end())
	{
		m_pollQueue.insert(argument);
	}
}

void NUIWindow::HandlePopupShow(bool show)
{
	if (!show)
	{
		if (m_parentTextures[CefRenderHandler::PaintElementType::PET_POPUP])
		{
			delete m_parentTextures[CefRenderHandler::PaintElementType::PET_POPUP];

			if (m_popupTexture != m_parentTextures[CefRenderHandler::PaintElementType::PET_POPUP])
			{
				delete m_popupTexture;
			}

			m_parentTextures[CefRenderHandler::PaintElementType::PET_POPUP] = nullptr;
			m_popupTexture = nullptr;
		}
	}
}

void NUIWindow::SetPopupRect(const CefRect& rect)
{
	m_popupRect = rect;

	m_popupTexture = g_nuiGi->CreateTextureBacking(rect.width, rect.height, nui::GITextureFormat::ARGB);
}

void NUIWindow::SetPaintType(NUIPaintType type)
{
	m_paintType = type;
}

void NUIWindow::Invalidate()
{
	((NUIClient*)m_client.get())->GetBrowser()->GetHost()->Invalidate(PET_VIEW);
}
