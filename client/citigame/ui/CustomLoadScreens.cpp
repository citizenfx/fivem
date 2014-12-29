/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#if 0
#ifdef GTA_NY
#include "CefOverlay.h"
#include "CustomLoadScreens.h"
#include "CrossLibraryInterfaces.h"
#include "NetLibrary.h"
#include <GameInit.h>
#include <mutex>

static fwRefContainer<NUIWindow> g_loadScreen;
static std::mutex g_loadScreenMutex;

void CustomLoadScreens::PrepareSwitchTo(std::string url)
{
	// set up our NUI window
	int resX = *(int*)0xFDCEAC;
	int resY = *(int*)0xFDCEB0;

	g_loadScreen = NUIWindow::Create(false, resX, resY, url);

	// and tell the game to prepare switching to our hooked load screen
	GameInit::PrepareSwitchToCustomLoad(g_loadScreen->GetTexture());
}

static InitFunction initFunction([] ()
{
	g_hooksDLL->SetHookCallback(StringHash("loadsFrame"), [] (void*)
	{
		g_netLibrary->RunFrame();

		/*g_loadScreenMutex.lock();

		if (g_loadScreen.get())
		{
			g_loadScreen->UpdateFrame();
		}

		g_loadScreenMutex.unlock();*/
	});

	g_hooksDLL->SetHookCallback(StringHash("loadsClean"), [] (void*)
	{
		g_loadScreenMutex.lock();

		if (g_loadScreen.GetRef())
		{
			g_loadScreen = nullptr;
		}

		g_loadScreenMutex.unlock();
	});
});
#endif
#endif