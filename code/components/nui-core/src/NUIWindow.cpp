/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIWindow.h"

#include "NUIClient.h"
#include "NUIWindowManager.h"
#include "DUIShaders.h"

#include <Error.h>

#include <LaunchMode.h>
#include <CoreConsole.h>

#include <include/base/cef_bind.h>
#include <include/base/cef_callback_helpers.h>
#include <include/wrapper/cef_closure_task.h>
#include <include/cef_request_context_handler.h>

#include <CefOverlay.h>

extern nui::GameInterface* g_nuiGi;

#include "memdbgon.h"

extern std::wstring GetNUIStoragePath();

static bool nuiFixedSizeEnabled;

namespace nui
{
extern bool g_rendererInit;
extern bool g_hasFirstRender;
extern void AddSchemeHandlerFactories(CefRefPtr<CefRequestContext> rc);
}

NUIWindow::NUIWindow(bool rawBlit, int width, int height, const std::string& windowContext)
	: m_isPrimary(rawBlit), m_width(width), m_height(height), m_onClientCreated(nullptr), m_nuiTexture(nullptr), m_popupTexture(nullptr),
	  m_lastFrameTime(0), m_lastMessageTime(0), m_paintType(NUIPaintTypeDummy), m_windowContext(windowContext)
#ifdef GTA_FIVE
	  ,m_swapTexture(nullptr), m_swapRtv(nullptr)
#endif
{
	memset(&m_sharedResourceTexturesCreated, 0, sizeof(m_sharedResourceTexturesCreated));
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
		nuiClient->SetWindowValid(false);
		nuiClient->ClearWindow();

		CefRefPtr<CefBrowser> browser;
		{
			std::unique_lock _(nuiClient->GetWindowLock());
			if (nuiClient->GetBrowser() && nuiClient->GetBrowser()->GetHost())
			{
				browser = nuiClient->GetBrowser();
			}
		}

		if (browser)
		{
			if (!CefCurrentlyOn(TID_UI))
			{
				CefPostTask(TID_UI, base::BindOnce(&::CloseBrowser, scoped_refptr(browser)));
			}
			else
			{
				browser->GetHost()->CloseBrowser(true);
			}
		}
	}

	Instance<NUIWindowManager>::Get()->RemoveWindow(this);
}

fwRefContainer<NUIWindow> NUIWindow::Create(bool primary, int width, int height, CefString url, bool instant, const std::string& context)
{
	auto window = new NUIWindow(primary, width, height, context);

	if (instant)
	{
		window->Initialize(url);
	}
	else
	{
		window->m_initUrl = url;
	}

	return window;
}

void NUIWindow::DeferredCreate()
{
	if (!m_client)
	{
		Initialize(m_initUrl);
	}
}

#include <dxgi1_6.h>

#pragma comment(lib, "dxgi.lib")

static uint64_t GetPrimaryGPUVRAM()
{
	namespace WRL = Microsoft::WRL;
	WRL::ComPtr<IDXGIFactory1> factory;
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
	{
		return 0;
	}

	auto hardwareVram = [](IDXGIAdapter1* adapter) -> uint64_t
	{
		DXGI_ADAPTER_DESC1 desc;
		if (FAILED(adapter->GetDesc1(&desc)) || (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
		{
			return 0;
		}
		return desc.DedicatedVideoMemory;
	};

	WRL::ComPtr<IDXGIFactory6> factory6;
	if (SUCCEEDED(factory.As(&factory6)))
	{
		WRL::ComPtr<IDXGIAdapter1> adapter;
		for (UINT i = 0; SUCCEEDED(factory6->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter))); ++i)
		{
			if (uint64_t vram = hardwareVram(adapter.Get()))
			{
				return vram;
			}
		}
	}

	uint64_t best = 0;
	WRL::ComPtr<IDXGIAdapter1> adapter;
	for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		best = std::max(best, hardwareVram(adapter.Get()));
	}
	return best;
}

