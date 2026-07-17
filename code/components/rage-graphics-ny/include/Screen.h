/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#pragma once
#include "DrawCommands.h"

inline int GetScreenResolutionX()
{
	int x, y;
	GetGameResolution(x, y);
	return x;
}

inline int GetScreenResolutionY()
{
	int x, y;
	GetGameResolution(x, y);
	return y;
}
