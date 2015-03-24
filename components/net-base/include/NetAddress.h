/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <boost/optional.hpp>

#include <NetBase.h>

namespace net
{
class 
#ifdef COMPILING_NET_BASE
	DLL_EXPORT
#endif
	PeerAddress
{
private:
	union AddressData
	{
		sockaddr_storage addr;
		sockaddr_in in4;
		sockaddr_in6 in6;
	};

	AddressData m_addr;

public:
	//
	// The type of address resolution to perform in FromString.
	//
	enum class LookupType
	{
		// No name resolution will be performed at all.
		NoResolution,
		// Names will be resolved (`A` query in DNS)
		ResolveName,
		// Names and service records will be resolved, overriding the default port if any.
		ResolveWithService
	};

private:
	inline PeerAddress()
	{
		memset(&m_addr, 0, sizeof(m_addr));
	}

	static boost::optional<std::string> LookupServiceRecord(const std::string& serviceHost, uint16_t* servicePort);

public:
	//
	// Create an instance of this structure from a standard socket API address.
	//`
	PeerAddress(const sockaddr* addr, int addrlen);

	//
	// Resolve the address of a remote peer from a passed string in ASCII notation.
	//
	static boost::optional<PeerAddress> FromString(const std::string& str, int defaultPort = 30120, LookupType lookupType = LookupType::ResolveWithService);

	//
	// Get the address family this peer address represents.
	//
	inline int GetAddressFamily()
	{
		return m_addr.addr.ss_family;
	}

	//
	// Obtain the contained address as an abstract sockaddr* structure pointer.
	//
	inline sockaddr* GetSocketAddress()
	{
		return (sockaddr*)&m_addr.addr;
	}

	//
	// Obtain a value suitable for use in the the 'addrlen' field of a socket API call.
	//
	int GetSocketAddressLength();

	//
	// Present the address as a canonical string.
	//
	std::string ToString();
};
}