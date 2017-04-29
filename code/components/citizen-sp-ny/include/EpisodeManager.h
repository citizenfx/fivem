/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

class Episode;

class Episode : public fwRefCountable
{
public:
	typedef std::function<bool(fwRefContainer<Episode>)> EpisodeInitializer;

private:
	std::string m_name;

	std::string m_identifier;

	EpisodeInitializer m_episodeInitializer;

public:
	inline std::string GetName() { return m_name; }

	inline void SetName(const std::string& name) { m_name = name; }

	inline std::string GetIdentifier() { return m_identifier; }

	inline void SetIdentifier(const std::string& identifier) { m_identifier = identifier; }

	inline bool RunEpisode() { return m_episodeInitializer(this); }

	inline void SetEpisodeInitializer(const EpisodeInitializer& initializer) { m_episodeInitializer = initializer; }
};

class EpisodeSource : public fwRefCountable
{
public:
	virtual std::vector<fwRefContainer<Episode>> GetEpisodes() = 0;
};

class EpisodeManager
{
private:
	std::vector<fwRefContainer<EpisodeSource>> m_episodeSources;

	std::vector<fwRefContainer<Episode>> m_episodes;

public:
	void Initialize();

	void AddEpisodeSource(fwRefContainer<EpisodeSource> episodeSource);

	inline const std::vector<fwRefContainer<Episode>>& GetEpisodes()
	{
		return m_episodes;
	}
};

DECLARE_INSTANCE_TYPE(EpisodeManager);