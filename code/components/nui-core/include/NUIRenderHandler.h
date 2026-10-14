/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
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

	virtual void OnFrameCaptured(CefRefPtr<CefBrowser> browser, PaintElementType type) override;
	
#if 0
    virtual bool GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info) override;

	virtual bool GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX, int& screenY) override;
#endif
private:
	IMPLEMENT_REFCOUNTING(NUIRenderHandler);

private:
	CefBrowserHost::DragOperationsMask OnDragEnter(
		CefRefPtr<CefDragData> drag_data,
		CefMouseEvent ev,
		CefBrowserHost::DragOperationsMask effect) override;
	CefBrowserHost::DragOperationsMask OnDragOver(
		CefMouseEvent ev,
		CefBrowserHost::DragOperationsMask effect) override;
	void OnDragLeave() override;
	CefBrowserHost::DragOperationsMask OnDrop(
		CefMouseEvent ev,
		CefBrowserHost::DragOperationsMask effect) override;

public:
	CComPtr<DropTargetWin> GetDropTarget();

private:
	CComPtr<DropTargetWin> m_dropTarget;

	DragOperation m_currentDragOp;
};
