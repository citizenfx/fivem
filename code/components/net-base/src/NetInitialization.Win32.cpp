/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
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
