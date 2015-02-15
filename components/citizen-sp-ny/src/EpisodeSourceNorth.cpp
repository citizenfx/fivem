/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "EpisodeManager.h"
#include "InitializerFactory.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

class EpisodeSourceNorthNY : public EpisodeSource
{
private:
	fwRefContainer<Episode> CreateNorthEpisode(const std::wstring& path);

public:
	virtual std::vector<fwRefContainer<Episode>> GetEpisodes() override;
};

std::vector<fwRefContainer<Episode>> EpisodeSourceNorthNY::GetEpisodes()
{
	std::vector<fwRefContainer<Episode>> retval;

	// obtain the EFLC InstallPath value from the registry.
	// this value *should* always exist _for EFLC_, as without it EFLC.exe will not start.
	wchar_t installPath[MAX_PATH];
	DWORD installPathSize = sizeof(installPath); // we don't reallocate as Win32 doesn't support larger paths using typical DosDevices syntax anyway

	DWORD result = RegGetValue(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Rockstar Games\\EFLC", L"InstallFolder", RRF_RT_REG_SZ, nullptr, reinterpret_cast<LPBYTE>(installPath), &installPathSize);
	
	if (result == ERROR_SUCCESS)
	{
		DWORD folderAttributes = GetFileAttributes(installPath);

		if (folderAttributes != INVALID_FILE_ATTRIBUTES && (folderAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			// add episodes based on, well, these
			auto addEpisode = [&] (const wchar_t* namePart)
			{
				std::wstring epPath = va(L"%s\\%s\\setup2.xml", installPath, namePart);
				fwRefContainer<Episode> episode = CreateNorthEpisode(epPath);

				if (episode.GetRef())
				{
					retval.push_back(episode);
				}
			};
			
			addEpisode(L"TLaD");
			addEpisode(L"TBoGT");
		}
	}

	return retval;
}

fwRefContainer<Episode> EpisodeSourceNorthNY::CreateNorthEpisode(const std::wstring& path)
{
	std::ifstream stream;
	stream.open(path);

	fwRefContainer<Episode> retval;

	try
	{
		boost::property_tree::ptree pt;
		boost::property_tree::read_xml(stream, pt);

		// iterate through all ini.content nodes
		for (auto& child : pt.get_child("ini"))
		{
			// try to get the 'episode' identifier
			auto& episodeIdRef = child.second.get_optional<int>("episode");

			// if this is actually an episode...
			if (episodeIdRef.is_initialized())
			{
				// get the episode ID and name
				int episodeId = episodeIdRef.value();
				std::string episodeName = child.second.get<std::string>("name");

				// convert the path to non-wide string as the game uses ANSI functions anyway
				char outPath[256];
				wcstombs(outPath, path.c_str(), sizeof(outPath));

				retval = new Episode();
				retval->SetIdentifier(std::string(va("gta-ny-dlc-%d@rockstarnorth.com", child.second.get<int>("id"))));
				retval->SetName(episodeName);
				retval->SetEpisodeInitializer(InitializerFactory::GetSinglePlayerInitializer(episodeId, outPath));
			}
		}
	}
	catch (...)
	{
		trace("parsing of an episode file failed.\n");
	}

	return retval;
}

static InitFunction initFunction([] ()
{
	Instance<EpisodeManager>::Get()->AddEpisodeSource(new EpisodeSourceNorthNY());
});