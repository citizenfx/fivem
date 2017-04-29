/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#ifdef COMPILING_RAGE_NUTSNBOLTS_NY
#define NUTS_DECL __declspec(dllexport)
#else
#define NUTS_DECL __declspec(dllimport)
#endif

extern NUTS_DECL fwEvent<> OnPreProcessNet;
extern NUTS_DECL fwEvent<> OnPostProcessNet;
extern NUTS_DECL fwEvent<> OnGameFrame;