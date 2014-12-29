/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "NUIClient.h"
#include <include/cef_render_handler.h>

class NUIRenderHandler : public CefRenderHandler
{
public:
	NUIRenderHandler(NUIClient* client);

private:
	NUIClient* m_owner;

	// used to prevent infinite recursion when redrawing the popup rectangle
	bool m_paintingPopup;

	CefRect m_popupRect;

protected:
	virtual bool GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;

	virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height) override;

	virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override;

	virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override;

private:
	void PaintView(const RectList& dirtyRects, const void* buffer, int width, int height);

	void PaintPopup(const void* buffer, int width, int height);

	void UpdatePopup();

	IMPLEMENT_REFCOUNTING(NUIRenderHandler);
};