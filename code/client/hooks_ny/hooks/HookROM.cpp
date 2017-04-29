// patches and stuff for read-only memory

#include "StdInc.h"

#define WM_ROM (43858)

struct dataStruct
{
	char a[16];
	int action;
	char* act100Addr;
	char* act18Addr;
};

const static uint8_t staticData[] = { 0x08, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 0xF1, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0xF3, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xF2, 0x00, 0x00, 0x00 };

static LRESULT WINAPI WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_ROM)
	{
		dataStruct* str = (dataStruct*)lParam;

		switch (str->action)
		{
			case 100:
				*(DWORD*)(str->act100Addr) = 1;
				break;
			case 18:
				memcpy(str->act18Addr, staticData, sizeof(staticData));
				break;
			case 51:
				return (LRESULT)(str->act100Addr + (DWORD)str->act18Addr);
		}

		return 0;
	}

	return 1;
}

static HWND CreateROMWindow()
{
	WNDCLASSEX wc;
	memset(&wc, 0, sizeof(wc));
	wc.cbSize = sizeof(wc);

	wc.style = 3;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = L"banana";
	wc.hInstance = GetModuleHandle(NULL);
	wc.hbrBackground = (HBRUSH)5;
	wc.hCursor = LoadCursor(0, MAKEINTRESOURCE(0x7F00));

	RegisterClassEx(&wc);

	HWND hWnd = CreateWindowEx(0, L"banana", L"", 0xCC00000, 0, 0, 0, 0, 0, 0, GetModuleHandle(NULL), 0);

	return hWnd;
}

void __stdcall SendMessageFakie(int, int, int, int)
{

}

static HookFunction hookFunction([] ()
{
	// windows message id
	hook::put<uint32_t>(0x1724240, WM_ROM);

	// window handle
	hook::put<HWND>(0x1724234, CreateROMWindow());

	// SendMessage call to this window that ends up freezing the audio update thread and resulting in a deadlock
	hook::nop(0x79EF7D, 6);
	hook::call(0x79EF7D, SendMessageFakie);

	// no idea how related this is to this file, but still putting it here: some entity-related function which sometimes ends up looping infinitely
	// this patch has no apparent side effects; forces only one iteration to run
	hook::nop(0xA4E17A, 6);
});