/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIRenderHandler.h"
#include "GfxUtil.h"

#include "CefImeHandler.h"

#include "memdbgon.h"

extern OsrImeHandlerWin* g_imeHandler;

NUIRenderHandler::NUIRenderHandler(NUIClient* client)
	: m_paintingPopup(false), m_owner(client), m_currentDragOp(DRAG_OPERATION_NONE)
{
	auto hWnd = FindWindow(
#ifdef GTA_FIVE
		L"grcWindow"
#elif defined(IS_RDR3)
		L"sgaWindow"
#else
		L"UNKNOWN_WINDOW"
#endif
	, nullptr);

	m_dropTarget = DropTargetWin::Create(this, hWnd);

	HRESULT hr = RegisterDragDrop(hWnd, m_dropTarget);
	if (FAILED(hr))
	{
		trace("registering drag/drop failed. hr: %08x\n", hr);
	}
}

void NUIRenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
	if (m_owner->GetWindowValid())
	{
		NUIWindow* window = m_owner->GetWindow();
		rect.Set(0, 0, window->GetWidth(), window->GetHeight());
	}
}

void NUIRenderHandler::OnImeCompositionRangeChanged(CefRefPtr<CefBrowser> browser, const CefRange& selected_range, const RectList& character_bounds)
{
	if (g_imeHandler) {
		// Convert from view coordinates to device coordinates.
		CefRenderHandler::RectList device_bounds;
		CefRenderHandler::RectList::const_iterator it = character_bounds.begin();
		for (; it != character_bounds.end(); ++it) {
			device_bounds.push_back(*it);
		}

		g_imeHandler->ChangeCompositionRange(selected_range, device_bounds);
	}
}

void NUIRenderHandler::OnAcceleratedPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, void* shared_handle)
{
	if (m_owner->GetWindowValid())
	{
		m_owner->GetWindow()->UpdateSharedResource(shared_handle, -1, dirtyRects, type);
	}
}

void NUIRenderHandler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height)
{
	if (m_owner->GetWindowValid() && m_owner->GetWindow()->GetRenderBuffer())
	{
		// lock the window lock
		auto& lock = m_owner->GetWindowLock();
		lock.lock();

		// paint the buffer
		if (type == PET_VIEW)
		{
			PaintView(dirtyRects, buffer, width, height);
		}
		else if (type == PET_POPUP)
		{
			PaintPopup(buffer, width, height);
		}

		// invalidate and paint the popup
		UpdatePopup();

		// mark the render buffer as dirty
		m_owner->GetWindow()->MarkRenderBufferDirty();

		// unlock the lock
		lock.unlock();
	}
}


void NUIRenderHandler::OnPopupShow(CefRefPtr<CefBrowser> browser, bool show)
{
	if (!m_owner->GetWindowValid())
	{
		return;
	}

	// if we're hiding the popup, redraw the view that the popup was drawn on
	if (!show)
	{
		m_popupRect.Set(0, 0, 0, 0);
		browser->GetHost()->Invalidate(PET_VIEW);
	}

	// accelerated handling
	m_owner->GetWindow()->HandlePopupShow(show);
}

void NUIRenderHandler::OnPopupSize(CefRefPtr<CefBrowser> browser, const CefRect& rect)
{
	if (rect.width <= 0 || rect.height <= 0)
	{
		return;
	}

	NUIWindow* window = m_owner->GetWindow();

	m_popupRect = rect;

	if (m_popupRect.x < 0)
	{
		m_popupRect.x = 0;
	}

	if (m_popupRect.y < 0)
	{
		m_popupRect.y = 0;
	}

	if ((m_popupRect.x + m_popupRect.width) > window->GetWidth())
	{
		m_popupRect.x = window->GetWidth() - m_popupRect.width;
	}

	if ((m_popupRect.y + m_popupRect.height) > window->GetHeight())
	{
		m_popupRect.y = window->GetHeight() - m_popupRect.height;
	}

	if (m_popupRect.x < 0)
	{
		m_popupRect.x = 0;
	}

	if (m_popupRect.y < 0)
	{
		m_popupRect.y = 0;
	}

	// accelerated handling
	m_owner->GetWindow()->SetPopupRect(m_popupRect);
}

void NUIRenderHandler::UpdatePopup()
{
	if (!m_paintingPopup)
	{
		if (!m_popupRect.IsEmpty())
		{
			m_paintingPopup = true;

			m_owner->GetBrowser()->GetHost()->Invalidate(PET_POPUP);

			m_paintingPopup = false;
		}
	}
}

