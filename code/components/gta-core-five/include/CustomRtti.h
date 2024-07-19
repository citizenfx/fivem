#pragma once

#include <string>


#ifdef COMPILING_GTA_CORE_FIVE
#define GTA_CORE_EXPORT DLL_EXPORT
#else
#define GTA_CORE_EXPORT DLL_IMPORT
#endif

std::string GTA_CORE_EXPORT SearchTypeName(void* ptr, bool debugFormat = false);
