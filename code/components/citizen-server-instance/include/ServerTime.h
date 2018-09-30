#pragma once

#include <chrono>

#ifdef COMPILING_CITIZEN_SERVER_INSTANCE
DLL_EXPORT
#else
DLL_IMPORT
#endif
	std::chrono::milliseconds msec();
