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

class
#ifdef COMPILING_GTA_GAME_NY
	DLL_EXPORT
#else
	DLL_IMPORT
#endif
	HudPositions
{
public:
	static HudPosition* ms_hudPositions;

public:
	static HudPosition* GetPosition(int index);

	static HudPosition* GetPosition(const char* name);
};
