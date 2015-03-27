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
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/in6.h> // FIXME for BSD

typedef int PlatformSocketType;

#define GetLastNetError() errno

#define closesocket close
#endif

namespace net
{
	void EnsureNetInitialized();
}