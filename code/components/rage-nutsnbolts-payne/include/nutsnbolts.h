#pragma once

#ifdef COMPILING_RAGE_NUTSNBOLTS_PAYNE
#define NUTS_DECL __declspec(dllexport)
#else
#define NUTS_DECL __declspec(dllimport)
#endif

extern NUTS_DECL fwEvent<> OnGameFrame;