/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#if defined(_WIN32)
#include <WS2tcpip.h>

typedef SOCKET PlatformSocketType;

#define GetLastNetError() WSAGetLastError()

#define FX_EAGAIN WSAEWOULDBLOCK
#else
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef int PlatformSocketType;

#define GetLastNetError() errno

#define closesocket close

#define FX_EAGAIN EAGAIN
#endif

namespace net
{
	void EnsureNetInitialized();
}
