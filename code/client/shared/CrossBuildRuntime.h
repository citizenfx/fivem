#pragma once

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/wstringize.hpp>

#include <unordered_map>

#ifdef GTA_FIVE
#define GAME_BUILDS \
	(3258) \
	(3095) \
	(2944) \
	(2802) \
	(2699) \
	(2612) \
	(2545) \
	(2372) \
	(2189) \
	(2060) \
	(1604) \
	(1)  // Special build that is used to remove all DLCs from the game.
#elif defined(IS_RDR3)
#define GAME_BUILDS \
	(1491) \
	(1436) \
	(1355) \
	(1311)
#elif defined(GTA_NY)
#define GAME_BUILDS \
	(43)
#else
#define GAME_BUILDS \
	(0)
#endif

namespace xbr
{
//
// Uniquifier data for the supported game builds. We have a map where we store a pair of the game name and
// major game build number as a key. The value is a pair of the minor build number and cfx-revision number.
// The major game build MUST have the same number as in the "version.txt" inside the "update.rpf" for the
// targeted game build. The revision number is used to maintain backward compatibility safety between the
// client and server when a game receives backward compatibility breaking changes in minor updates or hotfixes.
// Sometimes such title updates might not introduce any breaking changes, so forcing people to update their
// server sounds a bit too much. The revision number SHOULD always start from "0" when adding a new game build
// support. Increment this number when it's necessary to guard users from joining incompatible (not updated)
// servers. We keep track of minor game builds only for `GetCurrentGameBuildString` (i.e. for hint files).
// When there's no entry for a specific major game build, revision "0" will be assumed in the relevant code.
//

struct GameBuildUniquifier
{
	int m_minor;
	int m_revision;
};

static std::map<std::pair<std::string_view, int>, GameBuildUniquifier> kGameBuildUniquifiers
{
	// Cleanup for `GetCurrentGameBuildString`, no breaking changes.
	{ { "rdr3", 1491 }, { 50, 0 } },
};

inline const GameBuildUniquifier* GetGameBuildUniquifier(std::string_view gameName, int build)
{
	if (const auto it = kGameBuildUniquifiers.find({ gameName, build }); it != kGameBuildUniquifiers.end())
	{
		return &it->second;
	}

	return nullptr;
}

#ifdef IS_FXSERVER
inline std::pair<int, int> ParseGameBuildFromString(const std::string& buildStr)
{
	int major = 0, revision = 0;

	if (buildStr.length() > 0 && buildStr.length() < 32)
	{
		(void)sscanf(buildStr.c_str(), "%d_%d", &major, &revision);
	}

	return { major, revision };
}
#endif
}

