/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NetLibrary.h"

NetAddress::NetAddress(const ENetAddress* address)
{
	m_type = NA_INET6;

	memset(&m_in6, 0, sizeof(m_in6));
	m_in6.sin6_family = AF_INET6;

	memcpy(&m_in6.sin6_addr, &address->host, sizeof(in6_addr));

	m_in6.sin6_port = htons(address->port);
	m_in6.sin6_scope_id = address->sin6_scope_id;
}

NetAddress::NetAddress(const sockaddr* sockaddr)
{
	auto family = sockaddr->sa_family;

	if (family == AF_INET)
	{
		m_in4 = *(sockaddr_in*)sockaddr;
		m_type = NA_INET4;
	}
	else if (family == AF_INET6)
	{
		m_in6 = *(sockaddr_in6*)sockaddr;
		m_type = NA_INET6;
	}
}

NetAddress::NetAddress(const char* address, uint16_t port)
{
	if (address[0] == '[')
	{
		m_type = NA_INET6;

		memset(&m_in6, 0, sizeof(m_in6));
		m_in6.sin6_family = AF_INET6;

		inet_pton(AF_INET6, &address[1], &m_in6.sin6_addr);

		m_in6.sin6_port = htons(port);
	}
	else
	{
		m_type = NA_INET4;

		m_in4.sin_family = AF_INET;

		addrinfo* ai;
		if (getaddrinfo(address, nullptr, nullptr, &ai) >= 0)
		{
			for (addrinfo* i = ai; i; i = i->ai_next)
			{
				if (i->ai_family == AF_INET)
				{
					memcpy(&m_in4, i->ai_addr, sizeof(m_in4));
					break;
				}
			}

			freeaddrinfo(ai);
		}

		m_in4.sin_port = htons(port);
		memset(m_in4.sin_zero, 0, sizeof(m_in4.sin_zero));
	}
}

void NetAddress::GetSockAddr(sockaddr_storage* addr, int* addrLen) const
{
	if (m_type == NA_INET4)
	{
		memcpy(addr, &m_in4, sizeof(sockaddr_in));
		*addrLen = sizeof(sockaddr_in);
	}
	else if (m_type == NA_INET6)
	{
		memcpy(addr, &m_in6, sizeof(sockaddr_in6));
		*addrLen = sizeof(sockaddr_in6);
	}
}

ENetAddress NetAddress::GetENetAddress() const
{
	ENetAddress addr = { 0 };

	if (m_type == NA_INET4)
	{
		addr.host.u.Byte[10] = 0xFF;
		addr.host.u.Byte[11] = 0xFF;

		memcpy(&addr.host.u.Byte[12], &m_in4.sin_addr.S_un.S_un_b.s_b1, 4);
		addr.port = ntohs(m_in4.sin_port);
		addr.sin6_scope_id = 0;
	}
	else if (m_type == NA_INET6)
	{
		memcpy(&addr.host, &m_in6.sin6_addr, sizeof(addr.host));
		addr.port = ntohs(m_in6.sin6_port);
		addr.sin6_scope_id = m_in6.sin6_scope_id;
	}

	return addr;
}

bool NetAddress::operator==(const NetAddress& right) const
{
	if (m_type != right.m_type)
	{
		if (m_type == NA_INET4 && right.m_type == NA_INET6)
		{
			return (m_in4.sin_port == right.m_in6.sin6_port && memcmp(&m_in4.sin_addr, &right.m_in6.sin6_addr.u.Byte[12], 4) == 0);
		}
		else if (m_type == NA_INET6 && right.m_type == NA_INET4)
		{
			return (right.m_in4.sin_port == m_in6.sin6_port && memcmp(&right.m_in4.sin_addr, &m_in6.sin6_addr.u.Byte[12], 4) == 0);
		}

		return false;
	}

	if (m_type == NA_INET4)
	{
		return !memcmp(&m_in4, &right.m_in4, sizeof(m_in4));
	}
	else if (m_type == NA_INET6)
	{
		return !memcmp(&m_in6, &right.m_in6, sizeof(m_in6));
	}

	return false;
}

bool NetAddress::operator!=(const NetAddress& right) const
{
	return !(*this == right);
}

int NetAddress::GetPort()
{
	if (m_type == NA_INET4)
	{
		return ntohs(m_in4.sin_port);
	}
	else if (m_type == NA_INET6)
	{
		return ntohs(m_in6.sin6_port);
	}

	return -1;
}

fwString NetAddress::GetAddress()
{
	if (m_type == NA_INET4)
	{
		static char buffer[16];

		inet_ntop(m_in4.sin_family, &m_in4.sin_addr, buffer, _countof(buffer));

		return buffer;
	}
	else if (m_type == NA_INET6)
	{
		static char buffer[80];

		inet_ntop(m_in6.sin6_family, &m_in6.sin6_addr, buffer, _countof(buffer));

		return buffer;
	}

	return "unknown";
}

fwWString NetAddress::GetWAddress()
{
	if (m_type == NA_INET4)
	{
		static char buffer[16];
		static wchar_t bufferW[16];

		inet_ntop(m_in4.sin_family, &m_in4.sin_addr, buffer, _countof(buffer));
		mbstowcs(bufferW, buffer, _countof(bufferW));

		return bufferW;
	}
	else if (m_type == NA_INET6)
	{
		static char buffer[80];
		static wchar_t bufferW[80];

		inet_ntop(m_in6.sin6_family, &m_in6.sin6_addr, buffer, _countof(buffer));
		mbstowcs(bufferW, buffer, _countof(bufferW));

		return va(L"[%s]", bufferW);
	}

	return L"unknown";
}