#include "StdInc.h"

#if 0
#include "NetLibrary.h"

NetAddress::NetAddress(const char* address, uint16_t port)
{
	m_type = NA_INET4;
	m_in4.sin_family = AF_INET;
	m_in4.sin_addr.s_addr = inet_addr(address);
	m_in4.sin_port = htons(port);
	memset(m_in4.sin_zero, 0, sizeof(m_in4.sin_zero));
}

void NetAddress::GetSockAddr(sockaddr_storage* addr, int* addrLen)
{
	if (m_type == NA_INET4)
	{
		memcpy(addr, &m_in4, sizeof(sockaddr_in));
		*addrLen = sizeof(sockaddr_in);
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

	return -1;
}

std::string NetAddress::GetAddress()
{
	if (m_type == NA_INET4)
	{
		static char buffer[16];

		inet_ntop(m_in4.sin_family, &m_in4.sin_addr, buffer, _countof(buffer));

		return buffer;
	}
}

std::wstring NetAddress::GetWAddress()
{
	if (m_type == NA_INET4)
	{
		static char buffer[16];
		static wchar_t bufferW[16];

		inet_ntop(m_in4.sin_family, &m_in4.sin_addr, buffer, _countof(buffer));
		mbstowcs(bufferW, buffer, _countof(bufferW));

		return bufferW;
	}
}
#endif