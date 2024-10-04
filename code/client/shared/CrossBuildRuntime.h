#pragma once

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/wstringize.hpp>

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
int GetGameBuildInit();

inline int GetGameBuild()
{
#ifndef IS_FXSERVER
	static int buildNumber = -1;

	if (buildNumber == -1)
	{
		buildNumber = GetGameBuildInit();
	}

	return buildNumber;
#else
	return 0;
#endif
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

inline bool IsSupportedGameBuild(uint32_t targetBuild)
{
	switch (targetBuild)
	{
#define EXPAND(_, __, x) case x: return true;
		BOOST_PP_SEQ_FOR_EACH(EXPAND, , GAME_BUILDS)
#undef EXPAND
	}
	return false;
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
