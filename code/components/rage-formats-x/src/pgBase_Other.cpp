/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#if defined(GTA_FIVE) || defined(IS_RDR3)
#define RAGE_FORMATS_GAME ny
#define RAGE_FORMATS_GAME_NY
#else
#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE
#endif

#include "pgBase.cpp"
