#pragma once

class InputTarget
{
public:
	virtual inline void KeyDown(UINT vKey, UINT scanCode) {}

	virtual inline void KeyUp(UINT vKey, UINT scanCode) {}

	virtual inline void MouseDown(int buttonIdx, int x, int y) {}

	virtual inline void MouseUp(int buttonIdx, int x, int y) {}

	virtual inline void MouseWheel(int delta) {}

	virtual inline void MouseMove(int x, int y) {}
};