// for CrossBuildLaunch.cpp
#ifndef XBR_BUILDS_ONLY
namespace xbr
{
int GetRequestedGameBuildInit();
int GetLatestGameBuildInit();

#ifdef IS_FXSERVER
inline int GetGameBuild()
{
	return 0;
}

inline int GetRequestedGameBuild()
{
	return 0;
}

#else

inline int GetRequestedGameBuild()
{
	static int buildNumber = -1;

	if (buildNumber == -1)
	{
		buildNumber = GetRequestedGameBuildInit();
	}

	return buildNumber;
}

inline int GetGameBuild()
{
	// For GTA5, ignore the CLI build request and use the latest build.
	// The requested build behavior will be achieved by loading different DLC sets with IsDlcIncludedInBuild.
#ifdef GTA_FIVE
	static int buildNumber = -1;

	if (buildNumber == -1)
	{
		buildNumber = GetLatestGameBuildInit();
	}

	return buildNumber;
#else
	return GetRequestedGameBuild();
#endif
}
#endif

const std::unordered_map<std::string, int> BUILD_OF_DLC = {
	{"mpBeach", 1604},
	{"mpBusiness", 1604},
	{"mpChristmas", 1604},
	{"mpValentines", 1604},
	{"mpBusiness2", 1604},
	{"mpHipster", 1604},
	{"mpIndependence", 1604},
	{"mpPilot", 1604},
	{"spUpgrade", 1604},
	{"mpLTS", 1604},
	{"mpheist", 1604},
	{"mppatchesng", 1604},
	{"patchday1ng", 1604},
	{"patchday2ng", 1604},
	{"mpchristmas2", 1604},
	{"patchday2bng", 1604},
	{"patchday3ng", 1604},
	{"patchday4ng", 1604},
	{"mpluxe", 1604},
	{"patchday5ng", 1604},
	{"mpluxe2", 1604},
	{"patchday6ng", 1604},
	{"mpreplay", 1604},
	{"patchday7ng", 1604},
	{"mplowrider", 1604},
	{"mphalloween", 1604},
	{"patchday8ng", 1604},
	{"mpapartment", 1604},
	{"mpxmas_604490", 1604},
	{"mplowrider2", 1604},
	{"mpjanuary2016", 1604},
	{"mpvalentines2", 1604},
	{"patchday9ng", 1604},
	{"mpexecutive", 1604},
	{"patchday10ng", 1604},
	{"mpstunt", 1604},
	{"patchday11ng", 1604},
	{"mpimportexport", 1604},
	{"mpbiker", 1604},
	{"patchday12ng", 1604},
	{"patchday13ng", 1604},
	{"mpspecialraces", 1604},
	{"mpgunrunning", 1604},
	{"mpairraces", 1604},
	{"mpsmuggler", 1604},
	{"mpchristmas2017", 1604},
	{"mpassault", 1604},
	{"mpbattle", 1604},
	{"patchday14ng", 1604},
	{"patchday15ng", 1604},
	{"patchday16ng", 1604},
	{"patchday17ng", 1604},
	{"patchday18ng", 1604},
	{"patchday19ng", 1604},
	{"patchday20ng", 1604},
	{"mpchristmas2018", 1604},
	{"patchday21ng", 2060},
	{"mpvinewood", 2060},
	{"patchday22ng", 2060},
	{"mpheist3", 2060},
	{"mpsum", 2060},
	{"patchday23ng", 2060},
	{"mpheist4", 2189},
	{"patchday24ng", 2189},
	{"mptuner", 2372},
	{"patchday25ng", 2372},
	{"mpsecurity", 2545},
	{"patchday26ng", 2545},
	{"mpg9ec", 2612},
	{"patchdayg9ecng", 2612},
	{"mpsum2", 2699},
	{"patchday27ng", 2699},
	{"mpsum2_g9ec", 2699},
	{"patchday27g9ecng", 2699},
	{"mpchristmas3", 2802},
	{"mpchristmas3_g9ec", 2802},
	{"patchday28ng", 2802},
	{"patchday28g9ecng", 2802},
	{"mp2023_01", 2944},
	{"patch2023_01", 2944},
	{"patch2023_01_g9ec", 2944},
	{"mp2023_01_g9ec", 2944},
	{"mp2023_02", 3095},
	{"patch2023_02", 3095},
	{"mp2023_02_g9ec", 3095},
	{"mp2024_01", 3258},
	{"patch2024_01", 3258},
	{"patch2024_01_g9ec", 3258},
	{"mp2024_01_g9ec", 3258}
};

inline bool IsDlcIncludedInBuild(const std::string& dlcName)
{
	auto dlc_build = BUILD_OF_DLC.find(dlcName);
	if (dlc_build == BUILD_OF_DLC.end())
	{
		return false;
	}

	return dlc_build->second <= xbr::GetRequestedGameBuild();
}

#ifndef IS_FXSERVER
inline std::string_view GetCurrentGameBuildString()
{
	static std::string buildString = []() -> std::string
	{
		auto build = GetGameBuild();

#if defined(IS_RDR3)
		std::string gameName = "rdr3";
#elif defined(GTA_FIVE)
		std::string gameName = "gta5";
#elif defined(GTA_NY)
		std::string gameName = "gta4";
#else
		std::string gameName = "unk";
#endif

		// Try to find if there's any uniquifier for this game build that we should use.
		if (const auto uniquifier = GetGameBuildUniquifier(gameName, build))
		{
			return fmt::sprintf("%d_%d_%d", build, uniquifier->m_minor, uniquifier->m_revision);
		}

		// Old behavior, just keeping the major game build number here.
		return fmt::sprintf("%d", build);
	}();

	return buildString;
}
#endif
}

#define EXPAND2(_, __, x) \
	inline bool BOOST_PP_CAT(Is, x)() \
	{ \
		static bool retval; \
		static bool inited = false; \
		\
		if (!inited) \
		{ \
			retval = xbr::GetGameBuild() == x; \
			inited = true; \
		} \
		\
		return retval; \
	}

BOOST_PP_SEQ_FOR_EACH(EXPAND2, , GAME_BUILDS)

#undef EXPAND2

namespace xbr
{
template<int Build>
inline bool IsGameBuildOrGreater()
{
	return GetGameBuild() >= Build;
}

template<int Build>
inline bool IsGameBuild()
{
	return GetGameBuild() == Build;
}

#ifdef _WIN32
inline const wchar_t* GetGameWndClass()
{
	return
#if defined(IS_RDR3)
	L"sgaWindow"
#elif defined(GTA_FIVE) || defined(GTA_NY)
	L"grcWindow"
#else
	L"cfxUnkClass"
#endif
	;
}
#endif
}

#ifdef _WIN32
#ifdef COMPILING_CORE
extern "C" HWND DLL_EXPORT CoreGetGameWindow();
extern "C" void DLL_EXPORT CoreSetGameWindow(HWND hWnd);
#else
inline HWND CoreGetGameWindow()
{
	static decltype(&CoreGetGameWindow) func;

	if (!func)
	{
		if (auto coreRT = GetModuleHandleW(L"CoreRT.dll"))
		{
			func = (decltype(func))GetProcAddress(coreRT, "CoreGetGameWindow");
		}
	}

	return (!func) ? false : func();
}

inline void CoreSetGameWindow(HWND hWnd)
{
	static decltype(&CoreSetGameWindow) func;

	if (!func)
	{
		if (auto coreRT = GetModuleHandleW(L"CoreRT.dll"))
		{
			func = (decltype(func))GetProcAddress(coreRT, "CoreSetGameWindow");
		}
	}

	return (!func) ? void() : func(hWnd);
}
#endif
#endif
#endif
