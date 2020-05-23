/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "NUIClient.h"
#include <include/cef_render_handler.h>
#include "CefDragDropHandler.h"

class NUIRenderHandler : public CefRenderHandler, public OsrDragEvents
{
public:
	NUIRenderHandler(NUIClient* client);

	virtual ~NUIRenderHandler();

private:
	NUIClient* m_owner;

	// used to prevent infinite recursion when redrawing the popup rectangle
	bool m_paintingPopup;

	CefRect m_popupRect;

protected:
	virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect) override;

	virtual void OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height) override;

	virtual void OnPopupShow(CefRefPtr<CefBrowser> browser, bool show) override;

	virtual void OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect) override;

	virtual void OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> browser, const CefRange& selected_range, const RectList& character_bounds) override;

	bool StartDragging(CefRefPtr<CefBrowser> browser,
		CefRefPtr<CefDragData> drag_data,
		CefRenderHandler::DragOperationsMask allowed_ops,
		int x,
		int y) override;
	void UpdateDragCursor(CefRefPtr<CefBrowser> browser,
		CefRenderHandler::DragOperation operation) override;

	virtual void OnAcceleratedPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, void* shared_handle) override;

private:
	void PaintView(const RectList& dirtyRects, const void* buffer, int width, int height);

	void PaintPopup(const void* buffer, int width, int height);

	void UpdatePopup();

	IMPLEMENT_REFCOUNTING(NUIRenderHandler);

private:
	CefBrowserHost::DragOperationsMask OnDragEnter(
		CefRefPtr<CefDragData> drag_data,
		CefMouseEvent ev,
		CefBrowserHost::DragOperationsMask effect) OVERRIDE;
	CefBrowserHost::DragOperationsMask OnDragOver(
		CefMouseEvent ev,
		CefBrowserHost::DragOperationsMask effect) OVERRIDE;
	void OnDragLeave() OVERRIDE;
	CefBrowserHost::DragOperationsMask OnDrop(
		CefMouseEvent ev,
		CefBrowserHost::DragOperationsMask effect) OVERRIDE;

public:
	inline CComPtr<DropTargetWin> GetDropTarget()
	{
		return m_dropTarget;
	}

private:
	CComPtr<DropTargetWin> m_dropTarget;

	DragOperation m_currentDragOp;
};
