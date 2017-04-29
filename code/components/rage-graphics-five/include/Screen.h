#pragma once

#include "DrawCommands.h"

inline int GetScreenResolutionX()
{
	int width, height;
	GetGameResolution(width, height);

	return width;
}

inline int GetScreenResolutionY()
{
	int width, height;
	GetGameResolution(width, height);

	return height;
}