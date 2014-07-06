#include "StdInc.h"
#include "CefOverlay.h"
#include "CustomLoadScreens.h"
#include "CrossLibraryInterfaces.h"
#include <GameInit.h>

static std::shared_ptr<NUIWindow> g_loadScreen;

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
		if (g_loadScreen.get())
		{
			g_loadScreen->UpdateFrame();
		}
	});

	g_hooksDLL->SetHookCallback(StringHash("loadsClean"), [] (void*)
	{
		if (g_loadScreen.get())
		{
			g_loadScreen = nullptr;
		}
	});
});