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

class GtaGameInterface : public FontRendererGameInterface
{
public:
	virtual FontRendererTexture* CreateTexture(int width, int height, FontRendererTextureFormat format, void* pixelData);

	virtual void SetTexture(FontRendererTexture* texture);

	virtual void UnsetTexture();

	virtual void DrawIndexedVertices(int numVertices, int numIndices, FontRendererVertex* vertex, uint16_t* indices);

	virtual void InvokeOnRender(void(*cb)(void*), void* arg);

	virtual void DrawRectangles(int numRectangles, const ResultingRectangle* rectangles);
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
	SetTextureGtaIm(static_cast<GtaFontTexture*>(texture)->GetTexture());

	SetRenderState(0, grcCullModeNone); // 0 in NY, 1 in Payne
	SetRenderState(2, 0); // alpha blending

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
		color = (color & 0xFF00FF00) | _rotl(color & 0x00FF00FF, 16);

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

	SetRenderState(0, grcCullModeNone);
	SetRenderState(2, 0); // alpha blending m8

	PushDrawBlitImShader();

	for (int i = 0; i < numRectangles; i++)
	{
		auto rectangle = &rectangles[i];

		BeginImVertices(4, 4);

		auto& rect = rectangle->rectangle;
		uint32_t color = *(uint32_t*)&rectangle->color;

		// this swaps ABGR (as CRGBA is ABGR in little-endian) to ARGB by rotating left
		color = (color & 0xFF00FF00) | _rotl(color & 0x00FF00FF, 16);

		AddImVertex(rect.fX1, rect.fY1, 0.0f, 0.0f, 0.0f, -1.0f, color, 0.0f, 0.0f);
		AddImVertex(rect.fX2, rect.fY1, 0.0f, 0.0f, 0.0f, -1.0f, color, 0.0f, 0.0f);
		AddImVertex(rect.fX1, rect.fY2, 0.0f, 0.0f, 0.0f, -1.0f, color, 0.0f, 0.0f);
		AddImVertex(rect.fX2, rect.fY2, 0.0f, 0.0f, 0.0f, -1.0f, color, 0.0f, 0.0f);

		DrawImVertices();
	}

	PopDrawBlitImShader();
}

void GtaGameInterface::UnsetTexture()
{
	PopDrawBlitImShader();
}

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
		uint32_t argRef = (uint32_t)arg;

		EnqueueGenericDrawCommand([] (uint32_t a, uint32_t b)
		{
			auto cb = (void(*)(void*))a;

			cb((void*)b);
		}, (uint32_t*)&cb, &argRef);
#endif
	}
}

static GtaGameInterface g_gtaGameInterface;

FontRendererGameInterface* CreateGameInterface()
{
	return &g_gtaGameInterface;
}

static InitFunction initFunction([] ()
{
	OnPostFrontendRender.Connect([] ()
	{
		/*CRect rect(5, 5, 705, 5);
		CRGBA color(255, 255, 255);

		wchar_t russian[] = { 0x041f, 0x0440, 0x0438, 0x0432, 0x0435, 0x0442, 0x0020, 0x043c, 0x0438, 0x0440, 0x0000 };
		wchar_t chinese[] = { 0x4e16, 0x754c, 0x60a8, 0x597d, 0x0000 };
		wchar_t greek[] = { 0x0393, 0x03b5, 0x03b9, 0x03b1, 0x0020, 0x03c3, 0x03b1, 0x03c2, 0x0020, 0x03ba, 0x03cc, 0x03c3, 0x03bc, 0x03bf, 0x0000 };
		wchar_t japanese[] = { 0x4eca, 0x65e5, 0x306f, 0x4e16, 0x754c, 0x0000 };
		wchar_t runic[] = { 0x16ba, 0x16d6, 0x16da, 0x16df, 0x0020, 0x16b9, 0x16df, 0x16c9, 0x16da, 0x16de, 0x0000 };

		static wchar_t str[128];

		if (!str[0])
		{
			FILE* f = fopen("A:/lolunicode.txt", "rb");
			fread(str, 1, sizeof(str), f);
			fclose(f);
		}

		TheFonts->DrawText(va(L"\xD83C\xDF4E @ \xD83C\xDF55... Hi! O\x448\x438\x431\x43A\x430... %s %s %s %s %s", russian, chinese, greek, japanese, runic), rect, color, 44.0f, 1.0f, "Segoe UI");

		rect.SetRect(5, 205, 705, 205);

		TheFonts->DrawText(str, rect, color, 14.0f, 1.0f, "Segoe UI");

		rect.SetRect(500.0f, 500.0f, 700.0f, 700.0f);

		TheFonts->DrawRectangle(rect, color);*/

		g_fontRenderer.DrawPerFrame();
	}, 1000);
});