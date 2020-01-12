#pragma once

#include <Hooking.h>
#include <NetLibrary.h>

extern NetLibrary* g_netLibrary;

extern
	#ifdef COMPILING_GTA_GAME_NY
	__declspec(dllexport)
	#else
	__declspec(dllimport)
	#endif
	fwEvent<const char*> OnRender;
