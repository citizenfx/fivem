/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once
#include <stdint.h>

#include "RGBA.h"
#include "FontRendererAllocator.h"

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

struct FontRendererVertex : public FrpUseSequentialAllocator
{
	float x;
	float y;
	float u;
	float v;
	CRGBA color;
};

struct ResultingRectangle : public FrpUseSequentialAllocator
{
	CRect rectangle;
	CRGBA color;
};

struct ResultingRectangles : public FrpUseSequentialAllocator
{
	int count;
	ResultingRectangle* rectangles;
};

class FontRendererGameInterface
{
public:
	virtual FontRendererTexture* CreateTexture(int width, int height, FontRendererTextureFormat format, void* pixelData) = 0;

	virtual void SetTexture(FontRendererTexture* texture) = 0;

	virtual void DrawIndexedVertices(int numVertices, int numIndices, FontRendererVertex* vertices, uint16_t* indices) = 0;

	virtual void DrawRectangles(int numRectangles, const ResultingRectangle* rectangles) = 0;

	virtual void UnsetTexture() = 0;

	virtual void InvokeOnRender(void(*cb)(void*), void* arg) = 0;
};