void NUIRenderHandler::PaintView(const RectList& dirtyRects, const void* buffer, int width, int height)
{
	NUIWindow* window = m_owner->GetWindow();
	void* renderBuffer = window->GetRenderBuffer();
	int roundedWidth = window->GetRoundedWidth();

	for (auto& rect : dirtyRects)
	{
		{
			for (int y = rect.y; y < (rect.y + rect.height); y++)
			{
				int* src = &((int*)(buffer))[(y * width) + rect.x];
				int* dest = &((int*)(renderBuffer))[(y * roundedWidth) + rect.x];

				memcpy(dest, src, (rect.width * 4));
			}
		}

		window->AddDirtyRect(rect);
	}
}

void NUIRenderHandler::PaintPopup(const void* buffer, int width, int height)
{
	NUIWindow* window = m_owner->GetWindow();

	// clean popup rectangle
	int skip_pixels = 0, x = m_popupRect.x;
	int skip_rows = 0, yy = m_popupRect.y;

	int w = width;
	int h = height;

	if (x < 0)
	{
		skip_pixels = -x;
		x = 0;
	}

	if (yy < 0)
	{
		skip_rows = -yy;
		yy = 0;
	}

	if ((x + w) > window->GetWidth())
	{
		w -= ((x + w) - window->GetWidth());
	}

	if ((yy + h) > window->GetHeight())
	{
		h -= ((yy + h) - window->GetHeight());
	}

	// do rendering
	void* renderBuffer = window->GetRenderBuffer();
	int roundedWidth = window->GetRoundedWidth();

	{
		for (int y = yy; y < (yy + h); y++)
		{
			int* src = &((int*)(buffer))[((y - yy) * width)];
			int* dest = &((int*)(renderBuffer))[(y * roundedWidth) + x];

			memcpy(dest, src, (w * 4));
		}
	}

	// add dirty rect
	window->AddDirtyRect(m_popupRect);
}

bool g_isDragging;

bool NUIRenderHandler::StartDragging(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> drag_data, CefRenderHandler::DragOperationsMask allowed_ops, int x, int y)
{
	if (!m_dropTarget)
	{
		return false;
	}

	m_currentDragOp = DRAG_OPERATION_NONE;
	g_isDragging = true;
	CefBrowserHost::DragOperationsMask result =
		m_dropTarget->StartDragging(browser, drag_data, allowed_ops, x, y);
	g_isDragging = false;
	m_currentDragOp = DRAG_OPERATION_NONE;
	POINT pt = {};
	GetCursorPos(&pt);
	ScreenToClient(FindWindow(
#ifdef GTA_FIVE
		L"grcWindow"
#elif defined(IS_RDR3)
		L"sgaWindow"
#else
		L"UNKNOWN_WINDOW"
#endif	
	, nullptr), &pt);

	browser->GetHost()->DragSourceEndedAt(
		pt.x,
		pt.y, result);
	browser->GetHost()->DragSourceSystemDragEnded();
	return true;
}

void NUIRenderHandler::UpdateDragCursor(CefRefPtr<CefBrowser> browser,
	CefRenderHandler::DragOperation operation) {
	m_currentDragOp = operation;
}

CefBrowserHost::DragOperationsMask NUIRenderHandler::OnDragEnter(
	CefRefPtr<CefDragData> drag_data,
	CefMouseEvent ev,
	CefBrowserHost::DragOperationsMask effect) {
	if (m_owner->GetBrowser()) {
		m_owner->GetBrowser()->GetHost()->DragTargetDragEnter(drag_data, ev, effect);
		m_owner->GetBrowser()->GetHost()->DragTargetDragOver(ev, effect);
	}
	return m_currentDragOp;
}

CefBrowserHost::DragOperationsMask NUIRenderHandler::OnDragOver(
	CefMouseEvent ev,
	CefBrowserHost::DragOperationsMask effect) {
	if (m_owner->GetBrowser()) {
		m_owner->GetBrowser()->GetHost()->DragTargetDragOver(ev, effect);
	}
	return m_currentDragOp;
}

void NUIRenderHandler::OnDragLeave() {
	if (m_owner->GetBrowser())
		m_owner->GetBrowser()->GetHost()->DragTargetDragLeave();
}

CefBrowserHost::DragOperationsMask NUIRenderHandler::OnDrop(
	CefMouseEvent ev,
	CefBrowserHost::DragOperationsMask effect) {
	if (m_owner->GetBrowser()) {
		m_owner->GetBrowser()->GetHost()->DragTargetDragOver(ev, effect);
		m_owner->GetBrowser()->GetHost()->DragTargetDrop(ev);
	}
	return m_currentDragOp;
}
