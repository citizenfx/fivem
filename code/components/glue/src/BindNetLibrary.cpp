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

#include <DrawCommands.h>
#include <FontRenderer.h>

#include <msgpack.hpp>

#include <CoreConsole.h>
#include <se/Security.h>

static InitFunction initFunction([] ()
{
	seGetCurrentContext()->AddAccessControlEntry(se::Principal{ "system.internal" }, se::Object{ "builtin" }, se::AccessType::Allow);

	NetLibrary::OnNetLibraryCreate.Connect([] (NetLibrary* library)
	{
		static NetLibrary* netLibrary = library;
		static std::string netLibWarningMessage;
		static std::mutex netLibWarningMessageLock;

		library->OnConnectOKReceived.Connect([](NetAddress)
		{
			std::unique_lock<std::mutex> lock(netLibWarningMessageLock);
			netLibWarningMessage = "";
		});

		library->OnReconnectProgress.Connect([](const std::string& msg)
		{
			std::unique_lock<std::mutex> lock(netLibWarningMessageLock);
			netLibWarningMessage = msg;
		});

		OnPostFrontendRender.Connect([]()
		{
			if (!netLibWarningMessage.empty())
			{
				std::unique_lock<std::mutex> lock(netLibWarningMessageLock);
				TheFonts->DrawText(ToWide(netLibWarningMessage), CRect(40.0f, 40.0f, 800.0f, 500.0f), CRGBA(255, 0, 0, 255), 40.0f, 1.0f, "Comic Sans MS");
			}
		});

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
			{
				std::unique_lock<std::mutex> lock(netLibWarningMessageLock);
				netLibWarningMessage = "";
			}

			library->Disconnect(message);

			Instance<ICoreGameInit>::Get()->ClearVariable("storyMode");
			Instance<ICoreGameInit>::Get()->ClearVariable("localMode");
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
