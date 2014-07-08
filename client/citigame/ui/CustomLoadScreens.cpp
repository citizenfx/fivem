#include "StdInc.h"
#include "CefOverlay.h"
#include "CustomLoadScreens.h"
#include "CrossLibraryInterfaces.h"
#include <GameInit.h>
#include <mutex>

static std::shared_ptr<NUIWindow> g_loadScreen;
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
		g_loadScreenMutex.lock();

		if (g_loadScreen.get())
		{
			g_loadScreen->UpdateFrame();
		}

		g_loadScreenMutex.unlock();
	});

	g_hooksDLL->SetHookCallback(StringHash("loadsClean"), [] (void*)
	{
		g_loadScreenMutex.lock();

		if (g_loadScreen.get())
		{
			g_loadScreen = nullptr;
		}

		g_loadScreenMutex.unlock();
	});
});