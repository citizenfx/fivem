/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "FontRendererImpl.h"
#include "3rdparty/dxt.h"
#include <ppltasks.h>

#include <Error.h>

#include "memdbgon.h"

FontRendererTexture::~FontRendererTexture()
{ }

CachedFontPage::CachedFontPage(CachedFont* parent, uint32_t minCodePoint, uint32_t maxCodePoint)
	: m_owningFont(parent), m_minCodePoint(minCodePoint), m_maxCodePoint(maxCodePoint), m_pixelData(nullptr), m_enqueued(false)
{

}

const CRect& CachedFontPage::GetCharacterAddress(uint32_t codePoint)
{
	return m_characterAddresses[codePoint - m_minCodePoint].address;
}

const CRect& CachedFontPage::GetCharacterSize(uint32_t codePoint)
{
	return m_characterAddresses[codePoint - m_minCodePoint].affect;
}

bool CachedFontPage::EnsureFontPageCreated()
{
	// if we haven't set pixel data yet
	if (!m_pixelData)
	{
		if (!m_enqueued)
		{
			EnqueueCreateFontPage();
		}

		return false;
	}

	// return true if we're dequeued
	return !m_enqueued;
}

void CachedFontPage::EnqueueCreateFontPage()
{
	m_enqueued = true;

#if defined(GTA_NY)
	concurrency::create_task([=] ()
	{
		CreateNow();
	}).then([=] ()
	{
		// clear the queued flag
		m_enqueued = false;
	});
#else
	CreateNow();

	m_enqueued = false;
#endif
}

void CachedFontPage::CreateNow()
{
	// set up character data
	for (uint32_t i = m_minCodePoint; i < m_maxCodePoint; i++)
	{
		CreateCharacter(i);
	}

	// prepare data for the texture
	float maxHeight = 0;

	for (auto& character : m_characterAddresses)
	{
		float height = character.affect.Height();

		if (height > maxHeight)
		{
			maxHeight = height;
		}
	}

	maxHeight += 8;

	// make an initial buffer assuming a width/height of 1024x256
	uint32_t* pixelData = (uint32_t*)_aligned_malloc(1024 * 256 * 4, 16);
	memset(pixelData, 0, 1024 * 256 * 4);

	int x = 0;
	int y = 0;

	int texWidth = 1024;
	int texHeight = 256;

	for (auto& character : m_characterAddresses)
	{
		float width = character.affect.Width();
		float height = character.affect.Height();

		// check for vertical increment
		if ((x + 8 + width) > texWidth)
		{
			x = 0;
			y += (int)maxHeight;

			// check for vertical expansion
			if ((y + maxHeight) > texHeight)
			{
				pixelData = (uint32_t*)_aligned_realloc(pixelData, texWidth * (texHeight + 256) * 4, 16);
				memset(pixelData + (texWidth * texHeight), 0, 256 * texWidth * 4);

				texHeight += 256;
			}
		}

		// blit the image
		for (int sy = 0; sy < height; sy++)
		{
			int dy = (y + 4) + sy;

			for (int sx = 0; sx < width; sx++)
			{
				int dx = (x + 4) + sx;

				uint32_t* dest = &pixelData[(dy * texWidth) + dx];
				uint8_t* src = &character.pixelData[(sy * (int)width) + sx];

				*dest = (*src << 24) | 0xFFFFFF;
				//*dest = (*src) | (*src << 8) | (*src << 16) | (0xFF << 24);
			}
		}

		character.address.SetRect((float)(x + 4), (float)(y + 4), (float)(x + 4 + width), (float)(y + 4 + height));

		x += (int)width + 8;

		// delete the character's pixel data
		delete[] character.pixelData;
	}

#if defined(USE_ARGB8)
	m_targetTexture = g_fontRenderer.GetGameInterface()->CreateTexture(texWidth, texHeight, FontRendererTextureFormat::ARGB, pixelData);
#else
	uint32_t* inPixelData = pixelData;
	pixelData = (uint32_t*)_aligned_malloc(texWidth * texHeight, 16);

	int outputBytes;
	CompressImageDXT5((byte*)inPixelData, (byte*)pixelData, texWidth, texHeight, outputBytes);

	_aligned_free(inPixelData);

	m_targetTexture = g_fontRenderer.GetGameInterface()->CreateTexture(texWidth, texHeight, FontRendererTextureFormat::DXT5, pixelData);
#endif
	m_pixelData = pixelData;

	// store addresses
	for (auto& character : m_characterAddresses)
	{
		character.address.SetRect(character.address.fX1 / texWidth, character.address.fY1 / texHeight, character.address.fX2 / texWidth, character.address.fY2 / texHeight);
	}
}

void CachedFontPage::CreateCharacter(uint32_t codePoint)
{
	DWRITE_GLYPH_RUN dummyRun = { 0 };
	UINT16 glyphIndex = codePoint;

	dummyRun.fontFace = m_owningFont->m_dwFace.Get();
	dummyRun.glyphIndices = &glyphIndex;
	dummyRun.glyphCount = 1;

	dummyRun.fontEmSize = m_owningFont->m_emSize;

	ComPtr<IDWriteGlyphRunAnalysis> glyphRunAnalysis;

	if (FAILED(g_fontRenderer.GetDWriteFactory()->CreateGlyphRunAnalysis(&dummyRun, 1.0f, nullptr, DWRITE_RENDERING_MODE_GDI_NATURAL, DWRITE_MEASURING_MODE_GDI_NATURAL, 0.0f, 0.0f, glyphRunAnalysis.GetAddressOf())))
	{
		FatalError("glyphrunanalysis failed");
	}

	RECT textureBounds;
	glyphRunAnalysis->GetAlphaTextureBounds(DWRITE_TEXTURE_CLEARTYPE_3x1, &textureBounds);

	CRect boundsBits((float)textureBounds.left, (float)textureBounds.top, (float)textureBounds.right, (float)textureBounds.bottom);

	uint8_t* piecePixelData;
	uint8_t* pixelData;
	uint32_t pixelDataSize = (uint32_t)(boundsBits.Width() * boundsBits.Height());
	uint32_t piecePixelDataSize = pixelDataSize * 3;

	pixelData = new uint8_t[pixelDataSize];
	piecePixelData = new uint8_t[piecePixelDataSize];

	glyphRunAnalysis->CreateAlphaTexture(DWRITE_TEXTURE_CLEARTYPE_3x1, &textureBounds, piecePixelData, piecePixelDataSize);

	// normalize to grayscale
	for (uint32_t i = 0; i < pixelDataSize; i++)
	{
		int r = piecePixelData[i * 3];
		int g = piecePixelData[(i * 3) + 1];
		int b = piecePixelData[(i * 3) + 2];

		pixelData[i] = (r + g + b) / 3;
	}

	delete[] piecePixelData;

	// set initial data
	FontCharacter addressData;
	addressData.affect = boundsBits;
	addressData.pixelData = pixelData;

	m_characterAddresses.push_back(addressData);
}