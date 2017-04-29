/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NetAddress.h"

namespace net
{
PeerAddress::PeerAddress(const sockaddr* addr, socklen_t addrlen)
	: PeerAddress()
{
	assert(addrlen < sizeof(m_addr));

	memcpy(&m_addr, addr, addrlen);
}

boost::optional<PeerAddress> PeerAddress::FromString(const std::string& str, int defaultPort /* = 30120 */, LookupType lookupType /* = LookupType::ResolveWithService */)
{
	// ensure networking is initialized
	EnsureNetInitialized();

	// first, find a port argument in the string and see if it precedes any ']' (i.e. an IPv6 address of the form [::1] should not be seen as port 1)
	int portIdx = str.find_last_of(':');
	uint16_t port = defaultPort;

	std::string resolveName = str;

	if (portIdx != std::string::npos)
	{
		int bracketIdx = str.find_last_of(']');

		if (bracketIdx == std::string::npos || portIdx > bracketIdx)
		{
			resolveName = str.substr(0, portIdx);
			port = atoi(str.substr(portIdx + 1).c_str());
		}
	}

	boost::optional<PeerAddress> retval;

	// if we're supposed to resolve the passed name, try that
	if (lookupType == LookupType::ResolveName || lookupType == LookupType::ResolveWithService)
	{
		// however, if we're supposed to look up a service, perform the lookup for the service first
		if (lookupType == LookupType::ResolveWithService)
		{
			boost::optional<std::string> serviceName = LookupServiceRecord("_cfx._udp." + resolveName, &port);

			if (serviceName.is_initialized())
			{
				resolveName = serviceName.get();
			}
		}

		// then look up the actual host
		addrinfo* addrInfos;

		if (getaddrinfo(resolveName.c_str(), va("%u", port), nullptr, &addrInfos) == 0)
		{
			// loop through for both IPv4 and IPv6
			const int families[] = { AF_INET, AF_INET6, -1 };
			int curFamily = 0;

			while (!retval.is_initialized() && families[curFamily] != -1)
			{
				addrinfo* curInfo = addrInfos;

				while (curInfo)
				{
					// TODO: prioritize ipv6 properly for cases where such connectivity exists
					if (curInfo->ai_family == families[curFamily])
					{
						retval = PeerAddress(curInfo->ai_addr, curInfo->ai_addrlen);

						break;
					}

					curInfo = curInfo->ai_next;
				}

				curFamily++;
			}

			freeaddrinfo(addrInfos);
		}
	}
	else
	{
		assert(!"implement non-resolved names!");
	}

	return retval;
}

boost::optional<PeerAddress> PeerAddress::FromString(const char* str, int defaultPort /* = 30120 */, LookupType lookupType /* = LookupType::ResolveWithService */)
{
	return FromString(std::string(str), defaultPort, lookupType);
}

socklen_t PeerAddress::GetSocketAddressLength() const
{
	switch (m_addr.addr.ss_family)
	{
		case AF_INET:
			return sizeof(sockaddr_in);

		case AF_INET6:
			return sizeof(sockaddr_in6);

		default:
			assert(!"Unknown sockaddr family");
			return -1;
	}
}

std::string PeerAddress::ToString() const
{
	// ensure networking is initialized
	EnsureNetInitialized();

	// call inet_ntop on it
	char stringBuf[256];
	int family = m_addr.addr.ss_family;
	uint16_t port;

	switch (family)
	{
		case AF_INET:
			inet_ntop(AF_INET, const_cast<in_addr*>(&m_addr.in4.sin_addr), stringBuf, sizeof(stringBuf));

			port = m_addr.in4.sin_port;
			break;

		case AF_INET6:
			inet_ntop(AF_INET6, const_cast<in6_addr*>(&m_addr.in6.sin6_addr), stringBuf, sizeof(stringBuf));

			port = m_addr.in6.sin6_port;
			break;
	}

	// create a string
	std::string str;

	// add brackets if IPv6
	if (family == AF_INET6)
	{
		str = va("[%s]", stringBuf);
	}
	else
	{
		str = stringBuf;
	}

	// and add the port
	str += ":" + std::to_string(ntohs(port));

	return str;
}
}