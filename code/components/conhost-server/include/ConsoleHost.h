#pragma once

#ifndef _WIN32
#include <CoreConsole.h>

namespace ConHost
{
	inline void Print(ConsoleChannel channel, const std::string& string)
	{
		
	}
}
#else
#include <../../conhost-v2/include/ConsoleHost.h>
#endif
