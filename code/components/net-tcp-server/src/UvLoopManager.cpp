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
fwRefContainer<UvLoopHolder> UvLoopManager::Get(const std::string& loopTag)
{
	auto it = m_uvLoops.find(loopTag);

	return (it == m_uvLoops.end()) ? nullptr : it->second;
}

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

static thread_local UvLoopHolder* g_uvLoop;

fwRefContainer<UvLoopHolder> UvLoopManager::GetCurrent()
{
	return g_uvLoop;
}

void UvLoopManager::SetCurrent(UvLoopHolder* holder)
{
	g_uvLoop = holder;
}

void UvLoopManager::Disown(const std::string& loopTag)
{
	m_uvLoops.erase(loopTag);
}
}

struct Init
{
	Init()
	{
		Instance<net::UvLoopManager>::Set(new net::UvLoopManager());
	}
} init;
