/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "UvLoopHolder.h"

#include <unordered_map>

namespace net
{
class UvLoopManager
{
private:
	std::unordered_map<std::string, fwRefContainer<UvLoopHolder>> m_uvLoops;

public:
	fwRefContainer<UvLoopHolder> GetOrCreate(const std::string& loopTag);

	void Disown(const std::string& loopTag);
};
}

DECLARE_INSTANCE_TYPE(net::UvLoopManager);