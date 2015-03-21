/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#ifdef _WIN32
#include <ws2tcpip.h>
#endif

namespace net
{
#ifdef _WIN32
typedef SOCKET PlatformSocketType;
#else
typedef int PlatformSocketType;
#endif

class UdpSocket : public fwRefCountable
{
private:
	PlatformSocketType m_socket;

	
};
}