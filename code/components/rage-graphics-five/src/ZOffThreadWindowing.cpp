#include <StdInc.h>
#include <Hooking.h>

#include <concurrent_queue.h>

static decltype(&CreateWindowExW) g_origCreateWindowExW;

static concurrency::concurrent_queue<std::tuple<HWND, UINT, WPARAM, LPARAM, std::function<void(LRESULT)>>> g_wndMsgQueue;
static WNDPROC g_origWndProc;
static DWORD g_renderThreadId;
static HANDLE g_renderThread;
static HANDLE g_renderThreadGate;

static uint64_t g_renderThreadLastPoll;

void HandleMessageLoop(ULONG_PTR);

static LRESULT CALLBACK NewWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// list of unsafe messages
	bool unsafe = false;
	bool forwardNow = false;
	volatile LRESULT unsafeResult = 0;

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

	if (uMsg == WM_PAINT)
	{
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
	}

	if (uMsg == WM_TIMER)
	{
		unsafe = true;
		unsafeResult = 0;
	}

	if (uMsg == WM_ENTERSIZEMOVE)
	{
		ResetEvent(g_renderThreadGate);
	}

	if (uMsg == WM_SIZING || uMsg == WM_ENTERSIZEMOVE || uMsg == WM_EXITSIZEMOVE || uMsg == WM_SIZE || uMsg == WM_MOVE || uMsg == WM_MOVING)
	{
		unsafe = true;
		forwardNow = true;
	}

	if (uMsg > 0xC000)
	{
		unsafe = true;
		unsafeResult = 0;
	}

	if (unsafe)
	{
		if (forwardNow && (GetTickCount64() - g_renderThreadLastPoll) < 100)
		{
			static thread_local bool done = false;

			g_wndMsgQueue.push({ hWnd, uMsg, wParam, lParam, [&unsafeResult](LRESULT r)
			{
				if (!done)
				{
					unsafeResult = r;
					done = true;
				}
			} });

			QueueUserAPC(HandleMessageLoop, g_renderThread, NULL);

			while (!done && ((GetTickCount64() - g_renderThreadLastPoll) < 100))
			{
				Sleep(0);
			}

			done = true;
		}
		else
		{
			g_wndMsgQueue.push({ hWnd, uMsg, wParam, lParam, {} });
		}

		if (uMsg == WM_EXITSIZEMOVE)
		{
			SetEvent(g_renderThreadGate);
		}

		return unsafeResult;
	}

	LRESULT lr = g_origWndProc(hWnd, uMsg, wParam, lParam);

	if (uMsg == WM_EXITSIZEMOVE)
	{
		SetEvent(g_renderThreadGate);
	}

	return lr;
}

static HANDLE g_windowThreadWakeEvent;
static concurrency::concurrent_queue<std::function<void()>> g_wndFuncQueue;

void WakeWindowThreadFor(std::function<void()>&& func)
{
	if (!g_windowThreadWakeEvent)
	{
		func();
		return;
	}

	g_wndFuncQueue.push(std::move(func));
	SetEvent(g_windowThreadWakeEvent);
}

static CURSORINFO g_ci;
static HANDLE g_ourHwnd;

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
		g_renderThreadId = parentThreadId;

		HWND hWnd;

		g_renderThread = OpenThread(THREAD_SET_CONTEXT, FALSE, parentThreadId);
		g_renderThreadGate = CreateEvent(NULL, TRUE, TRUE, NULL);
		g_windowThreadWakeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

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

				std::function<void()> fn;
				while (g_wndFuncQueue.try_pop(fn))
				{
					fn();
				}

				g_ci.cbSize = sizeof(g_ci);
				GetCursorInfo(&g_ci);

				MSG msg = { 0 };
				while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&msg);
					DispatchMessageW(&msg);
				}
			}
		}).detach();

		WaitForSingleObject(hDoneEvent, INFINITE);
		CloseHandle(hDoneEvent);

		g_ourHwnd = hWnd;

		return hWnd;
	}

	return g_origCreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

static std::function<void(LRESULT)> g_curFn;
static thread_local bool g_inMessageLoop;