static int GetPrimaryMonitorRefreshRate()
{
	DEVMODE devMode = {};
	devMode.dmSize = sizeof(DEVMODE);

	// For GPU's that are limited by VRAM.
	// Cap the frame rate of CEF window updates in order to avoid 
	// allocating textures that may negatively impact user experience.
	if (GetPrimaryGPUVRAM() < 4096)
	{
		return 60;
	}

	if (EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode))
	{
		return devMode.dmDisplayFrequency;
	}

	return 240;
}

void NUIWindow::Initialize(CefString url)
{
	InitializeRenderBacking();

	// create the client/browser instance
	{
		CefRefPtr<NUIClient> client = new NUIClient(this);
		client->Initialize();

		m_client = client;
	}

	CefWindowInfo info;
	info.SetAsWindowless(NULL);
	info.shared_texture_enabled = true;
	info.bounds.x = 0;
	info.bounds.y = 0;
	info.bounds.width = m_width;
	info.bounds.height = m_height;

	CefBrowserSettings settings;
	settings.javascript_close_windows = STATE_DISABLED;
	settings.windowless_frame_rate = GetPrimaryMonitorRefreshRate(); // 240;
	CefString(&settings.default_encoding).FromString("utf-8");

	CefRefPtr<CefRequestContext> rc;

	if (m_windowContext.empty())
	{
		rc = CefRequestContext::GetGlobalContext();
	}
	else
	{
		auto cachePath = fmt::sprintf(L"%s\\context-%s", GetNUIStoragePath(), ToWide(m_windowContext));
		CreateDirectory(cachePath.c_str(), nullptr);

		CefRequestContextSettings rcConfig;
		CefString(&rcConfig.cache_path).FromWString(cachePath);
		rc = CefRequestContext::CreateContext(rcConfig, {});

		nui::AddSchemeHandlerFactories(rc);
	}

	CefBrowserHost::CreateBrowser(info, m_client, url, settings, {}, rc);
}

void NUIWindow::InitializeRenderBacking()
{
	if (!nui::g_rendererInit)
	{
		return;
	}

	// create the backing texture
	{
		std::lock_guard<std::shared_mutex> _(m_textureMutex);
		m_nuiTexture = g_nuiGi->CreateTextureBacking(m_width, m_height, nui::GITextureFormat::ARGB);
	}

#ifdef GTA_FIVE
	if (!m_isPrimary)
	{
		D3D11_TEXTURE2D_DESC tgtDesc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_B8G8R8A8_UNORM, m_width, m_height, 1, 1, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE);

		auto d3d = g_nuiGi->GetD3D11Device();
		struct
		{
			void* vtbl;
			ID3D11Device* rawDevice;
		}* deviceStuff = (decltype(deviceStuff))d3d;

		auto hr = deviceStuff->rawDevice->CreateTexture2D(&tgtDesc, nullptr, &m_swapTexture);
		if (SUCCEEDED(hr))
		{
			D3D11_RENDER_TARGET_VIEW_DESC rtDesc = CD3D11_RENDER_TARGET_VIEW_DESC(m_swapTexture.Get(), D3D11_RTV_DIMENSION_TEXTURE2D);
			deviceStuff->rawDevice->CreateRenderTargetView(m_swapTexture.Get(), &rtDesc, &m_swapRtv);
		}
		else
		{
			trace("Failed to create DUI texture, height: %i, width: %i, error: 0x%x\n", m_width, m_height, hr);
			m_swapTexture = nullptr;
		}
	}
#endif
}

CefBrowser* NUIWindow::GetBrowser()
{
	if (!m_client)
	{
		return nullptr;
	}

	return ((NUIClient*)m_client.get())->GetBrowser();
}

extern void NUI_AcceptTexture(uint64_t handle);

#include <d3d11_1.h>
#include <mmsystem.h>

void NUIWindow::TouchMessage()
{
	m_lastMessageTime = timeGetTime();
}

