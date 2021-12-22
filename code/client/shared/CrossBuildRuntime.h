#pragma once

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/wstringize.hpp>

#ifdef GTA_FIVE
#define GAME_BUILDS \
	(2545) \
	(2372) \
	(2189) \
	(2060) \
	(372 ) \
	(1604)
#elif defined(IS_RDR3)
#define GAME_BUILDS \
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
inline int GetGameBuild()
{
#ifndef IS_FXSERVER
	static int buildNumber = -1;

	if (buildNumber != -1)
	{
		return buildNumber;
	}

	constexpr const std::pair<std::wstring_view, int> buildNumbers[] = {
#define EXPAND(_, __, x) \
	{ BOOST_PP_WSTRINGIZE(BOOST_PP_CAT(b, x)), x },

		BOOST_PP_SEQ_FOR_EACH(EXPAND, , GAME_BUILDS)

#undef EXPAND
	};

	std::wstring_view cli = GetCommandLineW();
	buildNumber = std::get<1>(buildNumbers[std::size(buildNumbers) - 1]);

	for (auto [build, number] : buildNumbers)
	{
		if (cli.find(build) != std::string_view::npos)
		{
			buildNumber = number;
			break;
		}
	}

	return buildNumber;
#else
	return 0;
#endif
}
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
