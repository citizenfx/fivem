/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
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

typedef int PlatformSocketType;

#define GetLastNetError() errno

#define closesocket close
#endif

namespace net
{
	void EnsureNetInitialized();
}