void NUIWindow::UpdateFrame()
{
	if (m_client)
	{
		auto browser = ((NUIClient*)m_client.get())->GetBrowser();

		if (browser)
		{
			// the CEF API has a 'is muted' getter but it doesn't work
			bool shouldMute = false;
			g_nuiGi->QueryShouldMute(shouldMute);

			if (!m_isMuted && shouldMute)
			{
				browser->GetHost()->SetAudioMuted(true);
				m_isMuted = true;
			}
			else if (m_isMuted && !shouldMute)
			{
				browser->GetHost()->SetAudioMuted(false);
				m_isMuted = false;
			}
		}
	}

	if (!GetTexture().GetRef())
	{
		return;
	}

	if (m_isPrimary)
	{		
		int resX, resY;
		g_nuiGi->GetGameResolution(&resX, &resY);

		if (IsFixedSizeWindow())
		{
			resX = 1920;
			resY = 1080;
		}

		if (m_width != resX || m_height != resY)
		{
			m_width = resX;
			m_height = resY;
			if (m_client)
			{
				if (!m_nuiTexture.GetRef())
				{
					InitializeRenderBacking();
				}

				memset(m_sharedResourceTexturesCreated, 0, sizeof(m_sharedResourceTexturesCreated));

				auto client = ((NUIClient*)m_client.get());
				auto browser = client->GetBrowser();

				if (browser)
				{
					browser->GetHost()->WasResized();
					browser->GetHost()->NotifyScreenInfoChanged();
				}
				else
				{
					client->OnClientCreated.Connect([this](NUIClient* client)
					{
						client->GetBrowser()->GetHost()->WasResized();
						client->GetBrowser()->GetHost()->NotifyScreenInfoChanged();
					});
				}
			}
		}

		for (auto& item : m_pollQueue)
		{
			NUIClient* client = static_cast<NUIClient*>(m_client.get());
			auto browser = client->GetBrowser();

			auto message = CefProcessMessage::Create("doPoll");
			auto argList = message->GetArgumentList();

			argList->SetSize(1);
			argList->SetString(0, item);

			browser->GetMainFrame()->SendProcessMessage(PID_RENDERER, message);
		}
	}

	m_pollQueue.clear();

	NUIWindowManager* wm = Instance<NUIWindowManager>::Get();
	auto texture = GetParentTexture(CefRenderHandler::PaintElementType::PET_VIEW);

	if (texture.GetRef())
	{
#ifdef GTA_FIVE
		if (!m_isPrimary)
		{
			{
				ID3D11Device* device = g_nuiGi->GetD3D11Device();

				if (device)
				{
					ID3D11DeviceContext* deviceContext = g_nuiGi->GetD3D11DeviceContext();
					assert(deviceContext);

					ID3D11Resource* nativeTexture = nullptr;
					if (auto texture = GetTexture(); texture.GetRef())
					{
						nativeTexture = (ID3D11Resource*)texture->GetNativeTexture();
					}
					ID3D11ShaderResourceView* nativeSrv = (ID3D11ShaderResourceView*)texture->GetNativeSRV();
					if (m_swapTexture && m_swapRtv && nativeSrv && nativeTexture)
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
							HRESULT hr;
							D3D11_SAMPLER_DESC sd = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
							hr = g_nuiGi->GetD3D11Device()->CreateSamplerState(&sd, &ss);
							if (FAILED(hr))
							{
								trace("DUI CreateSamplerState failed 0x%08x\n", hr);
								return;
							}

							D3D11_BLEND_DESC bd = CD3D11_BLEND_DESC(CD3D11_DEFAULT());
							hr = g_nuiGi->GetD3D11Device()->CreateBlendState(&bd, &bs);
							if (FAILED(hr))
							{
								trace("DUI CreateBlendState failed 0x%08x\n", hr);
								return;
							}

							hr = g_nuiGi->GetD3D11Device()->CreateVertexShader(fx::shaders::quadVS, sizeof(fx::shaders::quadVS), nullptr, &vs);
							if (FAILED(hr))
							{
								trace("DUI CreateVertexShader failed 0x%08x\n", hr);
								return;
							}
							hr = g_nuiGi->GetD3D11Device()->CreatePixelShader(fx::shaders::quadPS, sizeof(fx::shaders::quadPS), nullptr, &ps);
							if (FAILED(hr))
							{
								trace("DUI CreatePixelShader failed 0x%08x\n", hr);
								return;
							}
						});

						if (!ps || !vs)
						{
							return;
						}

#ifdef _DEBUG
						Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> pPerf;
						deviceContext->QueryInterface(IID_PPV_ARGS(&pPerf));
						pPerf->BeginEvent(L"DrawDUI");
#endif
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

						D3D11_RECT oldSr;
						UINT numSRs = 1;

						deviceContext->RSGetScissorRects(&numSRs, &oldSr);
						deviceContext->RSGetViewports(&numVPs, &oldVp);

						deviceContext->OMGetBlendState(&oldBs, nullptr, nullptr);

						deviceContext->PSGetShader(&oldPs, nullptr, nullptr);
						deviceContext->PSGetSamplers(0, 1, &oldSs);
						deviceContext->PSGetShaderResources(0, 1, &oldSrv);

						deviceContext->VSGetShader(&oldVs, nullptr, nullptr);

						ID3D11RenderTargetView* rtv[] = { m_swapRtv.Get() };
						deviceContext->OMSetRenderTargets(std::size(rtv), rtv, nullptr);
						deviceContext->OMSetBlendState(bs, nullptr, 0xffffffff);

						CD3D11_VIEWPORT vp = CD3D11_VIEWPORT(0.0f, 0.0f, m_width, m_height);
						deviceContext->RSSetViewports(1, &vp);

						CD3D11_RECT sr = CD3D11_RECT(0, 0, m_width, m_height);
						deviceContext->RSSetScissorRects(1, &sr);

						deviceContext->PSSetShader(ps, nullptr, 0);
						deviceContext->PSSetSamplers(0, 1, &ss);

						ID3D11ShaderResourceView* srv[] = { nativeSrv };
						deviceContext->PSSetShaderResources(0, std::size(srv), srv);

						deviceContext->VSSetShader(vs, nullptr, 0);

						D3D11_PRIMITIVE_TOPOLOGY oldTopo;
						deviceContext->IAGetPrimitiveTopology(&oldTopo);

						ID3D11InputLayout* oldLayout;
						deviceContext->IAGetInputLayout(&oldLayout);

						deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
						deviceContext->IASetInputLayout(nullptr);

						deviceContext->Draw(4, 0);

						deviceContext->CopyResource(nativeTexture, m_swapTexture.Get());

						deviceContext->OMSetRenderTargets(1, &oldRtv, oldDsv);

						deviceContext->IASetPrimitiveTopology(oldTopo);
						deviceContext->IASetInputLayout(oldLayout);

						deviceContext->VSSetShader(oldVs, nullptr, 0);
						deviceContext->PSSetShader(oldPs, nullptr, 0);
						deviceContext->PSSetSamplers(0, 1, &oldSs);
						deviceContext->PSSetShaderResources(0, 1, &oldSrv);
						deviceContext->OMSetBlendState(oldBs, nullptr, 0xffffffff);
						deviceContext->RSSetViewports(1, &oldVp);
						deviceContext->RSSetScissorRects(numSRs, &oldSr);

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

#ifdef _DEBUG
						pPerf->EndEvent();
#endif
					}
				}
			}
		}
