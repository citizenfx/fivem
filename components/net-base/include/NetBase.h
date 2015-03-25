/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#if defined(_WIN32)
#include <WS2tcpip.h>

typedef SOCKET PlatformSocketType;

#define GetLastNetError() WSAGetLastError()

#define EAGAIN WSAEWOULDBLOCK
#else
#include <netinet/in.h>

typedef int PlatformSocketType;

#define GetLastNetError() errno

#define closesocket close
#endif

namespace net
{
	void EnsureNetInitialized();
}