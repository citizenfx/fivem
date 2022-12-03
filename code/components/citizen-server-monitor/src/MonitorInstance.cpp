/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <MonitorInstance.h>
#include <ServerInstanceBaseRef.h>

#include <KeyedRateLimiter.h>

#include <console/OptionTokenizer.h>
#include <filesystem>

#include <ComponentLoader.h>

#include <CoreConsole.h>

#include <VFSManager.h>

#include <se/Security.h>

#include <RelativeDevice.h>

#include <Resource.h>
#include <ResourceManager.h>

#include <TcpListenManager.h>
#include <HttpServerManager.h>

#include <cfx_version.h>

#include <skyr/url.hpp>
#include <skyr/percent_encode.hpp>

#ifdef _WIN32
#include <MinHook.h>

static void(WINAPI* g_origExitProcess)(DWORD exitCode);

static void WINAPI ExitProcessHook(DWORD exitCode)
{
	auto consoleWindow = GetConsoleWindow();
	DWORD pid;

	GetWindowThreadProcessId(consoleWindow, &pid);
	if (pid == GetCurrentProcessId())
	{
		printf("\n\nExited. Press any key to continue.\n");
		auto _ = getc(stdin);
		(void)_;
	}

	g_origExitProcess(exitCode);
}

void InitializeExitHook()
{
	MH_Initialize();
	MH_CreateHookApi(L"kernelbase.dll", "ExitProcess", ExitProcessHook, (void**)&g_origExitProcess);
	MH_CreateHookApi(L"kernel32.dll", "ExitProcess", ExitProcessHook, NULL);
	MH_EnableHook(MH_ALL_HOOKS);
}
#else
void InitializeExitHook() {}
#endif

class LocalResourceMounter : public fx::ResourceMounter
{
public:
	LocalResourceMounter(fx::ResourceManager* manager)
		: m_manager(manager)
	{

	}

	virtual bool HandlesScheme(const std::string& scheme) override
	{
		return (scheme == "file");
	}

	virtual pplx::task<fwRefContainer<fx::Resource>> LoadResource(const std::string& uri) override
	{
		auto uriParsed = skyr::make_url(uri);

		fwRefContainer<fx::Resource> resource;

		if (uriParsed)
		{
			auto pathRef = uriParsed->pathname();
			auto fragRef = uriParsed->hash().substr(1);

			if (!pathRef.empty() && !fragRef.empty())
			{
#ifdef _WIN32
				std::string pr = pathRef.substr(1);
#else
				std::string pr = pathRef;
#endif

				resource = m_manager->CreateResource(fragRef, this);
				if (!resource->LoadFrom(*skyr::percent_decode(pr)))
				{
					m_manager->RemoveResource(resource);
					resource = nullptr;
				}
			}
		}

		return pplx::task_from_result<fwRefContainer<fx::Resource>>(resource);
	}

private:
	fx::ResourceManager* m_manager;
};

inline std::string ToNarrow(const std::string& str)
{
	return str;
}

fwEvent<fx::MonitorInstance*> OnMonitorTick;

namespace fx
{
	MonitorInstance::MonitorInstance()
		: m_shouldTerminate(false)
	{
		// create a console context
		fwRefContainer<console::Context> consoleContext;
		console::CreateContext(console::GetDefaultContext(), &consoleContext);

		SetComponent(consoleContext);
	}

	bool MonitorInstance::SetArguments(const std::string& arguments)
	{
		auto [commands, setList] = TokenizeCommandLine(arguments);

		m_arguments = std::move(commands);
		m_setList = std::move(setList);

		return true;
	}

	void MonitorInstance::Run()
	{
		InitializeExitHook();

		auto execCommand = AddCommand("exec", [=](const std::string& path) {
			fwRefContainer<vfs::Stream> stream = vfs::OpenRead(path);

			if (!stream.GetRef())
			{
				return;
			}

			std::vector<uint8_t> data = stream->ReadToEnd();
			data.push_back('\n'); // add a newline at the end

			auto consoleCtx = GetComponent<console::Context>();

			consoleCtx->AddToBuffer(std::string(reinterpret_cast<char*>(&data[0]), data.size()));
			consoleCtx->ExecuteBuffer();
		});

		trace(R"(^2  _______  ______                           
 |  ___\ \/ / ___|  ___ _ ____   _____ _ __ 
 | |_   \  /\___ \ / _ \ '__\ \ / / _ \ '__|
 |  _|  /  \ ___) |  __/ |   \ V /  __/ |   
 |_|   /_/\_\____/ \___|_|    \_/ \___|_|   
-------------------------------- ^3monitor^2 ---^7

)");

