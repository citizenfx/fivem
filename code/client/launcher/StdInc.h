/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "../shared/StdInc.h"

int DL_RequestURL(const char* url, char* buffer, size_t bufSize);
const char* DL_RequestURLError();

// bootstrapper functions
// move the bootstrapper files if called by the initializer
bool Bootstrap_RunInit();

// run the bootstrapper/updater functions
bool Bootstrap_DoBootstrap();

// downloader functions
void CL_InitDownloadQueue();
void CL_QueueDownload(const char* url, const char* file, int64_t size, bool compressed);
void CL_QueueDownload(const char* url, const char* file, int64_t size, bool compressed, int segments);
//void CL_QueueDownload(const char* url, const char* file, int size, bool compressed, const uint8_t* hash, uint32_t hashLen);
bool DL_Process();

bool DL_RunLoop();

// UI functions
void UI_DoCreation(bool safeMode = false);
void UI_DoDestruction();
void UI_UpdateText(int textControl, const wchar_t* text);
void UI_UpdateProgress(double percentage);
bool UI_IsCanceled();
HWND UI_GetWindowHandle();

// updater functions
bool Updater_RunUpdate(int numCaches, ...);
const char* GetUpdateChannel();

#include <array>

bool CheckFileOutdatedWithUI(const wchar_t* fileName, const std::vector<std::array<uint8_t, 20>>& validHashes, uint64_t* fileStart, uint64_t fileTotal, std::array<uint8_t, 20>* foundHash = nullptr);

#include "LauncherConfig.h"

#ifdef LAUNCHER_PERSONALITY_MAIN
// cppwinrt is slow, add it to pch
#include <unknwn.h>

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

#ifdef _DEBUG
#define LAUNCHER_PERSONALITY_GAME
#endif

// game personality aliases
#ifdef GTA_FIVE
#ifdef LAUNCHER_PERSONALITY_GAME_2060
#define LAUNCHER_PERSONALITY_GAME
#elif defined(LAUNCHER_PERSONALITY_GAME_372)
#define LAUNCHER_PERSONALITY_GAME
#else
#define LAUNCHER_PERSONALITY_GAME_1604
#endif
#elif defined(IS_RDR3)
#ifdef LAUNCHER_PERSONALITY_MAIN
#define LAUNCHER_PERSONALITY_GAME
#endif
#endif

struct TenUIBase
{
	virtual ~TenUIBase() = default;
};

std::unique_ptr<TenUIBase> UI_InitTen();
