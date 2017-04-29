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
boost::optional<std::string> PeerAddress::LookupServiceRecord(const std::string& serviceHost, uint16_t* servicePort)
{
	// stub
	return boost::optional<std::string>();
}
}