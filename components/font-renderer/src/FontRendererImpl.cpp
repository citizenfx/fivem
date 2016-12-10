/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <FontRendererImpl.h>
#include "Hooking.h"

#include "memdbgon.h"

void FontRendererImpl::Initialize()
{
	trace("[FontRenderer] Initializing DirectWrite.\n");

	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)m_dwFactory.GetAddressOf());

	// attempt to get a IDWriteFactory2
	HRESULT hr = m_dwFactory.As(&m_dwFactory2);

	if (FAILED(hr))
	{
		trace("[FontRenderer] IDWriteFactory2 unavailable (hr=%08x), colored font rendering will not be used\n", hr);
	}

	m_gameInterface = CreateGameInterface();

	CreateTextRenderer();
}

ResultingGlyphRun::~ResultingGlyphRun()
{
	delete[] subRuns;
}

CitizenDrawingContext::~CitizenDrawingContext() {}

std::string FontRendererImpl::GetFontKey(ComPtr<IDWriteFontFace> fontFace, float emSize)
{
	// get the file reference key
	uint32_t reqFiles = 1;
	IDWriteFontFile* file;

	fontFace->GetFiles(&reqFiles, &file);

	const void* referenceKey;
	uint32_t referenceKeySize;

	file->GetReferenceKey(&referenceKey, &referenceKeySize);

	// store in a buffer and append the size
	uint32_t referenceKeySizeTarget = min(referenceKeySize, (uint32_t)128);

	char refKeyBuffer[256];
	memcpy(refKeyBuffer, referenceKey, referenceKeySizeTarget);

	*(float*)&refKeyBuffer[referenceKeySizeTarget] = emSize;

	// release the file
	file->Release();

	return std::string(refKeyBuffer, referenceKeySizeTarget + 4);
}

fwRefContainer<CachedFont> FontRendererImpl::GetFontInstance(ComPtr<IDWriteFontFace> fontFace, float emSize)
{
	auto key = GetFontKey(fontFace, emSize);

	auto it = m_fontCache.find(key);

	if (it != m_fontCache.end())
	{
		return it->second;
	}

	fwRefContainer<CachedFont> cachedFont = new CachedFont(fontFace, emSize);
	m_fontCache[key] = cachedFont;

	return cachedFont;
}

void FontRendererImpl::DrawRectangle(const CRect& rect, const CRGBA& color)
{
	ResultingRectangle resultRect;
	resultRect.rectangle = rect;
	resultRect.color = color;

	m_mutex.lock();
	m_queuedRectangles.push_back(resultRect);
	m_mutex.unlock();
}

