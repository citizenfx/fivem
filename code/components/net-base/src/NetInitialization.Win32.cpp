/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#include "StdInc.h"
#include "NetBase.h"

#include <mutex>

#include "Error.h"

namespace net
{
static std::once_flag g_netInitFlag;

void EnsureNetInitialized()
{
	std::call_once(g_netInitFlag, [] ()
	{
		WSADATA data;
		
		if (WSAStartup(MAKEWORD(2, 2), &data) != 0)
		{
			FatalError("WSAStartup failed!");
		}
	});
}
}
