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

		return new GtaFontTexture(texture);
	}

	return new GtaFontTexture(nullptr);
}

void GtaGameInterface::SetTexture(FontRendererTexture* texture)
{
	SetTextureGtaIm(static_cast<GtaFontTexture*>(texture)->GetTexture());

	SetRenderState(0, 0);
	SetRenderState(2, 0); // alpha blending

	PushDrawBlitImShader();
}

void GtaGameInterface::DrawIndexedVertices(int numVertices, int numIndices, FontRendererVertex* vertices, uint16_t* indices)
{
	BeginImVertices(3, numIndices);

	for (int j = 0; j < numIndices; j++)
	{
		auto vertex = &vertices[indices[j]];
		uint32_t color = *(uint32_t*)&vertex->color;
		color = (color & 0xFF00FF00) | (vertex->color.blue | vertex->color.red << 16);

		AddImVertex(vertex->x, vertex->y, 0.0, 0.0, 0.0, -1.0, color, vertex->u, vertex->v);
	}

	DrawImVertices();
}

void GtaGameInterface::UnsetTexture()
{
	PopDrawBlitImShader();
}

void GtaGameInterface::InvokeOnRender(void(*cb)(void*), void* arg)
{
	if (IsOnRenderThread())
	{
		cb(arg);
	}
	else
	{
		auto dc = new(0) CGenericDC1Arg((void(*)(int))cb, (int)arg);
		dc->Enqueue();
	}
}

static GtaGameInterface g_gtaGameInterface;

FontRendererGameInterface* CreateGameInterface()
{
	return &g_gtaGameInterface;
}

