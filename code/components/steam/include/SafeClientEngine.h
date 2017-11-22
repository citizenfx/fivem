/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

#ifdef COMPILING_STEAM
#define STEAM_EXPORT __declspec(dllexport)
#else
#define STEAM_EXPORT
#endif

#include <IClientEngine.h>

IClientEngine* CreateSafeClientEngine(IClientEngine* baseEngine, HSteamPipe steamPipe, HSteamUser steamUser);