static bool done;
static HANDLE g_preRenderThreadKick = CreateEvent(NULL, TRUE, FALSE, NULL);

static BOOL PeekMessageWFake(_Out_ LPMSG lpMsg, _In_opt_ HWND hWnd, _In_ UINT wMsgFilterMin, _In_ UINT wMsgFilterMax, _In_ UINT wRemoveMsg)
{
	if (GetCurrentThreadId() != g_renderThreadId)
	{
		return FALSE;
	}

	g_renderThreadLastPoll = GetTickCount64();

	if (!g_inMessageLoop)
	{
		WaitForSingleObjectEx(g_renderThreadGate, INFINITE, TRUE);
	}

	g_renderThreadLastPoll = GetTickCount64();

	decltype(g_wndMsgQueue)::value_type m;

	if (g_wndMsgQueue.try_pop(m))
	{
		lpMsg->hwnd = std::get<0>(m);
		lpMsg->message = std::get<1>(m);
		lpMsg->wParam = std::get<2>(m);
		lpMsg->lParam = std::get<3>(m);

		if (std::get<4>(m))
		{
			g_curFn = std::move(std::get<4>(m));
		}

		return TRUE;
	}

	return FALSE;
}

static void TranslateMessageFake()
{
}

static void DispatchMessageWFake(_In_ CONST MSG *lpMsg)
{
	LRESULT lr = g_origWndProc(lpMsg->hwnd, lpMsg->message, lpMsg->wParam, lpMsg->lParam);

	if (g_curFn)
	{
		g_curFn(lr);
		g_curFn = {};
	}
}

void HandleMessageLoop(ULONG_PTR)
{
	MSG msg;
	g_inMessageLoop = true;

	while (PeekMessageWFake(&msg, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessageFake();
		DispatchMessageWFake(&msg);
	}

	g_inMessageLoop = false;
}

static BOOL GetCursorInfoFake(PCURSORINFO i)
{
	*i = g_ci;
	return TRUE;
}

static BOOL ShowWindowWrap(_In_ HWND hWnd, _In_ int nCmdShow)
{
	if (hWnd == g_ourHwnd)
	{
		WakeWindowThreadFor([hWnd, nCmdShow]()
		{
			ShowWindow(hWnd, nCmdShow);
		});

		return TRUE;
	}

	return ShowWindow(hWnd, nCmdShow);
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

#include <MinHook.h>

static void (*g_origRenderThreadKick)(void* rti, bool value);

static void RenderThreadKickStub(void* rti, bool value)
{
	if (!done)
	{
		WaitForSingleObject(g_preRenderThreadKick, INFINITE);
		done = true;
		CloseHandle(g_preRenderThreadKick);
	}

	g_origRenderThreadKick(rti, value);
}

static void* (*g_origRenderThreadWait)(void*);

static void* RenderThreadWaitStub(void* a1)
{
	if (!done)
	{
		SetEvent(g_preRenderThreadKick);
	}

	return g_origRenderThreadWait(a1);
}

static HookFunction hookFunction([]()
{
	if (wcsstr(GetCommandLineW(), L"-otw") == nullptr)
	{
		return;
	}

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
	hook::iat("user32.dll", ShowWindowWrap, "ShowWindow");

	// render thread kick waiting until render thread settles
	MH_Initialize();
	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? 45 33 FF 44 38 7B 0D 74 0E")), RenderThreadKickStub, (void**)&g_origRenderThreadKick);
	MH_CreateHook(hook::get_call(hook::get_pattern("48 8B F9 48 83 C1 50 E8 ? ? ? ? 48 8B D8 48 85", 7)), RenderThreadWaitStub, (void**)&g_origRenderThreadWait);
	MH_EnableHook(MH_ALL_HOOKS);

	// watch dog function for weird counters
	{
		auto location = hook::get_pattern<char>("72 08 83 C9 FF E8", 2);
		hook::nop(location, 5 + 3);
		hook::put<uint8_t>(location, 0xBF); // mov edi, imm32 ...
		hook::put<uint32_t>(location + 1, 0); // mov edi, 0
	}
});
