#pragma once
#include <stdint.h>

#include "RGBA.h"

enum class FontRendererTextureFormat
{
	DXT5,
	ARGB
};

class FontRendererTexture
{
public:
	virtual ~FontRendererTexture() = 0;
};

struct FontRendererVertex
{
	float x;
	float y;
	float u;
	float v;
	CRGBA color;
};

class FontRendererGameInterface
{
public:
	virtual FontRendererTexture* CreateTexture(int width, int height, FontRendererTextureFormat format, void* pixelData) = 0;

	virtual void SetTexture(FontRendererTexture* texture) = 0;

	virtual void DrawIndexedVertices(int numVertices, int numIndices, FontRendererVertex* vertices, uint16_t* indices) = 0;

	virtual void UnsetTexture() = 0;

	virtual void InvokeOnRender(void(*cb)(void*), void* arg) = 0;
};
