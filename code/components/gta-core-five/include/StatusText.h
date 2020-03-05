#pragma once

#ifdef COMPILING_GTA_CORE_FIVE
#define GTACORE_EXPORT DLL_EXPORT
#else
#define GTACORE_EXPORT DLL_IMPORT
#endif

void GTACORE_EXPORT ActivateStatusText(const char* string, int type, int priority);
void GTACORE_EXPORT DeactivateStatusText(int priority);
