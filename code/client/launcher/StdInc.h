/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#ifndef COMPILING_GLUE
#include "../shared/StdInc.h"
#endif

#if !defined(COMPILING_GLUE)
struct HttpIgnoreCaseLess
{
	inline bool operator()(const std::string& left, const std::string& right) const
	{
		return _stricmp(left.c_str(), right.c_str()) < 0;
	}
};

using HttpHeaderList = std::map<std::string, std::string, HttpIgnoreCaseLess>;
using HttpHeaderListPtr = std::shared_ptr<HttpHeaderList>;
#else
#include <HttpClient.h>
#endif

int DL_RequestURL(const char* url, char* buffer, size_t bufSize, HttpHeaderListPtr responseHeaders = {});
const char* DL_RequestURLError();

// bootstrapper functions
// move the bootstrapper files if called by the initializer
bool Bootstrap_RunInit();

// run the bootstrapper/updater functions
bool Bootstrap_DoBootstrap();

// downloader functions
enum class compressionAlgo_e
{
	None,
	XZ,
	Zstd,
};

struct baseDownload
{
	int count = 1;
};

void CL_InitDownloadQueue();
std::shared_ptr<baseDownload> CL_QueueDownload(const char* url, const char* file, int64_t size, compressionAlgo_e algo);
std::shared_ptr<baseDownload> CL_QueueDownload(const char* url, const char* file, int64_t size, compressionAlgo_e algo, int segments);

bool DL_Process();

bool DL_RunLoop();

// UI functions
void UI_DoCreation(bool safeMode = false);
void UI_DoDestruction();
void UI_UpdateText(int textControl, const wchar_t* text);
void UI_UpdateProgress(double percentage);
void UI_DisplayError(const wchar_t* error);
bool UI_IsCanceled();
HWND UI_GetWindowHandle();

// updater functions
bool Updater_RunUpdate(std::initializer_list<std::string> wantedCaches);
const char* GetUpdateChannel();

#include <array>

bool CheckFileOutdatedWithUI(const wchar_t* fileName, const std::vector<std::array<uint8_t, 20>>& validHashes, uint64_t* fileStart, uint64_t fileTotal, std::array<uint8_t, 20>* foundHash = nullptr, size_t checkSize = -1);

#include "LauncherConfig.h"

#ifdef LAUNCHER_PERSONALITY_MAIN
// cppwinrt is slow, add it to pch
#include <unknwn.h>

// needed in newer cppwinrt
#define WINRT_NO_MAKE_DETECTION

#include <winrt/windows.foundation.h>
#include <winrt/windows.foundation.collections.h>
#include <winrt/windows.system.h>
#include <winrt/windows.ui.composition.core.h>
#include <winrt/windows.ui.composition.effects.h>
#include <winrt/windows.ui.xaml.hosting.h>
#include <windows.ui.xaml.hosting.desktopwindowxamlsource.h>
#include <winrt/windows.ui.xaml.controls.h>
#include <winrt/Windows.ui.xaml.media.h>
#include <winrt/Windows.Graphics.Effects.h>
#include <winrt/Windows.UI.Xaml.Markup.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#endif

#if defined(_DEBUG) && !defined(LAUNCHER_PERSONALITY_CHROME)
#define LAUNCHER_PERSONALITY_GAME
#endif

#ifdef LAUNCHER_PERSONALITY_GAME_MTL
#define LAUNCHER_PERSONALITY_GAME
#endif

// game personality aliases
#ifdef GTA_FIVE
#ifdef LAUNCHER_PERSONALITY_GAME_2060
#define LAUNCHER_PERSONALITY_GAME
#elif defined(LAUNCHER_PERSONALITY_GAME_2189)
#define LAUNCHER_PERSONALITY_GAME
#elif defined(LAUNCHER_PERSONALITY_GAME_2372)
#define LAUNCHER_PERSONALITY_GAME
#elif defined(LAUNCHER_PERSONALITY_GAME_2545)
#define LAUNCHER_PERSONALITY_GAME
#elif defined(LAUNCHER_PERSONALITY_GAME_2612)
#define LAUNCHER_PERSONALITY_GAME
#elif defined(LAUNCHER_PERSONALITY_GAME_2699)
#define LAUNCHER_PERSONALITY_GAME
#elif defined(LAUNCHER_PERSONALITY_GAME_2802)
#define LAUNCHER_PERSONALITY_GAME
#elif defined(LAUNCHER_PERSONALITY_GAME_372)
#define LAUNCHER_PERSONALITY_GAME
#elif defined(LAUNCHER_PERSONALITY_GAME_1604)
#define LAUNCHER_PERSONALITY_GAME
#endif
#elif defined(IS_RDR3)
#ifdef LAUNCHER_PERSONALITY_GAME_1311
#define LAUNCHER_PERSONALITY_GAME
#elif defined(LAUNCHER_PERSONALITY_GAME_1355)
#define LAUNCHER_PERSONALITY_GAME
#elif defined(LAUNCHER_PERSONALITY_GAME_1436)
#define LAUNCHER_PERSONALITY_GAME
#elif defined(LAUNCHER_PERSONALITY_GAME_1491)
#define LAUNCHER_PERSONALITY_GAME
#endif
#elif defined(GTA_NY)
#ifdef LAUNCHER_PERSONALITY_GAME_43
#define LAUNCHER_PERSONALITY_GAME
#endif
#endif

struct TenUIBase
{
	virtual ~TenUIBase() = default;
};

std::unique_ptr<TenUIBase> UI_InitTen();
