/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"

#include <Error.h>
#include "DeferredInitializer.h"

#include <CrossBuildRuntime.h>

struct CrossMappingEntry
{
	uint64_t entries[28];
};

static std::unordered_map<uint64_t, uint64_t> g_mappingTable;
static std::shared_ptr<DeferredInitializer> g_mappingInitializer;
static std::unordered_map<uint64_t, const CrossMappingEntry*> g_unmappedTable;

namespace rage
{
	uint64_t MapNative(uint64_t inNative)
	{
		g_mappingInitializer->Wait();

		// find the native, and return the original if not mapped (for custom natives and 'new' natives)
		auto it = g_mappingTable.find(inNative);

		if (it == g_mappingTable.end())
		{
			return inNative;
		}

		return it->second;
	}

	void ReviveNative(uint64_t inNative)
	{
		g_mappingInitializer->Wait();

		if (auto it = g_unmappedTable.find(inNative); it != g_unmappedTable.end())
		{
			for (uint64_t hash : it->second->entries)
			{
				if (hash != 0 && hash != inNative)
				{
					g_mappingTable.insert({ hash, inNative });
				}
			}
		}
	}
}

extern "C" DLL_EXPORT uint64_t MapNative(uint64_t inNative)
{
	return rage::MapNative(inNative);
}

extern "C" DLL_EXPORT void ReviveNative(uint64_t inNative)
{
	rage::ReviveNative(inNative);
}

static void DoMapping()
{
	// find the game build version
	char* location = hook::pattern("48 8D 8E 09 02 00 00 44 8B C5 33 D2").count(1).get(0).get<char>(20);
	char* buildString = (char*)(location + *(int32_t*)location + 4);

	int versionIdx = -1;
	if (strncmp(buildString, "Dec  5 2024", 11) == 0)
	{
		versionIdx = 3407;
	}
	else if (strncmp(buildString, "Sep 10 2024", 11) == 0)
	{
		versionIdx = 3323;
	}
	else if (strncmp(buildString, "Jun 20 2024", 11) == 0)
	{
		versionIdx = 3258;
	}
	else if (strncmp(buildString, "Dec  8 2023", 11) == 0)
	{
		versionIdx = 3095;
	}
	else if (strncmp(buildString, "Jun  8 2023", 11) == 0)
	{
		versionIdx = 2944;
	}
	else if (strncmp(buildString, "Dec  8 2022", 11) == 0)
	{
		versionIdx = 2802;
	}
	else if (strncmp(buildString, "Jul 21 2022", 11) == 0)
	{
		versionIdx = 2699;
	}
	else if (strncmp(buildString, "Apr 18 2022", 11) == 0)
	{
		versionIdx = 2612;
	}
	else if (strncmp(buildString, "Dec 14 2021", 11) == 0)
	{
		versionIdx = 2545;
	}
	else if (strncmp(buildString, "Jul 15 2021", 11) == 0)
	{
		versionIdx = 2372;
	}
	else if (strncmp(buildString, "Dec 10 2020", 11) == 0)
	{
		versionIdx = 2189;
	}
	else if (strncmp(buildString, "Aug  5 2020", 11) == 0)
	{
		versionIdx = 2060;
	}
	else if (strncmp(buildString, "Dec 11 2019", 11) == 0)
	{
		versionIdx = 1868;
	}
	else if (strncmp(buildString, "Jul 23 2019", 11) == 0)
	{
		versionIdx = 1737;
	}
	else if (strncmp(buildString, "Dec  5 2018", 11) == 0)
	{
		versionIdx = 1604; 
	}
	else if (strncmp(buildString, "Oct 14 2018", 11) == 0)
	{
		versionIdx = 1493; // .1
	}
	else if (strncmp(buildString, "Mar 14 2018", 11) == 0)
	{
		versionIdx = 1365;
	}
	else if (strncmp(buildString, "Dec  6 2017", 11) == 0)
	{
		versionIdx = 1290;
	}
	else if (strncmp(buildString, "Jun  9 2017", 11) == 0)
	{
		versionIdx = 1103;
	}
	else if (strncmp(buildString, "Mar 31 2017", 6) == 0)
	{
		versionIdx = 1032;
	}
	else if (strncmp(buildString, "Oct 13", 6) == 0)
	{
		versionIdx = 505;
	}
	else if (strncmp(buildString, "Jun 30", 6) == 0)
	{
		versionIdx = 393;
	}
	else if (strncmp(buildString, "Jun  9 2015", 11) == 0)
	{
		versionIdx = 372;
	}

	// early out if no version index matched
	if (versionIdx < 0)
	{
		FatalError("No native mapping information found for game executable built on %s.", buildString);
	}

	static const CrossMappingEntry crossMapping_universal[] =
	{
#include "CrossMapping_Universal.h"
	};

	/*
		b2802 entry:
		0x2ACCB195F3CCD9DE -> b2545 begin + reshuffle
		0x8E580AB902917360 -> b2545 end
		0x138679CA01E21F53 -> b2612 begin
		0xFCE2747EEF1D05FC -> b2612 end
		0x65671A4FB8218930 -> b2699 begin
		0xCAC2031EBF79B1A8 -> b2699 end
		0xAC0BB4D87777CAE2 -> b2802 begin + reshuffle
	*/

	int maxVersion = 0;
	auto newVersions = { 350, 372, 393, 463, 505, 573, 617, 678, 757, 791, 877, 944, 1011, 1103, 1180, 1290, 1365, 1493, 1604, 1737, 1868, 2060, 2189, 2372, 2545, 2802, 2944 };

	for (auto version : newVersions)
	{
		if (versionIdx >= version)
		{
			++maxVersion;
		}
	}

	if (xbr::IsGameBuild<1604>())
	{
		assert(maxVersion == 19);
	}
	else if (xbr::IsGameBuild<2060>())
	{
		assert(maxVersion == 22);
	}
	else if (xbr::IsGameBuild<2189>())
	{
		assert(maxVersion == 23);
	}
	else if (xbr::IsGameBuild<2372>())
	{
		assert(maxVersion == 24);
	}
	else if (xbr::IsGameBuild<2545>() || xbr::IsGameBuild<2612>() || xbr::IsGameBuild<2699>())
	{
		assert(maxVersion == 25);
	}
	else if (xbr::IsGameBuild<2802>())
	{
		assert(maxVersion == 26);
	}
	else if (xbr::IsGameBuildOrGreater<2944>())
	{
		assert(maxVersion == 27);
	}
	else
	{
		assert(!"Didn't define maxVersion assertion!");
	}

	g_mappingInitializer = DeferredInitializer::Create([maxVersion]()
	{
		for (auto& nativeEntry : crossMapping_universal)
		{
			bool hasNative = nativeEntry.entries[maxVersion] != 0;
			for (int i = 0; i < maxVersion; i++)
			{
				if (uint64_t hash = nativeEntry.entries[i])
				{
					if (hasNative)
					{
						g_mappingTable.insert({ hash, nativeEntry.entries[maxVersion] });
					}
					else
					{
						// Native was removed this version: add it to a separate table
						// so it can potentially be revived by custom implementations.
						g_unmappedTable.insert({ hash, &nativeEntry });
					}
				}
			}
		}
	});
}

static HookFunction hookFunction([] ()
{
	DoMapping();
});
