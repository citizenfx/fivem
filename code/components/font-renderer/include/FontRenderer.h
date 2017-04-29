/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#undef DrawText

#include <Rect.h>
#include <RGBA.h>

class FontRenderer
{
public:
	virtual void Initialize() = 0;

	virtual void DrawText(fwWString text, const CRect& rect, const CRGBA& color, float fontSize, float fontScale, fwString fontRef) = 0;

	virtual void DrawRectangle(const CRect& rect, const CRGBA& color) = 0;

	virtual void DrawPerFrame() = 0;

	virtual bool GetStringMetrics(fwWString characterString, float fontSize, float fontScale, fwString fontRef, CRect& outRect) = 0;
};

extern
#ifdef BUILDING_FONT_RENDERER
	__declspec(dllexport)
#else
	__declspec(dllimport)
#endif
	FontRenderer* TheFonts;