#endif
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
		auto popupTex = GetParentTexture(CefRenderHandler::PaintElementType::PET_POPUP);
		if (popupTex.GetRef())
		{
			popupTex = nullptr;	

			std::lock_guard<std::shared_mutex> _(m_textureMutex);
			m_popupTexture = nullptr;
#ifdef IS_RDR3
			m_sharedResourceTexturesCreated[PET_POPUP] = false;
#endif
		}
	}
}

extern void TranslateWindowRect(const fwRefContainer<NUIWindow>& window, CRect* rect);

void NUIWindow::UpdateSharedResource(CefRenderHandler::PaintElementType type)
{
	// mpMenu may start queuing up frames before the game has had change to begin rendering.
	// CEF/Chromium has a limit of inflight frames and as we are responsible for releasing frames.
	// in some cases we can execeed the max limit leading to frames being flushed and putting the renderer into a weird state.
	if (!nui::g_hasFirstRender)
	{
		return;
	}

	// PET_POPUP is not currently supported in DUI.
	if (!IsPrimary() && type == PET_POPUP)
	{
		return;
	}

	auto& texRef = type == PET_VIEW ? m_nuiTexture : m_popupTexture;
	if (!texRef.GetRef())
	{
		return;
	}
	
	auto frame = this->LockFrame(type);
	if (!frame)
	{
		return;
	}

	if (!frame->shared_handle)
	{
		this->ReleaseFrame(type, frame->frame_seq);
		return;
	}

	auto sharedHandle = frame->shared_handle;

	int w = frame->width;
	int h = frame->height;
	{
		std::unique_lock<std::shared_mutex> textureLock(m_textureMutex);

		auto frameSeq = frame->frame_seq;
		CefRefPtr<CefBrowserHost> host = GetBrowser() ? GetBrowser()->GetHost() : nullptr;
		if (!m_sharedResourceTexturesCreated[type])
		{
			auto cb = [host, type, frameSeq]()
			{
				if (host)
				{
					host->ReleaseFrame(type, frameSeq);
					return false;
				}

				return true;
			};

			if (IsPrimary())
			{
				texRef = g_nuiGi->CreateTextureFromShareHandle(sharedHandle, w, h, cb);
				SetParentTexture(type, texRef);
			}
#ifdef GTA_FIVE
			else
			{
				auto duiTexRef = g_nuiGi->CreateTextureFromShareHandle(sharedHandle, w, h, cb);
				SetParentTexture(type, duiTexRef);
			}
#endif
			m_sharedResourceTexturesCreated[type] = true;
			return;
		}

		g_nuiGi->UpdateTexture(sharedHandle, !IsPrimary() ? GetParentTexture(type) : texRef, w, h, std::make_pair(frame->token_high, frame->token_low), [frameSeq, host, type]()
		{
			if (host)
			{
				host->ReleaseFrame(type, frameSeq);
				return false;
			}
			return true;
		});
	}
}

