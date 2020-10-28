#include <StdInc.h>
#include <Hooking.h>

#include <concurrent_queue.h>

static decltype(&CreateWindowExW) g_origCreateWindowExW;

static concurrency::concurrent_queue<std::tuple<HWND, UINT, WPARAM, LPARAM>> g_wndMsgQueue;
static WNDPROC g_origWndProc;

static LRESULT CALLBACK NewWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// list of unsafe messages
	bool unsafe = false;
	LRESULT unsafeResult = 0;

	if (uMsg == WM_SYSKEYDOWN && wParam == 13)
	{
		unsafe = true;
		unsafeResult = 0;
	}
	
	if (uMsg == WM_ACTIVATEAPP)
	{
		unsafe = true;
		unsafeResult = 0;
	}

	if (uMsg == WM_DEVICECHANGE && wParam == 7)
	{
		unsafe = true;
		unsafeResult = 0;
	}

	if (uMsg == WM_SIZE)
	{
		unsafe = true;
		unsafeResult = DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}

	if (uMsg == WM_PAINT)
	{
		unsafe = true;
		unsafeResult = 0;
	}

	if (uMsg > 0xC000)
	{
		unsafe = true;
		unsafeResult = 0;
	}

	if (unsafe)
	{
		g_wndMsgQueue.push({ hWnd, uMsg, wParam, lParam });

		return unsafeResult;
	}

	return g_origWndProc(hWnd, uMsg, wParam, lParam);
}

static HANDLE g_windowThreadWakeEvent;
static concurrency::concurrent_queue<std::function<void()>> g_wndFuncQueue;

void WakeWindowThreadFor(std::function<void()>&& func)
{
	g_wndFuncQueue.push(std::move(func));
	SetEvent(g_windowThreadWakeEvent);
}

static CURSORINFO g_ci;

static HWND WINAPI CreateWindowExWStub(
DWORD dwExStyle,
LPCWSTR lpClassName,
LPCWSTR lpWindowName,
DWORD dwStyle,
int X,
int Y,
int nWidth,
int nHeight,
HWND hWndParent,
HMENU hMenu,
HINSTANCE hInstance,
LPVOID lpParam)
{
	if (!wcscmp(lpClassName, L"grcWindow"))
	{
		HANDLE hDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		auto parentThreadId = GetCurrentThreadId();
		HWND hWnd;

		g_windowThreadWakeEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

		static HANDLE handles[1] = { g_windowThreadWakeEvent };

		std::thread([=, &hWnd]()
		{
			SetThreadName(-1, "grcWindowThread");

			hWnd = g_origCreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
			AttachThreadInput(parentThreadId, GetCurrentThreadId(), TRUE);

			SetEvent(hDoneEvent);

			while (TRUE)
			{
				auto waitOn = MsgWaitForMultipleObjects(std::size(handles), handles, FALSE, 50, QS_ALLINPUT);
				ResetEvent(g_windowThreadWakeEvent);

				std::function<void()> fn;
				while (g_wndFuncQueue.try_pop(fn))
				{
					fn();
				}

				g_ci.cbSize = sizeof(g_ci);
				GetCursorInfo(&g_ci);

				MSG msg = { 0 };
				if (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessageW(&msg);
				}
			}
		}).detach();

		WaitForSingleObject(hDoneEvent, INFINITE);
		CloseHandle(hDoneEvent);

		return hWnd;
	}

	return g_origCreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

static BOOL PeekMessageWFake(_Out_ LPMSG lpMsg, _In_opt_ HWND hWnd, _In_ UINT wMsgFilterMin, _In_ UINT wMsgFilterMax, _In_ UINT wRemoveMsg)
{
	decltype(g_wndMsgQueue)::value_type m;

	if (g_wndMsgQueue.try_pop(m))
	{
		lpMsg->hwnd = std::get<0>(m);
		lpMsg->message = std::get<1>(m);
		lpMsg->wParam = std::get<2>(m);
		lpMsg->lParam = std::get<3>(m);

		return TRUE;
	}

	return FALSE;
}

static void TranslateMessageFake()
{
}

static void DispatchMessageWFake(_In_ CONST MSG *lpMsg)
{
	g_origWndProc(lpMsg->hwnd, lpMsg->message, lpMsg->wParam, lpMsg->lParam);
}

static BOOL GetCursorInfoFake(PCURSORINFO i)
{
	*i = g_ci;
	return TRUE;
}

static int ShowCursorWrap(BOOL ye)
{
	if (!ye)
	{
		if (g_ci.flags & CURSOR_SHOWING)
		{
			g_ci.flags &= ~CURSOR_SHOWING;

			WakeWindowThreadFor([]()
			{
				while (ShowCursor(FALSE) >= 0)
					;
			});
		}

		return -1;
	}
	else
	{
		if (!(g_ci.flags & CURSOR_SHOWING))
		{
			g_ci.flags |= CURSOR_SHOWING;

			WakeWindowThreadFor([]()
			{
				while (ShowCursor(TRUE) < 0)
					;
			});
		}

		return 0;
	}
}

static HookFunction hookFunction([]()
{
	g_origCreateWindowExW = hook::iat("user32.dll", CreateWindowExWStub, "CreateWindowExW");

	char* location = hook::pattern("48 8D 05 ? ? ? ? 33 C9 44 89 75 20 4C 89 7D").count(1).get(0).get<char>(3);
	g_origWndProc = (WNDPROC)(location + *(int32_t*)location + 4);
	*(int32_t*)location = (intptr_t)(hook::AllocateFunctionStub(NewWndProc)) - (intptr_t)location - 4;

	hook::iat("user32.dll", PeekMessageWFake, "PeekMessageW");
	hook::iat("user32.dll", DispatchMessageWFake, "DispatchMessageW");
	hook::iat("user32.dll", TranslateMessageFake, "TranslateMessageW");

	hook::iat("user32.dll", GetCursorInfoFake, "GetCursorInfo");
	hook::iat("user32.dll", GetAsyncKeyState, "GetKeyState");
	hook::iat("user32.dll", ShowCursorWrap, "ShowCursor");

	// watch dog function for weird counters
	{
		auto location = hook::get_pattern<char>("72 08 83 C9 FF E8", 2);
		hook::nop(location, 5 + 3);
		hook::put<uint8_t>(location, 0xBF); // mov edi, imm32 ...
		hook::put<uint32_t>(location + 1, 0); // mov edi, 0
	}
});
