/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <NetLibrary.h>
#include <CefOverlay.h>
#include <ICoreGameInit.h>
#include <GameInit.h>
#include <nutsnbolts.h>

#ifdef GTA_FIVE
static InitFunction initFunction([] ()
{
	NetLibrary::OnNetLibraryCreate.Connect([] (NetLibrary* library)
	{
		library->OnInitReceived.Connect([] (NetAddress server)
		{
			//nui::SetMainUI(false);

			//nui::DestroyFrame("mpMenu");

			ICoreGameInit* gameInit = Instance<ICoreGameInit>::Get();

			if (!gameInit->GetGameLoaded())
			{
				gameInit->LoadGameFirstLaunch([] ()
				{
					// download frame code
					Sleep(1);

					return true;
				});
			}
			else
			{
				gameInit->ReloadGame();
			}
		});

		OnKillNetwork.Connect([=] (const char* message)
		{
			library->FinalizeDisconnect();
		});

		Instance<ICoreGameInit>::Get()->OnGameRequestLoad.Connect([] ()
		{
			nui::SetMainUI(false);

			nui::DestroyFrame("mpMenu");
		});
	});

	OnFirstLoadCompleted.Connect([] ()
	{
		g_gameInit.SetGameLoaded();
	});
});
#endif