		// disable QuickEdit in *monitor mode only*
		// #TODO: detect if we're running under Windows Terminal, in which case
		//        selection isn't suspending
#ifdef _WIN32
		{
			HANDLE hConsole = GetStdHandle(STD_INPUT_HANDLE);

			DWORD consoleMode;
			if (GetConsoleMode(hConsole, &consoleMode))
			{
				consoleMode |= ENABLE_EXTENDED_FLAGS;
				consoleMode &= ~ENABLE_QUICK_EDIT_MODE;

				SetConsoleMode(hConsole, consoleMode);
			}
		}
#endif

		// initialize the server configuration
		std::shared_ptr<ConVar<std::string>> rootVar;

		auto monitorVar = AddVariable<bool>("monitorMode", ConVar_None, true);
		auto versionVar = AddVariable<std::string>("version", ConVar_None, "FXServer-" GIT_DESCRIPTION);

		{
			se::ScopedPrincipal principalScope(se::Principal{ "system.console" });

			auto consoleCtx = GetComponent<console::Context>();

			for (const auto& set : m_setList)
			{
				consoleCtx->ExecuteSingleCommandDirect(ProgramArguments{ "set", set.first, set.second });
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

			rootVar = AddVariable<std::string>("serverRoot", ConVar_None, m_rootPath);

			Initialize();

			for (const auto& bit : m_arguments)
			{
				consoleCtx->ExecuteSingleCommandDirect(bit);
			}

			// execute config
			consoleCtx->ExecuteSingleCommandDirect(ProgramArguments{ "exec", "monitor_autoexec.cfg" });
		}

		// tasks should be running in background threads; we'll just wait until someone wants to get rid of us
		while (!m_shouldTerminate)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	static std::shared_ptr<ConVar<std::string>> g_citizenDir;
	static std::shared_ptr<ConVar<std::string>> g_citizenRoot;

	void MonitorInstance::Initialize()
	{
		// initialize early components
		SetComponent(new fx::TcpListenManager("svMain"));
		SetComponent(new fx::HttpServerManager());
		SetComponent(new fx::PeerAddressRateLimiterStore(GetComponent<console::Context>().GetRef()));

		// grant Se access
		seGetCurrentContext()->AddAccessControlEntry(se::Principal{ "resource.monitor" }, se::Object{ "command" }, se::AccessType::Allow);

		// mount citizen
		g_citizenRoot = AddVariable<std::string>("citizen_root", ConVar_None, ToNarrow(MakeRelativeCitPath(L"")));
		g_citizenDir = AddVariable<std::string>("citizen_dir", ConVar_None, ToNarrow(MakeRelativeCitPath(L"citizen")));
		vfs::Mount(new vfs::RelativeDevice(g_citizenDir->GetValue() + "/"), "citizen:/");

		// initialize resource manager
		fwRefContainer<fx::ResourceManager> resourceManager = fx::CreateResourceManager();
		resourceManager->SetComponent(new fx::ServerInstanceBaseRef(this));
		resourceManager->SetComponent(GetComponent<console::Context>());

		resourceManager->AddMounter(new LocalResourceMounter(resourceManager.GetRef()));

		SetComponent(resourceManager);

		// add local resources
		auto addResource = [&](std::string_view resourceName)
		{
			std::string systemResourceRoot(g_citizenDir->GetValue() + "/system_resources/");

			skyr::url_record record;
			record.scheme = "file";

			skyr::url url{ std::move(record) };
			url.set_pathname(*skyr::percent_encode(fmt::sprintf("%s/%s", systemResourceRoot, resourceName), skyr::encode_set::path));
			url.set_hash(*skyr::percent_encode(resourceName, skyr::encode_set::fragment));

			return resourceManager->AddResource(url.href()).get();
		};

		// monitor does not use webadmin currently
		//addResource("webadmin")->Start();
		addResource("monitor")->Start();

		// setup ticks
		using namespace std::chrono_literals;

		auto loop = GetComponent<fx::TcpListenManager>()->GetTcpStack()->GetWrapLoop();
		m_tickTimer = loop->resource<uvw::TimerHandle>();

		m_tickTimer->on<uvw::TimerEvent>([this, resourceManager](const uvw::TimerEvent& ev, uvw::TimerHandle& timer)
		{
			OnMonitorTick(this);

			resourceManager->MakeCurrent();
			resourceManager->Tick();
		});

		m_tickTimer->start(0ms, 50ms);
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
			trace("citizen:server:monitor component does not implement RunnableComponent. Exiting.\n");
		}
	}
};

static ServerMain g_main;

DECLARE_INSTANCE_TYPE(ServerMain);

static InitFunction initFunction([]()
{
	Instance<ServerMain>::Set(&g_main);
});
