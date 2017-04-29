/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "UvLoopManager.h"

namespace net
{
fwRefContainer<UvLoopHolder> UvLoopManager::GetOrCreate(const std::string& loopTag)
{
	auto it = m_uvLoops.find(loopTag);

	if (it == m_uvLoops.end())
	{
		auto insertPair = m_uvLoops.insert(std::make_pair(loopTag, new UvLoopHolder(loopTag)));
		it = insertPair.first;
	}

	return it->second;
}

void UvLoopManager::Disown(const std::string& loopTag)
{
	m_uvLoops.erase(loopTag);
}
}

static InitFunction initFunction([] ()
{
	Instance<net::UvLoopManager>::Set(new net::UvLoopManager());
}, -5000);