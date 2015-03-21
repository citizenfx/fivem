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
PeerAddress::PeerAddress(const sockaddr* addr, int addrlen)
	: PeerAddress()
{
	assert(addrlen < sizeof(m_addr));

	memcpy(&m_addr, addr, addrlen);
}

boost::optional<PeerAddress> PeerAddress::FromString(const std::string& str, int defaultPort /* = 30120 */, LookupType lookupType /* = LookupType::ResolveWithService */)
{
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
			addrinfo* curInfo = addrInfos;

			do
			{
				// TODO: handle ipv6 properly for cases where such connectivity exists
				if (curInfo->ai_family == AF_INET)
				{
					retval = PeerAddress(curInfo->ai_addr, curInfo->ai_addrlen);

					break;
				}
			} while (curInfo->ai_next);

			freeaddrinfo(addrInfos);
		}
	}
	else
	{
		assert(!"implement non-resolved names!");
	}

	return retval;
}
}

static InitFunction initFunction([] ()
{
	WSADATA data;
	WSAStartup(MAKEWORD(2, 2), &data);

	auto addr = net::PeerAddress::FromString(std::string("fivem.net"));

	printf("");
});