void FontRendererImpl::DrawText(fwWString text, const CRect& rect, const CRGBA& color, float fontSize, float fontScale, fwString fontRef)
{
	// wait for a swap to complete
	FrpSeqAllocatorWaitForSwap();

	m_mutex.lock();

	// create or find a text format
	ComPtr<IDWriteTextFormat> textFormat;

	auto formatKey = std::make_pair(fontRef, fontSize);
	auto formatIter = m_textFormatCache.find(formatKey);

	if (formatIter != m_textFormatCache.end())
	{
		textFormat = formatIter->second;
	}
	else
	{
		wchar_t fontRefWide[128];
		mbstowcs(fontRefWide, fontRef.c_str(), _countof(fontRefWide));

		m_dwFactory->CreateTextFormat(fontRefWide, nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-us", textFormat.GetAddressOf());

		m_textFormatCache[formatKey] = textFormat;
	}

	// create or find a cached text layout
	ComPtr<IDWriteTextLayout> textLayout;

	auto layoutKey = std::make_pair(textFormat.Get(), std::make_pair(color.AsARGB(), text));
	auto layoutIter = m_textLayoutCache.find(layoutKey);

	if (layoutIter != m_textLayoutCache.end())
	{
		textLayout = layoutIter->second;
	}
	else
	{
		m_dwFactory->CreateTextLayout(text.c_str(), text.length(), textFormat.Get(), rect.Width(), rect.Height(), textLayout.GetAddressOf());

		m_textLayoutCache[layoutKey] = textLayout;

		// set effect
		DWRITE_TEXT_RANGE effectRange = { 0, UINT32_MAX };
		ComPtr<CitizenDrawingEffect> effect = Make<CitizenDrawingEffect>();

		effect->SetColor(color);

		textLayout->SetDrawingEffect((IUnknown*)effect.Get(), effectRange);
	}

	// draw
	auto drawingContext = new CitizenDrawingContext();
	textLayout->Draw(drawingContext, m_textRenderer.Get(), rect.Left(), rect.Top());

	auto numRuns = drawingContext->glyphRuns.size();

	if (numRuns)
	{
		for (auto& run : drawingContext->glyphRuns)
		{
			m_queuedGlyphRuns.push_back(run);
		}
	}

	delete drawingContext;

	m_mutex.unlock();
}

bool FontRendererImpl::GetStringMetrics(fwWString characterString, float fontSize, float fontScale, fwString fontRef, CRect& outRect)
{
	wchar_t fontRefWide[128];
	mbstowcs(fontRefWide, fontRef.c_str(), _countof(fontRefWide));

	ComPtr<IDWriteTextFormat> textFormat;
	m_dwFactory->CreateTextFormat(fontRefWide, nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-us", textFormat.GetAddressOf());

	ComPtr<IDWriteTextLayout> textLayout;
	m_dwFactory->CreateTextLayout(characterString.c_str(), characterString.length(), textFormat.Get(), 8192.0, 8192.0, textLayout.GetAddressOf());

	DWRITE_TEXT_METRICS textMetrics;
	textLayout->GetMetrics(&textMetrics);

	outRect.SetRect(textMetrics.left, textMetrics.top, textMetrics.left + textMetrics.width, textMetrics.top + textMetrics.height);

	return true;
}

void FontRendererImpl::DrawPerFrame()
{
	m_mutex.lock();

	// draw rectangles
	auto numRectangles = m_queuedRectangles.size();

	if (numRectangles)
	{
		// allocate a structure
		ResultingRectangles* rectangles = new ResultingRectangles();
		rectangles->count = numRectangles;

		rectangles->rectangles = new ResultingRectangle[numRectangles];

		// copy to output
		std::copy(m_queuedRectangles.begin(), m_queuedRectangles.end(), rectangles->rectangles);

		m_gameInterface->InvokeOnRender([] (void* arg)
		{
			auto rectangles = (ResultingRectangles*)arg;

			g_fontRenderer.GetGameInterface()->DrawRectangles(rectangles->count, rectangles->rectangles);

			delete[] rectangles->rectangles;
			delete rectangles;
		}, rectangles);

		m_queuedRectangles.clear();
	}

	// draw glyph runs
	auto numRuns = m_queuedGlyphRuns.size();

	if (numRuns)
	{
		ResultingGlyphRun** glyphRuns = new ResultingGlyphRun*[numRuns + 1];
		memcpy(glyphRuns, &m_queuedGlyphRuns[0], sizeof(ResultingGlyphRun*) * numRuns);
		glyphRuns[numRuns] = nullptr;

		m_gameInterface->InvokeOnRender([] (void* arg)
		{
			auto glyphRuns = (ResultingGlyphRun**)arg;

			for (ResultingGlyphRun** p = glyphRuns; *p; p++)
			{
				auto glyphRun = *p;

				for (uint32_t i = 0; i < glyphRun->numSubRuns; i++)
				{
					auto subRun = &glyphRun->subRuns[i];

					g_fontRenderer.GetGameInterface()->SetTexture(subRun->texture);
					g_fontRenderer.GetGameInterface()->DrawIndexedVertices(subRun->numVertices, subRun->numIndices, subRun->vertices, subRun->indices);
					g_fontRenderer.GetGameInterface()->UnsetTexture();
				}

				delete[] glyphRun;
			}

			delete glyphRuns;
		}, glyphRuns);

		m_queuedGlyphRuns.clear();
	}

	FrpSeqAllocatorSwapPage();

	m_gameInterface->InvokeOnRender([] (void* arg)
	{
		FrpSeqAllocatorUnlockSwap();
	}, nullptr);

	// clean the layout cache if needed
	// TODO: keep layouts for longer if they actually get recently used
	if ((GetTickCount() - m_lastLayoutClean) > 5000)
	{
		m_textLayoutCache.clear();

		m_lastLayoutClean = GetTickCount();
	}

	m_mutex.unlock();
}

FontRendererImpl g_fontRenderer;
FontRenderer* TheFonts = &g_fontRenderer;