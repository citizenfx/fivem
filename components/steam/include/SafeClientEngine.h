/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#ifdef COMPILING_STEAM
#define STEAM_EXPORT __declspec(dllexport)
#else
#define STEAM_EXPORT
#endif

#include <IClientEngine.h>

IClientEngine* CreateSafeClientEngine(IClientEngine* baseEngine, HSteamPipe steamPipe, HSteamUser steamUser);