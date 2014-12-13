#include "StdInc.h"
#include "NetLibrary.h"

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
		inet_pton(AF_INET, address, &m_in4.sin_addr);

		m_in4.sin_port = htons(port);
		memset(m_in4.sin_zero, 0, sizeof(m_in4.sin_zero));
	}
}

void NetAddress::GetSockAddr(sockaddr_storage* addr, int* addrLen)
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

bool NetAddress::operator==(const NetAddress& right) const
{
	if (m_type != right.m_type)
	{
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