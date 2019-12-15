/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#ifdef COMPILING_RAGE_NUTSNBOLTS_RDR3
#define NUTSNBOLTS_EXPORT DLL_EXPORT
#else
#define NUTSNBOLTS_EXPORT DLL_IMPORT
#endif

extern NUTSNBOLTS_EXPORT fwEvent<> OnLookAliveFrame;
extern NUTSNBOLTS_EXPORT fwEvent<> OnGameFrame;
extern NUTSNBOLTS_EXPORT fwEvent<> OnMainGameFrame;
extern NUTSNBOLTS_EXPORT fwEvent<> OnCriticalGameFrame;
extern NUTSNBOLTS_EXPORT fwEvent<> OnFirstLoadCompleted;
