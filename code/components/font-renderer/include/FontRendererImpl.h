/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <dwrite.h>
#include <dwrite_2.h>

#include <wrl.h>
#include "FontRenderer.h"
#include "FontRendererAbstract.h"

#include <mutex>

using namespace Microsoft::WRL;

class CachedFont;

class CachedFontPage : public fwRefCountable
{
private:
	struct FontCharacter
	{
		CRect address;

		CRect affect;

		uint8_t* pixelData;
	};

private:
	CachedFont* m_owningFont;

	uint32_t m_minCodePoint;
	uint32_t m_maxCodePoint;

	bool m_enqueued;

	std::vector<FontCharacter> m_characterAddresses;

	void* m_pixelData;

	FontRendererTexture* m_targetTexture;

private:
	void CreateFontPage();

	void EnqueueCreateFontPage();

	void CreateNow();

	void CreateCharacter(uint32_t codePoint);

public:
	CachedFontPage(CachedFont* parent, uint32_t minCodePoint, uint32_t maxCodePoint);

	bool EnsureFontPageCreated();

	inline FontRendererTexture* GetTexture() { return m_targetTexture; }

	const CRect& GetCharacterAddress(uint32_t codePoint);

	const CRect& GetCharacterSize(uint32_t codePoint);
};

typedef FontRendererVertex ResultingVertex;
typedef uint16_t ResultingIndex;

struct ResultingSubGlyphRun : public FrpUseSequentialAllocator
{
	FontRendererTexture* texture;

	uint32_t numVertices;
	uint32_t numIndices;

	ResultingVertex* vertices;
	ResultingIndex* indices;

	int order;

	ResultingSubGlyphRun();

	~ResultingSubGlyphRun();
};

struct ResultingGlyphRun : public FrpUseSequentialAllocator
{
	uint32_t numSubRuns;

	ResultingSubGlyphRun* subRuns;

	~ResultingGlyphRun();
};

class CachedFont : public fwRefCountable
{
friend class CachedFontPage;

private:
	std::map<uint32_t, fwRefContainer<CachedFontPage>> m_pages;

	std::vector<bool> m_presentPages;

	ComPtr<IDWriteFontFace> m_dwFace;

	float m_emSize;

private:
	void CreateFace();

	void TargetGlyphRunInternal(float originX, float originY, const DWRITE_GLYPH_RUN* glyphRun, ResultingSubGlyphRun* initialRuns, ResultingVertex* initialVertices, ResultingIndex* initialIndices, CRGBA color, int& runCount);

public:
	CachedFont(ComPtr<IDWriteFontFace> fontFace, float emSize);

	bool EnsureFaceCreated();

	ResultingGlyphRun* TargetGlyphRun(float originX, float originY, const DWRITE_GLYPH_RUN* glyphRun, const DWRITE_GLYPH_RUN_DESCRIPTION* glyphRunDescription, CRGBA color);
};

struct CitizenDrawingContext
{
	std::vector<ResultingGlyphRun*> glyphRuns;

	float fontScale;

	~CitizenDrawingContext();
};

namespace std
{
	template<typename T1, typename T2>
	struct hash<std::pair<T1, T2>>
	{
		std::size_t operator()(const std::pair<T1, T2>& x) const
		{
			return (3 * std::hash<T1>()(x.first)) ^ std::hash<T2>()(x.second);
		}
	};
}

class FontRendererImpl : public FontRenderer
{
private:
	ComPtr<IDWriteFactory> m_dwFactory;

	ComPtr<IDWriteFactory2> m_dwFactory2;

	ComPtr<IDWriteTextRenderer> m_textRenderer;

	FontRendererGameInterface* m_gameInterface;

	std::vector<ResultingGlyphRun*> m_queuedGlyphRuns;

	std::vector<ResultingRectangle> m_queuedRectangles;

	std::unordered_map<std::string, fwRefContainer<CachedFont>> m_fontCache;

	std::unordered_map<std::pair<fwString, float>, ComPtr<IDWriteTextFormat>> m_textFormatCache;

	std::unordered_map<std::pair<IDWriteTextFormat*, std::pair<uint32_t, fwWString>>, ComPtr<IDWriteTextLayout>> m_textLayoutCache;

	uint32_t m_lastLayoutClean;

	std::recursive_mutex m_mutex;

private:
	void CreateTextRenderer();

	std::string GetFontKey(ComPtr<IDWriteFontFace> fontFace, float emSize);

public:
	virtual void Initialize();

	void PerFrameInit();

	virtual void DrawPerFrame();

	fwRefContainer<CachedFont> GetFontInstance(ComPtr<IDWriteFontFace> fontFace, float emSize);

	inline FontRendererGameInterface* GetGameInterface() { return m_gameInterface; }

	inline ComPtr<IDWriteFactory> GetDWriteFactory() { return m_dwFactory; }

	inline ComPtr<IDWriteFactory2> GetDWriteFactory2() { return m_dwFactory2; }

	virtual void DrawText(fwWString text, const CRect& rect, const CRGBA& color, float fontSize, float fontScale, fwString fontRef);

	virtual void DrawRectangle(const CRect& rect, const CRGBA& color);

	virtual bool GetStringMetrics(fwWString characterString, float fontSize, float fontScale, fwString fontRef, CRect& outRect);
};

// not entirely COM calling convention, but we'll only use it internally
// {71246052-4EEA-4339-BBC0-D2246A3F5CE3}
DEFINE_GUID(IID_ICitizenDrawingEffect,
			0x71246052, 0x4eea, 0x4339, 0xbb, 0xc0, 0xd2, 0x24, 0x6a, 0x3f, 0x5c, 0xe3);

interface DECLSPEC_UUID("71246052-4EEA-4339-BBC0-D2246A3F5CE3") ICitizenDrawingEffect;

struct ICitizenDrawingEffect : public IUnknown
{
	virtual CRGBA GetColor() = 0;

	virtual void SetColor(CRGBA color) = 0;
};

class CitizenDrawingEffect : public RuntimeClass<RuntimeClassFlags<ClassicCom>, ICitizenDrawingEffect>
{
private:
	CRGBA m_color;

public:
	virtual CRGBA GetColor() { return m_color; }

	virtual void SetColor(CRGBA color) { m_color = color; }
};

FontRendererGameInterface* CreateGameInterface();

extern FontRendererImpl g_fontRenderer;