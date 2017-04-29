/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "../shared/StdInc.h"

int DL_RequestURL(const char* url, char* buffer, size_t bufSize);

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
void UI_DoCreation();
void UI_DoDestruction();
void UI_UpdateText(int textControl, const wchar_t* text);
void UI_UpdateProgress(double percentage);
bool UI_IsCanceled();
HWND UI_GetWindowHandle();

// updater functions
bool Updater_RunUpdate(int numCaches, ...);
const char* GetUpdateChannel();

bool CheckFileOutdatedWithUI(const wchar_t* fileName, const uint8_t hash[20]);

#include "LauncherConfig.h"