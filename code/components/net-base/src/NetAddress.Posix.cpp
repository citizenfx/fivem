/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
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