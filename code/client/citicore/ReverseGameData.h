#pragma once

#ifndef IS_FXSERVER
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

	ReverseGameData()
		: mouseX(0), mouseY(0), mouseWheel(0), mouseButtons(0), inputMutex(NULL), produceSema(NULL), consumeSema(NULL), surfaceLimit(0), produceIdx(0), consumeIdx(0), inited(false), isLauncher(false), editWidth(false)
	{
		memset(keyboardState, 0, sizeof(keyboardState));
		memset(surfaces, 0, sizeof(surfaces));
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
