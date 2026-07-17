/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
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