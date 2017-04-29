#pragma once

struct HudPosition
{
	int pad;
	float x;
	float y;
	float w;
	float h;
	int alpha;
	int colorIdx;
};

class GAMESPEC_EXPORT HudPositions
{
private:
	static HudPosition* ms_hudPositions;

public:
	static HudPosition* GetPosition(int index);

	static HudPosition* GetPosition(const char* name);
};