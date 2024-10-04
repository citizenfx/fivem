/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ServerInstance.h>

#include <OptionParser.h>

#include <filesystem>

#include <ComponentLoader.h>

#include <CoreConsole.h>

#include <UvLoopManager.h>
#include <VFSManager.h>
#include <ResourceManager.h>

#include <se/Security.h>

#include "SdkIpc.h"

#ifdef _WIN32
#include <mmsystem.h>
#endif

namespace fx
{
	ServerInstance::ServerInstance()
		: m_shouldTerminate(false)
	{
		// increase timer resolution on Windows
#ifdef _WIN32
		timeBeginPeriod(1);
#endif

		std::filesystem::path rootPath;
		try
		{
			rootPath = std::filesystem::canonical(".");

			m_rootPath = rootPath.u8string();
		}
		catch (std::exception&)
		{
		}

		// create a console context
		fwRefContainer<console::Context> consoleContext;
		console::CreateContext(console::GetDefaultContext(), &consoleContext);

		SetComponent(consoleContext);

		auto quit = [this](const std::string& reason)
		{
			OnRequestQuit(reason);

			m_shouldTerminate = true;
		};

		m_quitCommand_0 = AddCommand("quit", [quit]()
		{
			quit("Quit command executed.");
		});

		m_quitCommand_1 = AddCommand("quit", [quit](const std::string& reason)
		{
			quit(reason);
		});

		SetComponent(new fx::OptionParser());
	}

	bool ServerInstance::SetArguments(const std::string& arguments)
	{
		auto optionParser = GetComponent<OptionParser>();
		
		return optionParser->ParseArgumentString(arguments);
	}

	void ServerInstance::Run()
	{
		trace("Running in FxDK mode\n");

		// initialize the server configuration
		{
			se::ScopedPrincipal principalScope(se::Principal{ "system.console" });

			auto optionParser = GetComponent<OptionParser>();
			auto consoleCtx = GetComponent<console::Context>();

			// set this early so svgui won't pop up
			consoleCtx->ExecuteSingleCommandDirect(ProgramArguments{ "set", "svgui_disable", "1" });
			consoleCtx->ExecuteSingleCommandDirect(ProgramArguments{ "set", "sv_fxdkMode", "1" });
			consoleCtx->ExecuteSingleCommandDirect(ProgramArguments{ "set", "sv_fxdkServerMode", "1" });

			// it's fine to have this scoped as it holds multiple references to itself internally
			fwRefContainer<fxdk::SdkIpc> sdkIpc = new fxdk::SdkIpc(this, optionParser->GetPipeAppendix());

			// wait for fxdk ipc to init
			sdkIpc->WaitForInitialized();

			// terminate early if needed
			if (m_shouldTerminate)
			{
				return;
			}

			// initial fxdk configuration
			consoleCtx->GetVariableManager()->ShouldSuppressReadOnlyWarning(true);
			consoleCtx->ExecuteSingleCommandDirect(ProgramArguments{ "set", "sv_lan", "1" });
			consoleCtx->ExecuteSingleCommandDirect(ProgramArguments{ "set", "onesync", "on" });
			consoleCtx->ExecuteSingleCommandDirect(ProgramArguments{ "set", "sv_maxclients", "2" });
			consoleCtx->GetVariableManager()->ShouldSuppressReadOnlyWarning(false);

			// invoke target events
			OnServerCreate(this);

			// add endpoints
			consoleCtx->ExecuteSingleCommandDirect(ProgramArguments{ "endpoint_add_tcp", "127.0.0.1:30120" });
			consoleCtx->ExecuteSingleCommandDirect(ProgramArguments{ "endpoint_add_udp", "127.0.0.1:30120" });

			// grant sdk-game resource cmd access
			seGetCurrentContext()->AddAccessControlEntry(se::Principal{ "resource.sdk-game" }, se::Object{ "command" }, se::AccessType::Allow);

			// let fxdk host know that it can resume init sequence
			sdkIpc->NotifyStarted();

			Instance<net::UvLoopManager>::Get()->GetOrCreate("svMain")->EnqueueCallback([this, sdkIpc]()
			{
				OnInitialConfiguration();

				// finally, fxdk can safely connect client to server
				sdkIpc->NotifyReady();
			});
		}

		// tasks should be running in background threads; we'll just wait until someone wants to get rid of us
		while (!m_shouldTerminate)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
}

class ServerMain
{
	virtual void Run(fwRefContainer<Component> component)
	{
		// run the server's main routine
		fwRefContainer<RunnableComponent> runnableServer = dynamic_cast<RunnableComponent*>(component.GetRef());

		if (runnableServer.GetRef() != nullptr)
		{
			runnableServer->Run();
		}
		else
		{
			trace("citizen:server:fxdk component does not implement RunnableComponent. Exiting.\n");
		}
	}
};

static ServerMain g_main;

DECLARE_INSTANCE_TYPE(ServerMain);

static InitFunction initFunction([]()
{
	Instance<ServerMain>::Set(&g_main);
});
