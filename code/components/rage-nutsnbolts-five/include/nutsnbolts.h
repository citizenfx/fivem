/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#ifdef COMPILING_RAGE_NUTSNBOLTS_FIVE
#define NUTSNBOLTS_EXPORT DLL_EXPORT
#else
#define NUTSNBOLTS_EXPORT DLL_IMPORT
#endif

#define HAS_EARLY_GAME_FRAME

extern NUTSNBOLTS_EXPORT DWORD g_mainThreadId;

extern NUTSNBOLTS_EXPORT fwEvent<> OnLookAliveFrame;
extern NUTSNBOLTS_EXPORT fwEvent<> OnGameFrame;
extern NUTSNBOLTS_EXPORT fwEvent<> OnEarlyGameFrame;
extern NUTSNBOLTS_EXPORT fwEvent<> OnMainGameFrame;
extern NUTSNBOLTS_EXPORT fwEvent<> OnCriticalGameFrame;
extern NUTSNBOLTS_EXPORT fwEvent<> OnFirstLoadCompleted;

extern NUTSNBOLTS_EXPORT fwEvent<> OnBeginGameFrame;
extern NUTSNBOLTS_EXPORT fwEvent<> OnEndGameFrame;
