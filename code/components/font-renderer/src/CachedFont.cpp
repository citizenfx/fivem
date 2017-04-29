/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "FontRendererImpl.h"

#include "memdbgon.h"

CachedFont::CachedFont(ComPtr<IDWriteFontFace> face, float emSize)
	: m_dwFace(face), m_emSize(emSize)
{

}

bool CachedFont::EnsureFaceCreated()
{
	return true;
}

ResultingSubGlyphRun::ResultingSubGlyphRun()
	: vertices(nullptr), indices(nullptr)
{

}

ResultingSubGlyphRun::~ResultingSubGlyphRun()
{
	delete[] vertices;
	delete[] indices;
}

void CachedFont::TargetGlyphRunInternal(float originX, float originY, const DWRITE_GLYPH_RUN* glyphRun, ResultingSubGlyphRun* initialRuns, ResultingVertex* initialVertices, ResultingIndex* initialIndices, CRGBA color, int& runCount)
{
	// continue on as usual
	float x = originX;

	for (uint32_t i = 0; i < glyphRun->glyphCount; i++)
	{
		uint32_t pageIndex = glyphRun->glyphIndices[i] / 256;

		fwRefContainer<CachedFontPage> page;

		if (pageIndex >= m_presentPages.size())
		{
			m_presentPages.resize(pageIndex + 1);
		}

		if (!m_presentPages[pageIndex])
		{
			page = new CachedFontPage(this, pageIndex * 256, (pageIndex + 1) * 256);
			m_presentPages[pageIndex] = true;
			m_pages[pageIndex] = page;
		}
		else
		{
			page = m_pages[pageIndex];
		}

		if (page->EnsureFontPageCreated())
		{
			ResultingSubGlyphRun* thisRun = &initialRuns[runCount];

			thisRun->texture = page->GetTexture();

			thisRun->numIndices = 6;
			thisRun->numVertices = 4;

			thisRun->vertices = &initialVertices[runCount * 4];
			thisRun->indices = &initialIndices[runCount * 6];

			auto& address = page->GetCharacterAddress(glyphRun->glyphIndices[i]);
			auto& affect = page->GetCharacterSize(glyphRun->glyphIndices[i]);

			float tX = x + affect.Left();// -0.5f;
			float tY = originY + affect.Top();// -0.5f;

			if (glyphRun->glyphOffsets)
			{
				auto offset = &glyphRun->glyphOffsets[i];

				tX += offset->advanceOffset;
				tY -= offset->ascenderOffset;
			}

			thisRun->vertices[0].x = tX;
			thisRun->vertices[0].y = tY;
			thisRun->vertices[0].u = address.fX1;
			thisRun->vertices[0].v = address.fY1;
			thisRun->vertices[0].color = color;

			thisRun->vertices[1].x = tX + affect.Width();
			thisRun->vertices[1].y = tY;
			thisRun->vertices[1].u = address.fX2;
			thisRun->vertices[1].v = address.fY1;
			thisRun->vertices[1].color = color;

			thisRun->vertices[2].x = tX + affect.Width();
			thisRun->vertices[2].y = tY + affect.Height();
			thisRun->vertices[2].u = address.fX2;
			thisRun->vertices[2].v = address.fY2;
			thisRun->vertices[2].color = color;

			thisRun->vertices[3].x = tX;
			thisRun->vertices[3].y = tY + affect.Height();
			thisRun->vertices[3].u = address.fX1;
			thisRun->vertices[3].v = address.fY2;
			thisRun->vertices[3].color = color;

			thisRun->indices[0] = 0;
			thisRun->indices[1] = 1;
			thisRun->indices[2] = 2;

			thisRun->indices[3] = 2;
			thisRun->indices[4] = 3;
			thisRun->indices[5] = 0;

			thisRun->order = runCount;

			runCount++;
		}

		x += glyphRun->glyphAdvances[i];
	}
}

