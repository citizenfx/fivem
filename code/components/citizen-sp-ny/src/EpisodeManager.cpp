/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "EpisodeManager.h"

void EpisodeManager::Initialize()
{
	for (auto&& source : m_episodeSources)
	{
		for (auto&& episode : source->GetEpisodes())
		{
			m_episodes.push_back(episode);
		}
	}
}

void EpisodeManager::AddEpisodeSource(fwRefContainer<EpisodeSource> episodeSource)
{
	m_episodeSources.push_back(episodeSource);
}

static InitFunction initFunction([] ()
{
	Instance<EpisodeManager>::Set(new EpisodeManager());
}, -500);

static InitFunction initFunctionPost([] ()
{
	Instance<EpisodeManager>::Get()->Initialize();
}, 500);