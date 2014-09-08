#pragma once

#ifdef COMPILING_RAGE_NUTSNBOLTS_NY
#define NUTS_DECL __declspec(dllexport)
#else
#define NUTS_DECL __declspec(dllimport)
#endif

extern NUTS_DECL fwEvent<> OnPreProcessNet;
extern NUTS_DECL fwEvent<> OnPostProcessNet;
extern NUTS_DECL fwEvent<> OnGameFrame;