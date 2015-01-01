/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "FontRendererImpl.h"

#include "memdbgon.h"

class CitizenTextRenderer : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IDWriteTextRenderer>
{
	// IDWritePixelSnapping methods
	virtual HRESULT STDMETHODCALLTYPE IsPixelSnappingDisabled(void * clientDrawingContext,
															  _Out_ BOOL * isDisabled) override;

	virtual HRESULT STDMETHODCALLTYPE GetPixelsPerDip(void * clientDrawingContext,
													  _Out_ FLOAT * pixelsPerDip) override;

	virtual HRESULT STDMETHODCALLTYPE GetCurrentTransform(void * clientDrawingContext,
														  _Out_ DWRITE_MATRIX * transform) override;

	// IDWriteTextRenderer methods
	virtual HRESULT STDMETHODCALLTYPE DrawGlyphRun(void * clientDrawingContext,
												   FLOAT baselineOriginX,
												   FLOAT baselineOriginY,
												   DWRITE_MEASURING_MODE measuringMode,
												   _In_ const DWRITE_GLYPH_RUN * glyphRun,
												   _In_ const DWRITE_GLYPH_RUN_DESCRIPTION * glyphRunDescription,
												   IUnknown * clientDrawingEffect) override;

	virtual HRESULT STDMETHODCALLTYPE DrawUnderline(void * clientDrawingContext,
													FLOAT baselineOriginX,
													FLOAT baselineOriginY,
													_In_ const DWRITE_UNDERLINE * underline,
													IUnknown * clientDrawingEffect) override;

	virtual HRESULT STDMETHODCALLTYPE DrawStrikethrough(void * clientDrawingContext,
														FLOAT baselineOriginX,
														FLOAT baselineOriginY,
														_In_ const DWRITE_STRIKETHROUGH * strikethrough,
														IUnknown * clientDrawingEffect) override;

	virtual HRESULT STDMETHODCALLTYPE DrawInlineObject(void * clientDrawingContext,
													   FLOAT originX,
													   FLOAT originY,
													   IDWriteInlineObject * inlineObject,
													   BOOL isSideways,
													   BOOL isRightToLeft,
													   IUnknown * clientDrawingEffect) override;
};

HRESULT CitizenTextRenderer::DrawGlyphRun(void * clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY, DWRITE_MEASURING_MODE measuringMode,
										  _In_ const DWRITE_GLYPH_RUN * glyphRun, _In_ const DWRITE_GLYPH_RUN_DESCRIPTION * glyphRunDescription, IUnknown * clientDrawingEffect)
{
	auto drawingContext = static_cast<CitizenDrawingContext*>(clientDrawingContext);

	auto cachedFont = g_fontRenderer.GetFontInstance(glyphRun->fontFace, glyphRun->fontEmSize);

	// get drawing effect
	ComPtr<IUnknown> drawingEffectPtr = clientDrawingEffect;
	ComPtr<ICitizenDrawingEffect> drawingEffect;
	drawingEffectPtr.As(&drawingEffect);

	if (cachedFont->EnsureFaceCreated())
	{
		auto resultingRun = cachedFont->TargetGlyphRun(baselineOriginX, baselineOriginY, glyphRun, glyphRunDescription, drawingEffect->GetColor());

		if (resultingRun)
		{
			drawingContext->glyphRuns.push_back(resultingRun);
		}
	}

	return S_OK;
}

HRESULT CitizenTextRenderer::IsPixelSnappingDisabled(void * clientDrawingContext, _Out_ BOOL * isDisabled)
{
	*isDisabled = TRUE;

	return S_OK;
}

HRESULT CitizenTextRenderer::GetPixelsPerDip(void * clientDrawingContext, _Out_ FLOAT * pixelsPerDip)
{
	*pixelsPerDip = 1.0f;

	return S_OK;
}

HRESULT CitizenTextRenderer::GetCurrentTransform(void * clientDrawingContext, _Out_ DWRITE_MATRIX * transform)
{
	transform->dx = 0.0f;
	transform->dy = 0.0f;
	transform->m11 = 1.0f;
	transform->m12 = 0.0f;
	transform->m21 = 0.0f;
	transform->m22 = 1.0f;

	return S_OK;
}

HRESULT CitizenTextRenderer::DrawInlineObject(void * clientDrawingContext, FLOAT originX, FLOAT originY, IDWriteInlineObject * inlineObject, BOOL isSideways, BOOL isRightToLeft, IUnknown * clientDrawingEffect)
{
	return E_NOTIMPL;
}

HRESULT CitizenTextRenderer::DrawStrikethrough(void * clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY, _In_ const DWRITE_STRIKETHROUGH * strikethrough, IUnknown * clientDrawingEffect)
{
	return E_NOTIMPL;
}

HRESULT CitizenTextRenderer::DrawUnderline(void * clientDrawingContext, FLOAT baselineOriginX, FLOAT baselineOriginY, _In_ const DWRITE_UNDERLINE * underline, IUnknown * clientDrawingEffect)
{
	return E_NOTIMPL;
}

void FontRendererImpl::CreateTextRenderer()
{
	m_textRenderer = Make<CitizenTextRenderer>();
}