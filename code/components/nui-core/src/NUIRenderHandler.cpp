/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NUIRenderHandler.h"
#include "GfxUtil.h"

#include "CefImeHandler.h"
#include "CefOleHandler.h"

#include "memdbgon.h"

#include <CrossBuildRuntime.h>

#include <ShellScalingApi.h>
#pragma comment(lib, "Shcore.lib")

extern OsrImeHandlerWin* g_imeHandler;
extern OsrDragHandlerWin g_dragHandler;

NUIRenderHandler::NUIRenderHandler(NUIClient* client)
	: m_owner(client), m_currentDragOp(DRAG_OPERATION_NONE)
{
}

NUIRenderHandler::~NUIRenderHandler()
{
	if (m_dropTarget)
	{
		m_dropTarget->CancelCallback();
	}
}

CComPtr<DropTargetWin> NUIRenderHandler::GetDropTarget()
{
	if (!m_dropTarget)
	{
		if (auto hWnd = CoreGetGameWindow())
		{
			m_dropTarget = DropTargetWin::Create(this, hWnd);
		}
	}
	return m_dropTarget;
}

void NUIRenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
	// TODO: Fully implement scaling support.
#if 0
    UINT dpiX, dpiY;
	HMONITOR monitor = MonitorFromPoint({ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);
	GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);

	// https://learn.microsoft.com/en-us/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows
	const float kDPIScale = 96.0f;
	float scale = dpiX / kDPIScale;
#else
	float scale = 1.0f;
#endif

	NUIWindow* window = m_owner->GetWindowValid() ? m_owner->GetWindow() : nullptr;
	int width = window ? window->GetWidth() : 1280;
	int height = window ? window->GetHeight() : 720;

	rect.Set(0, 0, width / scale, height / scale);
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

void NUIRenderHandler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, const RectList& dirtyRects, const void* buffer, int width, int height)
{
	// OnPaint hasn't worked as intended since M103 
	// With inconsistent texture updates and incorrect dirty rect leading to partily stale frames.
	trace("NUIRenderHandler::OnPaint was called, this may be a sign that the CEF process has fatally errored and isn't recoverable\n");
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


bool NUIRenderHandler::StartDragging(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDragData> drag_data, CefRenderHandler::DragOperationsMask allowed_ops, int x, int y)
{
	auto dropTarget = GetDropTarget();
	if (!dropTarget)
	{
		return false;
	}

	auto wndThread = GetWindowThreadProcessId(CoreGetGameWindow(), nullptr);
	auto dragThread = GetCurrentThreadId();

	m_currentDragOp = DRAG_OPERATION_NONE;
	g_dragHandler.EnableBackgroundDragging(wndThread, dragThread);
	CefBrowserHost::DragOperationsMask result =
		dropTarget->StartDragging(browser, drag_data, allowed_ops, x, y);
	g_dragHandler.DisableBackgroundDragging();
	m_currentDragOp = DRAG_OPERATION_NONE;
	POINT pt = {};
	GetCursorPos(&pt);
	ScreenToClient(CoreGetGameWindow(), &pt);

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

void NUIRenderHandler::OnFrameCaptured(CefRefPtr<CefBrowser> browser, PaintElementType type)
{
	if (m_owner->GetWindowValid())
	{
		m_owner->GetWindow()->UpdateSharedResource(type);
	}
}

// TODO: NUI should take into consider window scaling support. This code below does it but is incomplete
// input such as mouse movements needs to also take this into account, which it currently doesnt.
#if 0
bool NUIRenderHandler::GetScreenInfo(CefRefPtr<CefBrowser> browser, CefScreenInfo& screen_info)
{
	UINT dpiX, dpiY;
	HMONITOR monitor = MonitorFromPoint({ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);
	GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
    
	// https://learn.microsoft.com/en-us/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows
	const float kDPIScale = 96.0f;
	float scale = dpiX / kDPIScale;

    int physicalWidth = GetSystemMetrics(SM_CXSCREEN);
	int physicalHeight = GetSystemMetrics(SM_CYSCREEN);

    screen_info.device_scale_factor = scale;
	//screen_info.depth = 24;
	//screen_info.depth_per_component = 8;
	//screen_info.is_monochrome = false;
	screen_info.rect = { 0, 0, (int)(physicalWidth / scale), (int)(physicalHeight / scale) };
	screen_info.available_rect = screen_info.rect;
	return true;
}

bool NUIRenderHandler::GetScreenPoint(CefRefPtr<CefBrowser> browser, int viewX, int viewY, int& screenX, int& screenY)
{
	screenX = viewX;
	screenY = viewY;
	return true;
}
#endif

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
