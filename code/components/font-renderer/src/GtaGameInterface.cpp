/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "FontRendererImpl.h"
#include <DrawCommands.h>
#include <grcTexture.h>

#include "memdbgon.h"

class GtaGameInterface : public FontRendererGameInterface
{
private:
#ifdef _HAVE_GRCORE_NEWSTATES
	uint32_t m_oldBlendState;

	uint32_t m_oldRasterizerState;

	uint32_t m_oldDepthStencilState;

	uint32_t m_oldSamplerState;

	uint32_t m_pointSamplerState;
#endif

public:
	virtual FontRendererTexture* CreateTexture(int width, int height, FontRendererTextureFormat format, void* pixelData);

	virtual void SetTexture(FontRendererTexture* texture);

	virtual void UnsetTexture();

	virtual void DrawIndexedVertices(int numVertices, int numIndices, FontRendererVertex* vertex, uint16_t* indices);

	virtual void InvokeOnRender(void(*cb)(void*), void* arg);

	virtual void DrawRectangles(int numRectangles, const ResultingRectangle* rectangles);

#ifdef _HAVE_GRCORE_NEWSTATES
	inline void SetPointSamplerState(uint32_t state)
	{
		m_pointSamplerState = state;
	}
#endif
};

class GtaFontTexture : public FontRendererTexture
{
private:
	rage::grcTexture* m_texture;

public:
	GtaFontTexture(rage::grcTexture* texture)
		: m_texture(texture)
	{

	}

	virtual ~GtaFontTexture()
	{

	}

	inline rage::grcTexture* GetTexture() { return m_texture; }
};

FontRendererTexture* GtaGameInterface::CreateTexture(int width, int height, FontRendererTextureFormat format, void* pixelData)
{
	if (!IsRunningTests())
	{
#if defined(GTA_NY)
		// odd NY-specific stuff to force DXT5
		*(uint32_t*)0x10C45F8 = D3DFMT_DXT5;
		rage::grcTexture* texture = rage::grcTextureFactory::getInstance()->createManualTexture(width, height, (format == FontRendererTextureFormat::ARGB) ? FORMAT_A8R8G8B8 : 0, 0, nullptr);
		*(uint32_t*)0x10C45F8 = 0;

		int pixelSize = (format == FontRendererTextureFormat::ARGB) ? 4 : 1;

		// copy texture data
		D3DLOCKED_RECT lockedRect;
		texture->m_pITexture->LockRect(0, &lockedRect, nullptr, D3DLOCK_DISCARD);

		memcpy(lockedRect.pBits, pixelData, width * height * pixelSize);

		texture->m_pITexture->UnlockRect(0);

		// store pixel data so the game can reload the texture
		texture->m_pixelData = pixelData;
#else
		rage::grcTextureReference reference;
		memset(&reference, 0, sizeof(reference));
		reference.width = width;
		reference.height = height;
		reference.depth = 1;
		reference.stride = width;
		reference.format = 3; // dxt5?
		reference.pixelData = (uint8_t*)pixelData;

		rage::grcTexture* texture = rage::grcTextureFactory::getInstance()->createImage(&reference, nullptr);
#endif

		return new GtaFontTexture(texture);
	}

	return new GtaFontTexture(nullptr);
}

void GtaGameInterface::SetTexture(FontRendererTexture* texture)
{
#ifdef _HAVE_GRCORE_NEWSTATES
	m_oldSamplerState = GetImDiffuseSamplerState();

	SetImDiffuseSamplerState(m_pointSamplerState);
#endif

	SetTextureGtaIm(static_cast<GtaFontTexture*>(texture)->GetTexture());

#ifndef _HAVE_GRCORE_NEWSTATES
	SetRenderState(0, grcCullModeNone); // 0 in NY, 1 in Payne
	SetRenderState(2, 0); // alpha blending
#else
	m_oldRasterizerState = GetRasterizerState();
	SetRasterizerState(GetStockStateIdentifier(RasterizerStateNoCulling));

	m_oldBlendState = GetBlendState();
	SetBlendState(GetStockStateIdentifier(BlendStateDefault));

	m_oldDepthStencilState = GetDepthStencilState();
	SetDepthStencilState(GetStockStateIdentifier(DepthStencilStateNoDepth));
#endif

	PushDrawBlitImShader();
}

void GtaGameInterface::DrawIndexedVertices(int numVertices, int numIndices, FontRendererVertex* vertices, uint16_t* indices)
{
#if GTA_NY
	IDirect3DDevice9* d3dDevice = *(IDirect3DDevice9**)0x188AB48;

	d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
#endif

	BeginImVertices(3, numIndices);

	for (int j = 0; j < numIndices; j++)
	{
		auto vertex = &vertices[indices[j]];
		uint32_t color = *(uint32_t*)&vertex->color;

		// this swaps ABGR (as CRGBA is ABGR in little-endian) to ARGB by rotating left
		if (!rage::grcTexture::IsRenderSystemColorSwapped())
		{
			color = (color & 0xFF00FF00) | _rotl(color & 0x00FF00FF, 16);
		}

		AddImVertex(vertex->x, vertex->y, 0.0, 0.0, 0.0, -1.0, color, vertex->u, vertex->v);
	}

	DrawImVertices();

#if GTA_NY
	d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
	d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
#endif
}

