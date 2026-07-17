/*
* This file is part of the Cfx project - https://cfx.re/
*
* See LICENSE in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"

#if defined(GTA_FIVE)
#define RAGE_FORMATS_GAME rdr3
#define RAGE_FORMATS_GAME_RDR3

#include "pgBase.cpp"
#elif defined(IS_RDR3)
#define RAGE_FORMATS_GAME five
#define RAGE_FORMATS_GAME_FIVE

#include "pgBase.cpp"
#include "BlockDump.cpp"
#endif
