#pragma once

#include <StdInc.h>

// Windows-specific drag-and-drop handling in WebContentsView. Based on
// Chromium's previous content/browser/web_contents/web_contents_drag_win.cc
// implementation.
class OsrDragHandlerWin
{
private:
	DWORD m_sourceThread;
	DWORD m_dragThread;
	bool m_mouseUpReceived;

public:
	explicit OsrDragHandlerWin();

	/// Initialize drag-drop message forwarding
	void EnableBackgroundDragging(DWORD wndThread, DWORD backgrndThread);

	/// Disable drag-drag message forwarding
	void DisableBackgroundDragging();

	/// Forward relevant messages from the UI thread (Render) to the drag-drop thread (crBrowser).
	bool OnWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	/// Return true if a drag operation is active.
	inline bool IsDragging()
	{
		return m_dragThread != 0 && !m_mouseUpReceived;
	}
};
