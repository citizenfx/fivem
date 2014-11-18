#include "StdInc.h"
#include <FontRendererImpl.h>
#include "Hooking.h"

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

	// store in a std::string
	std::string referenceKeyStr((const char*)referenceKey, referenceKeySize);

	// release the file
	file->Release();

	// get metrics
	referenceKeyStr += va("_%f", emSize);

	return referenceKeyStr;
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

void FontRendererImpl::DrawText(fwWString text, const CRect& rect, const CRGBA& color, float fontSize, float fontScale, fwString fontRef)
{
	ComPtr<IDWriteTextFormat> textFormat;

	m_dwFactory->CreateTextFormat(L"Segoe UI", nullptr, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-us", textFormat.GetAddressOf());

	ComPtr<IDWriteTextLayout> textLayout;

	m_dwFactory->CreateTextLayout(text.c_str(), text.length(), textFormat.Get(), rect.Width(), rect.Height(), textLayout.GetAddressOf());

	auto drawingContext = new CitizenDrawingContext();
	textLayout->Draw(drawingContext, m_textRenderer.Get(), rect.Left(), rect.Top());

	auto numRuns = drawingContext->glyphRuns.size();

	if (numRuns)
	{
		auto glyphRuns = new ResultingGlyphRun*[numRuns + 1];
		memcpy(glyphRuns, &drawingContext->glyphRuns[0], sizeof(ResultingGlyphRun*) * numRuns);
		glyphRuns[numRuns] = nullptr;

		m_gameInterface->InvokeOnRender([] (void* arg)
		{
			auto glyphRuns = (ResultingGlyphRun**)arg;

			for (ResultingGlyphRun** p = glyphRuns; *p; p++)
			{
				auto glyphRun = *p;

				for (int i = 0; i < glyphRun->numSubRuns; i++)
				{
					auto subRun = &glyphRun->subRuns[i];

					g_fontRenderer.GetGameInterface()->SetTexture(subRun->texture);
					g_fontRenderer.GetGameInterface()->DrawIndexedVertices(subRun->numVertices, subRun->numIndices, subRun->vertices, subRun->indices);
					g_fontRenderer.GetGameInterface()->UnsetTexture();
				}

				delete glyphRun;
			}

			delete glyphRuns;
		}, glyphRuns);
	}

	delete drawingContext;
}

FontRendererImpl g_fontRenderer;
FontRenderer* TheFonts = &g_fontRenderer;