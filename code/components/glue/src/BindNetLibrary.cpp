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

#include <msgpack.hpp>

#include <CoreConsole.h>
#include <se/Security.h>

#ifdef GTA_FIVE
static InitFunction initFunction([] ()
{
	seGetCurrentContext()->AddAccessControlEntry(se::Principal{ "system.internal" }, se::Object{ "builtin" }, se::AccessType::Allow);

	NetLibrary::OnNetLibraryCreate.Connect([] (NetLibrary* library)
	{
		static NetLibrary* netLibrary = library;

		library->OnStateChanged.Connect([] (NetLibrary::ConnectionState curState, NetLibrary::ConnectionState lastState)
		{
			if (curState == NetLibrary::CS_ACTIVE)
			{
				ICoreGameInit* gameInit = Instance<ICoreGameInit>::Get();

				if (!gameInit->GetGameLoaded())
				{
					trace("Triggering LoadGameFirstLaunch()\n");

					gameInit->LoadGameFirstLaunch([]()
					{
						// download frame code
						Sleep(1);

						return netLibrary->AreDownloadsComplete();
					});
				}
				else
				{
					trace("Triggering ReloadGame()\n");

					gameInit->ReloadGame();
				}
			}
		});

		OnKillNetwork.Connect([=] (const char* message)
		{
			library->Disconnect(message);
		});

		OnKillNetworkDone.Connect([=]()
		{
			library->FinalizeDisconnect();

			console::GetDefaultContext()->GetVariableManager()->RemoveVariablesWithFlag(ConVar_Replicated);
		});

		library->AddReliableHandler("msgConVars", [](const char* buf, size_t len)
		{
			auto unpacked = msgpack::unpack(buf, len);
			auto conVars = unpacked.get().as<std::map<std::string, std::string>>();

			se::ScopedPrincipal principalScope(se::Principal{ "system.console" });
			se::ScopedPrincipal principalScopeInternal(se::Principal{ "system.internal" });

			// set variable
			for (const auto& conVar : conVars)
			{
				console::GetDefaultContext()->ExecuteSingleCommandDirect(ProgramArguments{ "setr", conVar.first, conVar.second });
			}
		}, true);
	});

	Instance<ICoreGameInit>::Get()->OnGameRequestLoad.Connect([]()
	{
		nui::SetMainUI(false);

		nui::DestroyFrame("mpMenu");
	});

	OnFirstLoadCompleted.Connect([] ()
	{
		g_gameInit.SetGameLoaded();
	});
});
#endif
