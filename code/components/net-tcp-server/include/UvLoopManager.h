/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "UvLoopHolder.h"

#include <unordered_map>

#ifdef COMPILING_NET_TCP_SERVER
#define TCP_SERVER_EXPORT DLL_EXPORT
#else
#define TCP_SERVER_EXPORT DLL_IMPORT
#endif

namespace net
{
class TCP_SERVER_EXPORT UvLoopManager
{
private:
	std::unordered_map<std::string, fwRefContainer<UvLoopHolder>> m_uvLoops;

public:
	fwRefContainer<UvLoopHolder> Get(const std::string& loopTag);

	fwRefContainer<UvLoopHolder> GetOrCreate(const std::string& loopTag);

	void Disown(const std::string& loopTag);
};
}

DECLARE_INSTANCE_TYPE(net::UvLoopManager);