void GtaGameInterface::DrawRectangles(int numRectangles, const ResultingRectangle* rectangles)
{
	SetTextureGtaIm(rage::grcTextureFactory::GetNoneTexture());

#ifndef _HAVE_GRCORE_NEWSTATES
	SetRenderState(0, grcCullModeNone);
	SetRenderState(2, 0); // alpha blending m8
#else
	auto oldRasterizerState = GetRasterizerState();
	SetRasterizerState(GetStockStateIdentifier(RasterizerStateNoCulling));

	auto oldBlendState = GetBlendState();
	SetBlendState(GetStockStateIdentifier(BlendStateDefault));

	auto oldDepthStencilState = GetDepthStencilState();
	SetDepthStencilState(GetStockStateIdentifier(DepthStencilStateNoDepth));
#endif

	PushDrawBlitImShader();

	for (int i = 0; i < numRectangles; i++)
	{
		auto rectangle = &rectangles[i];

		BeginImVertices(4, 4);

		auto& rect = rectangle->rectangle;
		uint32_t color = *(uint32_t*)&rectangle->color;

		// this swaps ABGR (as CRGBA is ABGR in little-endian) to ARGB by rotating left
		if (!rage::grcTexture::IsRenderSystemColorSwapped())
		{
			color = (color & 0xFF00FF00) | _rotl(color & 0x00FF00FF, 16);
		}

		AddImVertex(rect.fX1, rect.fY1, 0.0f, 0.0f, 0.0f, -1.0f, color, 0.0f, 0.0f);
		AddImVertex(rect.fX2, rect.fY1, 0.0f, 0.0f, 0.0f, -1.0f, color, 0.0f, 0.0f);
		AddImVertex(rect.fX1, rect.fY2, 0.0f, 0.0f, 0.0f, -1.0f, color, 0.0f, 0.0f);
		AddImVertex(rect.fX2, rect.fY2, 0.0f, 0.0f, 0.0f, -1.0f, color, 0.0f, 0.0f);

		DrawImVertices();
	}

	PopDrawBlitImShader();

#ifdef _HAVE_GRCORE_NEWSTATES
	SetRasterizerState(oldRasterizerState);

	SetBlendState(oldBlendState);

	SetDepthStencilState(oldDepthStencilState);
#endif
}

void GtaGameInterface::UnsetTexture()
{
	PopDrawBlitImShader();

#ifdef _HAVE_GRCORE_NEWSTATES
	SetRasterizerState(m_oldRasterizerState);
	SetBlendState(m_oldBlendState);
	SetDepthStencilState(m_oldDepthStencilState);
	SetImDiffuseSamplerState(m_oldSamplerState);
#endif
}

#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

void GtaGameInterface::InvokeOnRender(void(*cb)(void*), void* arg)
{
	if (IsRunningTests())
	{
		return;
	}

	if (IsOnRenderThread())
	{
		cb(arg);
	}
	else
	{
#if defined(GTA_NY)
		int argRef = (int)arg;

		auto dc = new(0) CGenericDC1Arg((void(*)(int))cb, &argRef);
		dc->Enqueue();
#else
		uintptr_t argRef = (uintptr_t)arg;

		EnqueueGenericDrawCommand([] (uintptr_t a, uintptr_t b)
		{
			D3DPERF_BeginEvent(D3DCOLOR_ARGB(0, 0, 0, 0), L"FontRenderer");

			auto cb = (void(*)(void*))a;

			cb((void*)b);

			D3DPERF_EndEvent();
		}, (uintptr_t*)&cb, &argRef);
#endif
	}
}

static GtaGameInterface g_gtaGameInterface;

FontRendererGameInterface* CreateGameInterface()
{
	return &g_gtaGameInterface;
}

#include <random>

static InitFunction initFunction([] ()
{
	static std::random_device random_core;
	static std::mt19937 random(random_core());

#ifdef _HAVE_GRCORE_NEWSTATES
	OnGrcCreateDevice.Connect([] ()
	{
		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		samplerDesc.MaxAnisotropy = 0;

		g_gtaGameInterface.SetPointSamplerState(CreateSamplerState(&samplerDesc));
	});
#endif

	OnPostFrontendRender.Connect([=] ()
	{
#if defined(GTA_FIVE)
		int x, y;
		GetGameResolution(x, y);

		SYSTEMTIME systemTime;
		GetLocalTime(&systemTime);

		const wchar_t* brandingString = L"";

		switch (systemTime.wHour)
		{
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
				brandingString = L"FiveM \xD83C\xDF19";
				break;
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
				brandingString = L"FiveM \xD83C\xDF42";
				break;
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
			case 18:
				brandingString = L"FiveM \xD83D\xDD25";
				break;
			case 19:
			case 20:
			case 21:
			case 22:
			case 23:
			case 0:
				brandingString = L"FiveM \xD83C\xDF59";
				break;
		}

		static CRect metrics;
		
		if (metrics.Width() <= 0.1f)
		{
			g_fontRenderer.GetStringMetrics(brandingString, 18.0f, 1.0f, "Segoe UI", metrics);
		}

		CRect drawRect(x - metrics.Width() - 10.0f, 10.0f, x, y);
		CRGBA color(180, 180, 180);

		g_fontRenderer.DrawText(brandingString, drawRect, color, 18.0f, 1.0f, "Segoe UI");
#endif

		g_fontRenderer.DrawPerFrame();
	}, 1000);
});