CefRect NUIWindow::GetPopupRect()
{
	auto rect = m_popupRect;

	if (IsFixedSizeWindow())
	{
		CRect baseRect;
		TranslateWindowRect(this, &baseRect);

		float scaleX = (baseRect.Width() / float(m_width));
		float scaleY = (baseRect.Height() / float(m_height));

		rect.x = (rect.x * scaleX) + baseRect.Left();
		rect.y = (rect.y * scaleY) + baseRect.Top();
		rect.width *= scaleX;
		rect.height *= scaleY;
	}

	return rect;
}

void NUIWindow::SetPopupRect(const CefRect& rect)
{
	m_popupRect = rect;

	std::lock_guard<std::shared_mutex> _(m_textureMutex);
	m_popupTexture = g_nuiGi->CreateTextureBacking(rect.width, rect.height, nui::GITextureFormat::ARGB);
}

void NUIWindow::SetPaintType(NUIPaintType type)
{
	m_paintType = type;
}

bool NUIWindow::IsFixedSizeWindow() const
{
	return nuiFixedSizeEnabled && m_name == "root";
}

static InitFunction initFunction([]
{
	static ConVar<bool> nuiFixedSize("nui_useFixedSize", ConVar_Archive | ConVar_UserPref, false, &nuiFixedSizeEnabled);
});
