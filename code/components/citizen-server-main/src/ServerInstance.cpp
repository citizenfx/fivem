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

#include <se/Security.h>

#ifdef _WIN32
#include <mmsystem.h>
#endif

// a list of variables that *need* to be used with +set (to automatically pre-parse from a config script)
static std::set<std::string, console::IgnoreCaseLess> setList =
{
	"onesync",
	"onesync_enabled",
	"onesync_population",
	"netlib",
	"onesync_enableInfinity",
	"onesync_enableBeyond",
	"gamename",
	"sv_enforceGameBuild",
	"sv_replaceExeToSwitchBuilds",
	"sv_licenseKey",
	"resources_useSystemChat",
};

namespace fx
{
	ServerInstance::ServerInstance()
		: m_shouldTerminate(false)
	{
		// increase timer resolution on Windows
#ifdef _WIN32
		timeBeginPeriod(1);
#endif

		// create a console context
		fwRefContainer<console::Context> consoleContext;
		console::CreateContext(console::GetDefaultContext(), &consoleContext);

		SetComponent(consoleContext);

		m_execCommand = AddCommand("exec", [=](const std::string& path) {
			fwRefContainer<vfs::Stream> stream = vfs::OpenRead(path);

			if (!stream.GetRef())
			{
				console::Printf("cmd", "No such config file: %s\n", path.c_str());
				return;
			}

			std::vector<uint8_t> data = stream->ReadToEnd();
			data.push_back('\n'); // add a newline at the end

			auto consoleCtx = GetComponent<console::Context>();

			consoleCtx->AddToBuffer(std::string(reinterpret_cast<char*>(&data[0]), data.size()));
			consoleCtx->ExecuteBuffer();
		});

		auto quit = [this](const std::string& reason)
		{
			if (!reason.empty())
			{
				trace("-> Quitting: %s\n", reason);
				OnRequestQuit(reason);
			}
			else
			{
				OnRequestQuit("Quit command executed.");
			}

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
		// initialize the server configuration
		{
			se::ScopedPrincipal principalScope(se::Principal{ "system.console" });

			auto optionParser = GetComponent<OptionParser>();
			auto consoleCtx = GetComponent<console::Context>();

			for (const auto& set : optionParser->GetSetList())
			{
				// save this in the default context so V8ScriptRuntime can read this
				if (set.first == "txAdminServerMode" || set.first == "gamename")
				{
					console::GetDefaultContext()->ExecuteSingleCommandDirect(ProgramArguments{ "set", set.first, set.second });
				}

				consoleCtx->ExecuteSingleCommandDirect(ProgramArguments{ "set", set.first, set.second });
			}

			// run early exec so we can set convars
			{
				fwRefContainer<console::Context> execContext;
				console::CreateContext(nullptr, &execContext);

				auto consoleCtxRef = consoleCtx.GetRef();

				auto forwardArgs = [consoleCtxRef](const std::string& cmd, const ProgramArguments& args)
				{
					std::vector<std::string> argList(args.Count() + 1);
					argList[0] = cmd;

					int i = 1;
					for (const auto& arg : args.GetArguments())
					{
						argList[i] = arg;
						i++;
					}

					consoleCtxRef->ExecuteSingleCommandDirect(ProgramArguments{ argList });
					console::GetDefaultContext()->ExecuteSingleCommandDirect(ProgramArguments{ argList });
				};

				execContext->GetCommandManager()->FallbackEvent.Connect([consoleCtxRef, forwardArgs](const std::string& cmd, const ProgramArguments& args, const std::any& context)
				{
					if (consoleCtxRef->GetVariableManager()->FindEntryRaw(cmd))
					{
						forwardArgs(cmd, args);
					}
					else if (setList.find(cmd) != setList.end() && args.Count() >= 1)
					{
						forwardArgs("set", ProgramArguments{ cmd, args.Get(0) });
					}

					return false;
				});

				std::queue<std::string> execList;

				ConsoleCommand fakeExecCommand(execContext.GetRef(), "exec", [&execList](const std::string& path)
				{
					execList.push(path);
				});

				for (const auto& bit : optionParser->GetArguments())
				{
					if (bit.Count() > 0)
					{
						execContext->ExecuteSingleCommandDirect(bit);
					}
				}

				while (!execList.empty())
				{
					auto e = execList.front();
					execList.pop();

					fwRefContainer<vfs::Stream> stream = vfs::OpenRead(e);

					if (!stream.GetRef())
					{
						if (e[0] != '@')
						{
							console::Printf("cmd", "No such config file: %s\n", e);
						}

						continue;
					}

					std::vector<uint8_t> data = stream->ReadToEnd();
					data.push_back('\n'); // add a newline at the end

					execContext->AddToBuffer(std::string(reinterpret_cast<char*>(&data[0]), data.size()));
					execContext->ExecuteBuffer();
				}

				execContext->GetVariableManager()->ForAllVariables([forwardArgs](const std::string& name, int flags, const std::shared_ptr<internal::ConsoleVariableEntryBase>& var)
				{
					if (!(flags & ConVar_ServerInfo))
					{
						forwardArgs("set", ProgramArguments{ name, var->GetValue() });
					}
				});
			}

			std::filesystem::path rootPath;
			try
			{
				rootPath = std::filesystem::canonical(".");

				m_rootPath = rootPath.u8string();
			}
			catch (std::exception&)
			{
			}

			// invoke target events
			OnServerCreate(this);

			Instance<net::UvLoopManager>::Get()->GetOrCreate("svMain")->EnqueueCallback([this, consoleCtx, optionParser]()
			{
				se::ScopedPrincipal principalScope(se::Principal{ "system.console" });

				// start standard resources
				if (console::GetDefaultContext()->GetVariableManager()->FindEntryRaw("txAdminServerMode"))
				{
					consoleCtx->ExecuteSingleCommandDirect(ProgramArguments{ "start", "monitor" });
				}

				// default forwarded commands to no-print
				consoleCtx->ExecuteSingleCommandDirect(ProgramArguments{ "con_addChannelFilter", "forward:*/*", "noprint" });

				// add system console access
				seGetCurrentContext()->AddAccessControlEntry(se::Principal{ "system.console" }, se::Object{ "webadmin" }, se::AccessType::Allow);
				seGetCurrentContext()->AddAccessControlEntry(se::Principal{ "resource.monitor" }, se::Object{ "command.quit" }, se::AccessType::Allow);

				consoleCtx->GetVariableManager()->ShouldSuppressReadOnlyWarning(true);

				for (const auto& bit : optionParser->GetArguments())
				{
					consoleCtx->ExecuteSingleCommandDirect(bit);
				}

				consoleCtx->GetVariableManager()->ShouldSuppressReadOnlyWarning(false);

				OnInitialConfiguration();
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
			trace("citizen:server:main component does not implement RunnableComponent. Exiting.\n");
		}
	}
};

static ServerMain g_main;

DECLARE_INSTANCE_TYPE(ServerMain);

static InitFunction initFunction([]()
{
	Instance<ServerMain>::Set(&g_main);
});
