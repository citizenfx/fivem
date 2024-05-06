#include <StdInc.h>

#include "CefOleHandler.h"

OsrDragHandlerWin::OsrDragHandlerWin()
	: m_sourceThread(0), m_dragThread(0), m_mouseUpReceived(false)
{
}

void OsrDragHandlerWin::EnableBackgroundDragging(DWORD wndThread, DWORD backgroundThread)
{
	if (wndThread == 0 || backgroundThread == 0)
	{
		return;
	}

	// Attach input state so that SetCursor can work from the drag thread
	m_sourceThread = wndThread;
	m_dragThread = backgroundThread;
	m_mouseUpReceived = false;
	AttachThreadInput(m_dragThread, m_sourceThread, TRUE);
}

void OsrDragHandlerWin::DisableBackgroundDragging()
{
	if (m_sourceThread == 0 || m_dragThread == 0)
	{
		return;
	}

	AttachThreadInput(m_dragThread, m_sourceThread, FALSE);
	m_sourceThread = 0;
	m_dragThread = 0;
	m_mouseUpReceived = false;
}

/// Based on: content/browser/web_contents/web_contents_drag_win.cc (MsgFilterProc)
bool OsrDragHandlerWin::OnWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (!IsDragging())
	{
		return false;
	}

	// "We do not care about WM_SYSKEYDOWN and WM_SYSKEYUP because when ALT key
	// is pressed down on drag-and-drop, it means to create a link."
	if (msg == WM_KEYDOWN || msg == WM_KEYUP || msg == WM_MOUSEMOVE || msg == WM_LBUTTONUP)
	{
		PostThreadMessage(m_dragThread, msg, wParam, lParam);
		if (msg == WM_LBUTTONUP || !(GetKeyState(VK_LBUTTON) & 0x8000))
		{
			m_mouseUpReceived = true;
		}
		return true;
	}

	return false;
}
