/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
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