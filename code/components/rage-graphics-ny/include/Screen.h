/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
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
