#pragma once

#include <dwrite.h>
#include <dwrite_2.h>

#include <wrl.h>
#include "FontRenderer.h"
#include "FontRendererAbstract.h"

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

struct ResultingSubGlyphRun
{
	FontRendererTexture* texture;

	uint32_t numVertices;
	uint32_t numIndices;

	ResultingVertex* vertices;
	ResultingIndex* indices;

	ResultingSubGlyphRun();

	~ResultingSubGlyphRun();
};

struct ResultingGlyphRun
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

class FontRendererImpl : public FontRenderer
{
private:
	ComPtr<IDWriteFactory> m_dwFactory;

	ComPtr<IDWriteFactory2> m_dwFactory2;

	ComPtr<IDWriteTextRenderer> m_textRenderer;

	FontRendererGameInterface* m_gameInterface;

	std::unordered_map<std::string, fwRefContainer<CachedFont>> m_fontCache;

private:
	void CreateTextRenderer();

	std::string GetFontKey(ComPtr<IDWriteFontFace> fontFace, float emSize);

public:
	virtual void Initialize();

	void PerFrameInit();

	fwRefContainer<CachedFont> GetFontInstance(ComPtr<IDWriteFontFace> fontFace, float emSize);

	inline FontRendererGameInterface* GetGameInterface() { return m_gameInterface; }

	inline ComPtr<IDWriteFactory> GetDWriteFactory() { return m_dwFactory; }

	inline ComPtr<IDWriteFactory2> GetDWriteFactory2() { return m_dwFactory2; }

	virtual void DrawText(fwWString text, const CRect& rect, const CRGBA& color, float fontSize, float fontScale, fwString fontRef);
};

FontRendererGameInterface* CreateGameInterface();

extern FontRendererImpl g_fontRenderer;