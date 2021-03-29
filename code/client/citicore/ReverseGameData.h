#pragma once

#ifndef IS_FXSERVER
// size in the game is exactly 96 bytes - 0x60

namespace rage
{
struct ioPad
{
	static constexpr const int TRIGGER_THRESHOLD = 64; // the original function uses a threshold at 0x141d8abb9, it is normally 64.
	static constexpr const int BUTTON_LEFT_TRIGGER = (1 << 0); //1 -- uses TRIGGER_THRESHOLD
	static constexpr const int BUTTON_RIGHT_TRIGGER = (1 << 1); //2 -- ^^^^
	static constexpr const int BUTTON_LEFT_SHOULDER = (1 << 2); //4
	static constexpr const int BUTTON_RIGHT_SHOULDER = (1 << 3); //8
	static constexpr const int BUTTON_Y = (1 << 4); //0x10
	static constexpr const int BUTTON_B = (1 << 5); //0x20
	static constexpr const int BUTTON_A = (1 << 6); //0x40
	static constexpr const int BUTTON_X = (1 << 7); //0x80
	static constexpr const int BUTTON_BACK = (1 << 8); //0x100
	static constexpr const int BUTTON_LEFT_THUMB = (1 << 9); //0x200
	static constexpr const int BUTTON_RIGHT_THUMB = (1 << 10); //0x400
	static constexpr const int BUTTON_START = (1 << 11); //0x800
	static constexpr const int BUTTON_UP = (1 << 12); //0x1000
	static constexpr const int BUTTON_RIGHT = (1 << 13); //0x2000
	static constexpr const int BUTTON_DOWN = (1 << 14); //0x4000
	static constexpr const int BUTTON_LEFT = (1 << 15); //0x8000

	inline void Reset()
	{
		controllerIndex = 0;
		buttonFlags = 0;
		lastButtonFlags = 0;
		stickValues = 0x80808080; // stick neutral = 127/255
		leftTriggerAnalog = 0;
		rightTriggerAnalog = 0;
		leftMotorSpeed = 0;
		rightMotorSpeed = 0;
		isFresh = true; // set this in case someone wants to just assign the struct to the game's memory
	}
	ioPad() 
	{
		Reset();
	}
	virtual ~ioPad() = default;

	int controllerIndex; // could be a smaller value with some unk bytes
	int buttonFlags; // +12
	int lastButtonFlags; // buttonFlags is copied here and then set to 0 on a new cycle +16
	union
	{
		int stickValues;
		// analog sticks are packed into 4 bytes
		struct
		{
			//[0-255]
			//left - | right +
			//up   - | down  +
			uint8_t leftX; // +20
			uint8_t leftY; // +21
			uint8_t rightX; // +22
			uint8_t rightY; // +23
		};
	};

	//[0-255]
	uint8_t leftTriggerAnalog; // +24
	uint8_t rightTriggerAnalog; // +25

	char _pad[24];

	bool isFresh; // +50 - supposedly set to Zero when conditions fail ( like GetXInputState )
	char _padmotor[15];
	short leftMotorSpeed; // +66
	short rightMotorSpeed; // +68
	char _padend[20];
};
}

struct ReverseGameData
{
	uint8_t keyboardState[256];
	int mouseX;
	int mouseY;
	int mouseWheel;
	int mouseButtons;

	HANDLE inputMutex;

	HANDLE produceSema;
	HANDLE consumeSema;
	HANDLE surfaces[4];
	int surfaceLimit;
	uint32_t produceIdx;
	uint32_t consumeIdx;

	int width;
	int height;

	int twidth;
	int theight;

	bool inited;
	bool isLauncher;
	bool createHandles;
	bool editWidth;

	HWND mainWindowHandle;

	uint32_t fpsLimit;

	uint16_t inputChar;

	rage::ioPad gamepad;

	ReverseGameData()
		: mouseX(0), mouseY(0), mouseWheel(0), mouseButtons(0), inputMutex(NULL), produceSema(NULL), consumeSema(NULL), surfaceLimit(0), produceIdx(0), consumeIdx(0), inited(false), isLauncher(false), editWidth(false), fpsLimit(0)
	{
		memset(keyboardState, 0, sizeof(keyboardState));
		memset(surfaces, 0, sizeof(surfaces));
		gamepad.Reset();
	}

	int GetNextSurface(DWORD timeout = INFINITE)
	{
		if (WaitForSingleObject(produceSema, timeout) != WAIT_OBJECT_0)
		{
			return -1;
		}

		return produceIdx;
	}

	void SubmitSurface()
	{
		produceIdx = (produceIdx + 1) % surfaceLimit;
		BOOL r = ReleaseSemaphore(consumeSema, 1, NULL);

		if (!r)
		{
			trace("Signal Sema Error: %d\n", GetLastError());
		}
	}

	int FetchSurfaceIdx()
	{
		int idx = consumeIdx;
		consumeIdx = (consumeIdx + 1) % surfaceLimit;

		return idx;
	}

	int FetchSurface(DWORD timeout = INFINITE)
	{
		if (WaitForSingleObject(consumeSema, timeout) != WAIT_OBJECT_0)
		{
			return -1;
		}

		return FetchSurfaceIdx();
	}

	void ReleaseSurface()
	{
		ReleaseSemaphore(produceSema, 1, NULL);
	}
};
#endif
