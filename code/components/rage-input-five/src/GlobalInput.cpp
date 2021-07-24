// code adapted from https://github.com/mumble-voip/mumble/blob/cc291eee2e0407f6b8d170d1aa5d43c9c7b4b18b/src/mumble/GlobalShortcut_win.cpp
// Copyright 2007-2021 The Mumble Developers. All rights reserved.

#include <StdInc.h>
#include <GlobalInput.h>

#include <hidsdi.h>
#include <hidpi.h>

struct GlobalInputHandlerLocal : GlobalInputHandler, std::enable_shared_from_this<GlobalInputHandlerLocal>
{
	GlobalInputHandlerLocal();

	~GlobalInputHandlerLocal();

private:
	void Create();

	LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

	void KeyboardMessage(const uint16_t flags, uint16_t scanCode, const uint16_t virtualKey);

private:
	HWND hWnd = NULL;
	std::thread thread;
};

GlobalInputHandlerLocal::GlobalInputHandlerLocal()
{
	thread = std::thread([this]()
	{
		Create();

		while (hWnd)
		{
			MSG msg = { 0 };

			if (GetMessage(&msg, NULL, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	});
}

LRESULT GlobalInputHandlerLocal::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CREATE:
		{
			constexpr uint8_t nRid = 1;
			RAWINPUTDEVICE rid[nRid] = {};

			rid[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
			rid[0].usUsage = HID_USAGE_GENERIC_KEYBOARD;
			rid[0].dwFlags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY;
			rid[0].hwndTarget = hWnd;

			if (!RegisterRawInputDevices(rid, nRid, sizeof(RAWINPUTDEVICE)))
			{
				trace("RegisterRawInputDevices() failed with error %u!\n", GetLastError());
			}

			break;
		}

		case WM_INPUT:
		{
			auto handle = reinterpret_cast<HRAWINPUT>(lParam);
			UINT size = 0;
			if (GetRawInputData(handle, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) != 0)
			{
				break;
			}

			auto buffer = std::make_unique<uint8_t[]>(size);
			if (GetRawInputData(handle, RID_INPUT, buffer.get(), &size, sizeof(RAWINPUTHEADER)) <= 0)
			{
				break;
			}

			auto input = reinterpret_cast<const PRAWINPUT>(buffer.get());
			switch (input->header.dwType)
			{
				case RIM_TYPEMOUSE:
				{
					const RAWMOUSE& mouse = input->data.mouse;
					// no-op
					break;
				}
				case RIM_TYPEKEYBOARD:
				{
					const RAWKEYBOARD& keyboard = input->data.keyboard;
					if (keyboard.MakeCode == KEYBOARD_OVERRUN_MAKE_CODE)
					{
						// Invalid or unrecognizable combination of keys is pressed or
						// the number of keys pressed exceeds the limit for this keyboard.
						break;
					}

					if (keyboard.VKey == 0xFF)
					{
						// Discard "fake keys" which are part of an escaped sequence.
						break;
					}

					KeyboardMessage(keyboard.Flags, keyboard.MakeCode, keyboard.VKey);
					break;
				}
				case RIM_TYPEHID:
				{
					const RAWHID& hid = input->data.hid;
					std::vector<char> reports(hid.dwSizeHid * hid.dwCount);
					memcpy(reports.data(), hid.bRawData, reports.size());

					// no-op
				}
			}

			break;
		}
		case WM_DESTROY:
			hWnd = NULL;
			break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

static auto GetUSKeyboard()
{
	static HKL usKeyboard = LoadKeyboardLayoutW(L"00000409", 0);
	return usKeyboard;
}

static uint16_t MapGameE1(const uint16_t virtualKey)
{
	uint16_t extFlag = 0;

	switch (virtualKey)
	{
		case VK_LEFT:
		case VK_UP:
		case VK_RIGHT:
		case VK_DOWN:
		case VK_RCONTROL:
		case VK_RMENU:
		case VK_LWIN:
		case VK_RWIN:
		case VK_APPS:
		case VK_PRIOR:
		case VK_NEXT:
		case VK_END:
		case VK_HOME:
		case VK_INSERT:
		case VK_DELETE:
		case VK_DIVIDE:
		case VK_NUMLOCK:
			extFlag = KF_EXTENDED;
			break;
	}

	if (virtualKey == VK_PAUSE)
	{
		return 0x45;
	}

	return MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC) | extFlag;
}

static uint16_t MapGameKey(const uint16_t virtualKey, const uint16_t scanCode, bool isE0)
{
	if (virtualKey == VK_PAUSE)
	{
		return VK_PAUSE;
	}

	auto newVk = MapVirtualKeyExW(scanCode, MAPVK_VSC_TO_VK, GetUSKeyboard());

	switch (newVk)
	{
		// numpad distinction
		case VK_HOME:
			if (!isE0)
			{
				newVk = VK_NUMPAD7;
			}
			break;
		case VK_PRIOR:
			if (!isE0)
			{
				newVk = VK_NUMPAD9;
			}
			break;
		case VK_CLEAR:
			if (!isE0)
			{
				newVk = VK_NUMPAD5;
			}
			break;
		case VK_NEXT:
			if (!isE0)
			{
				newVk = VK_NUMPAD3;
			}
			break;
		case VK_END:
			if (!isE0)
			{
				newVk = VK_NUMPAD1;
			}
			break;
		case VK_LEFT:
			if (!isE0)
			{
				newVk = VK_NUMPAD4;
			}
			break;
		case VK_RIGHT:
			if (!isE0)
			{
				newVk = VK_NUMPAD6;
			}
			break;
		case VK_UP:
			if (!isE0)
			{
				newVk = VK_NUMPAD8;
			}
			break;
		case VK_DOWN:
			if (!isE0)
			{
				newVk = VK_NUMPAD2;
			}
			break;
		case VK_INSERT:
			if (!isE0)
			{
				newVk = VK_NUMPAD0;
			}
			break;
		case VK_DELETE:
			if (!isE0)
			{
				newVk = VK_DECIMAL;
			}
			break;
		case VK_MULTIPLY:
			if (isE0)
			{
				newVk = VK_SNAPSHOT;
			}
			break;
		case VK_NUMLOCK:
			if (!isE0)
			{
				newVk = VK_PAUSE;
			}
			break;
		case VK_RETURN:
			newVk = (isE0) ? VK_PA1 : VK_RETURN;
			break;
		// left/right keys
		case VK_SHIFT:
			newVk = MapVirtualKeyW(scanCode, MAPVK_VSC_TO_VK_EX);
			break;
		case VK_CONTROL:
			newVk = (isE0) ? VK_RCONTROL : VK_LCONTROL;
			break;
		case VK_MENU:
			newVk = (isE0) ? VK_RMENU : VK_LMENU;
			break;
	}

	return newVk;
}

void GlobalInputHandlerLocal::KeyboardMessage(const uint16_t flags, uint16_t scanCode, const uint16_t virtualKey)
{
	if (flags & RI_KEY_E1)
	{
		scanCode = MapGameE1(virtualKey);
	}

	OnKey(MapGameKey(virtualKey, scanCode, flags & RI_KEY_E0), !(flags & RI_KEY_BREAK));
}

void GlobalInputHandlerLocal::Create()
{
	static std::map<HWND, std::weak_ptr<GlobalInputHandlerLocal>> windowMap;

	WNDCLASS wc = { 0 };
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpfnWndProc = [](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) -> LRESULT
	{
		if (uMsg == WM_CREATE)
		{
			auto owner = reinterpret_cast<GlobalInputHandlerLocal*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);
			windowMap[hWnd] = owner->shared_from_this();
			owner->hWnd = hWnd;
		}

		if (auto entry = windowMap.find(hWnd); entry != windowMap.end())
		{
			if (auto wnd = entry->second.lock())
			{
				return wnd->WndProc(uMsg, wParam, lParam);
			}
		}

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	};
	wc.lpszClassName = L"Cfx_GlobalInputWindow";

	RegisterClass(&wc);

	CreateWindow(L"Cfx_GlobalInputWindow", L"CitizenFX Global Input Window", 0, 0, 0, 1, 1, NULL, NULL, GetModuleHandle(NULL), this);
}

GlobalInputHandlerLocal::~GlobalInputHandlerLocal()
{
	if (hWnd)
	{
		DestroyWindow(hWnd);
	}

	if (thread.joinable())
	{
		thread.join();
	}
}

std::shared_ptr<GlobalInputHandler> CreateGlobalInputHandler()
{
	return std::make_shared<GlobalInputHandlerLocal>();
}