ResultingGlyphRun* CachedFont::TargetGlyphRun(float originX, float originY, const DWRITE_GLYPH_RUN* glyphRun, const DWRITE_GLYPH_RUN_DESCRIPTION* glyphRunDescription, CRGBA color)
{
	// check if we're colored
	bool isColor = false;
	ComPtr<IDWriteColorGlyphRunEnumerator> colorEnumerator;
	ComPtr<IDWriteFactory2> factory = g_fontRenderer.GetDWriteFactory2();

	if (factory.Get())
	{
		HRESULT hr = factory->TranslateColorGlyphRun(originX, originY, glyphRun, glyphRunDescription, DWRITE_MEASURING_MODE_GDI_NATURAL, nullptr, 0, colorEnumerator.GetAddressOf());

		if (SUCCEEDED(hr)) // i.e. not some runtime error/DWRITE_E_NOCOLOR
		{
			isColor = true;
		}
	}

	// target the actual vertices
	int runCount = 0;
	ResultingSubGlyphRun* initialRuns;
	ResultingVertex* initialVertices;
	ResultingIndex* initialIndices;

	if (!isColor)
	{
		initialRuns = new ResultingSubGlyphRun[glyphRun->glyphCount + 1];
		initialVertices = new ResultingVertex[glyphRun->glyphCount * 4];
		initialIndices = new ResultingIndex[glyphRun->glyphCount * 6];

		TargetGlyphRunInternal(originX, originY, glyphRun, initialRuns, initialVertices, initialIndices, color, runCount);
	}
	else
	{
		// assume there's 6 layers at most on average
		initialRuns = new ResultingSubGlyphRun[(glyphRun->glyphCount * 20) + 1];
		initialVertices = new ResultingVertex[glyphRun->glyphCount * 20 * 4];
		initialIndices = new ResultingIndex[glyphRun->glyphCount * 20 * 6];

		BOOL hasRun = TRUE;
		
		colorEnumerator->MoveNext(&hasRun);

		while(hasRun)
		{
			const DWRITE_COLOR_GLYPH_RUN* colorGlyphRun;
			colorEnumerator->GetCurrentRun(&colorGlyphRun);

			CRGBA curColor = (colorGlyphRun->paletteIndex == 0xFFFF) ? color : CRGBA::FromFloat(colorGlyphRun->runColor.r, colorGlyphRun->runColor.g, colorGlyphRun->runColor.b, colorGlyphRun->runColor.a);

			TargetGlyphRunInternal(colorGlyphRun->baselineOriginX, colorGlyphRun->baselineOriginY, &colorGlyphRun->glyphRun, initialRuns, initialVertices, initialIndices, curColor, runCount);

			colorEnumerator->MoveNext(&hasRun);
		}
	}

	if (runCount == 0)
	{
		delete[] initialRuns;
		delete[] initialIndices;
		delete[] initialVertices;

		return nullptr;
	}

	// compress the runs
	qsort(initialRuns, runCount, sizeof(*initialRuns), [] (const void* a, const void* b) -> int
	{
		auto left = static_cast<const ResultingSubGlyphRun*>(a);
		auto right = static_cast<const ResultingSubGlyphRun*>(b);

		if (left->texture == right->texture)
		{
			if (left->order == right->order)
			{
				return 0;
			}
			return (left->order < right->order) ? -1 : 1;
		}

		return (left->texture < right->texture) ? -1 : 1;
	});

	initialRuns[runCount].texture = nullptr;

	ResultingGlyphRun* resultRun = new ResultingGlyphRun[1];
	resultRun->numSubRuns = 0;
	resultRun->subRuns = new ResultingSubGlyphRun[runCount];

	if (runCount > 0)
	{
		ResultingSubGlyphRun* firstRun = &initialRuns[0];
		ResultingSubGlyphRun* lastRun = &initialRuns[0];

		for (int i = 0; i < runCount + 1; i++)
		{
			auto thisRun = &initialRuns[i];

			if (lastRun->texture != thisRun->texture)
			{
				int numIndices = firstRun->numIndices;
				int numVertices = firstRun->numVertices;

				for (auto run = firstRun + 1; run <= lastRun; run++)
				{
					for (uint32_t i = 0; i < run->numIndices; i++)
					{
						run->indices[i] += numVertices;
					}

					numIndices += run->numIndices;
					numVertices += run->numVertices;
				}

				auto vertices = new ResultingVertex[numVertices];
				auto indices = new ResultingIndex[numIndices];

				int vi = 0;
				int ii = 0;

				for (auto run = firstRun; run <= lastRun; run++)
				{
					memcpy(&vertices[vi], run->vertices, run->numVertices * sizeof(ResultingVertex));
					vi += run->numVertices;

					memcpy(&indices[ii], run->indices, run->numIndices * sizeof(ResultingIndex));
					ii += run->numIndices;

					run->indices = nullptr;
					run->vertices = nullptr;
				}

				firstRun->numIndices = numIndices;
				firstRun->numVertices = numVertices;

				firstRun->vertices = vertices;
				firstRun->indices = indices;

				resultRun->subRuns[resultRun->numSubRuns++] = *firstRun;

				// clean up so they won't get freed when the array gets delete[]'d
				firstRun->vertices = nullptr;
				firstRun->indices = nullptr;

				firstRun = nullptr;
			}

			if (firstRun == nullptr)
			{
				firstRun = thisRun;
			}

			lastRun = thisRun;
		}
	}
	
	delete[] initialRuns;
	delete[] initialIndices;
	delete[] initialVertices;

	return resultRun;
}