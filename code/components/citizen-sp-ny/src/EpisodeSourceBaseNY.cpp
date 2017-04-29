/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "EpisodeManager.h"
#include "InitializerFactory.h"

class EpisodeSourceBaseNY : public EpisodeSource
{
public:
	virtual std::vector<fwRefContainer<Episode>> GetEpisodes() override;
};

std::vector<fwRefContainer<Episode>> EpisodeSourceBaseNY::GetEpisodes()
{
	std::vector<fwRefContainer<Episode>> retval;
	
	if (GetFileAttributes(MakeRelativeGamePath(L"876bd1d9393712ac.bin").c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		fwRefContainer<Episode> baseEpisode(new Episode());

		baseEpisode->SetName(std::string("Grand Theft Auto IV"));
		baseEpisode->SetIdentifier(std::string("gta-ny@rockstarnorth.com"));
		baseEpisode->SetEpisodeInitializer(InitializerFactory::GetSinglePlayerInitializer(0, ""));

		retval.push_back(baseEpisode);
	}

	return retval;
}

static InitFunction initFunction([] ()
{
	Instance<EpisodeManager>::Get()->AddEpisodeSource(new EpisodeSourceBaseNY());
});