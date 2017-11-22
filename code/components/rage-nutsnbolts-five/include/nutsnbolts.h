/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

#ifdef COMPILING_RAGE_NUTSNBOLTS_FIVE
#define NUTSNBOLTS_EXPORT DLL_EXPORT
#else
#define NUTSNBOLTS_EXPORT DLL_IMPORT
#endif

extern NUTSNBOLTS_EXPORT fwEvent<> OnGameFrame;
extern NUTSNBOLTS_EXPORT fwEvent<> OnMainGameFrame;
extern NUTSNBOLTS_EXPORT fwEvent<> OnFirstLoadCompleted;