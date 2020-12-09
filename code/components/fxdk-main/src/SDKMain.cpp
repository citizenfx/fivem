#include "StdInc.h"
#include <ComponentLoader.h>


#ifdef _WIN32
#include <mmsystem.h>
#include <ShlObj.h>
#endif

#pragma comment(lib, "d3d11.lib")



#include <ResourceManager.h>
#include <ResourceEventComponent.h>

#include <UvLoopManager.h>
#include <UvTcpServer.h>

#include <skyr/url.hpp>
#include <skyr/percent_encode.hpp>

#include <CfxSubProcess.h>

#include <Manager.h>
#include <RelativeDevice.h>

#include <ReverseGameData.h>

#include <SDK.h>
#include <console/OptionTokenizer.h>

static std::function<ipc::Endpoint&()> proxyLauncherTalk;

namespace fxdk
{
ipc::Endpoint& GetLauncherTalk()
{
	return proxyLauncherTalk();
}
}


namespace fx
{
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
				std::string pr = pathRef.substr(1);

				resource = m_manager->CreateResource(fragRef, this);
				resource->LoadFrom(*skyr::percent_decode(pr));
			}
		}

		return pplx::task_from_result<fwRefContainer<fx::Resource>>(resource);
	}

private:
	fx::ResourceManager* m_manager;
};

void MakeBrowser(const std::string& url)
{
	CefRefPtr<SDKCefClient> handler(new SDKCefClient());

	// Specify CEF browser settings here.
	CefBrowserSettings browser_settings;

	// Create the BrowserView.
	CefRefPtr<CefBrowserView> browser_view = CefBrowserView::CreateBrowserView(
	handler, url, browser_settings, {}, NULL, new SubViewDelegate());

	// Create the Window. It will show itself after creation.
	CefWindow::CreateTopLevelWindow(new SimpleWindowDelegate(browser_view));
}

static bool terminateRenderThread = false;

static void RenderThread()
{
	fxdk::InitRender();

	while (!terminateRenderThread)
	{
		fxdk::Render();
	}

	trace(__FUNCTION__ ": Shutting down render thread.\n");
}

void SdkMain()
{
	// increase timer resolution on Windows
#ifdef _WIN32
	timeBeginPeriod(1);
#endif
	ConVar<std::string> sdkUrlVar("sdk_url", ConVar_None, "http://localhost:35419/");
	ConVar<std::string> sdkRootPath("sdk_root_path", ConVar_None, "built-in");
	ConVar<std::string> citizenPath("citizen_path", ConVar_None, ToNarrow(MakeRelativeCitPath(L"citizen/")));

	SetEnvironmentVariable(L"CitizenFX_ToolMode", nullptr);

	ipc::Endpoint launcherTalk("launcherTalk", true);

	Instance<ICoreGameInit>::Set(new SDKInit());

	Instance<vfs::Manager>::Set(new vfs::ManagerServer());
	vfs::Mount(new vfs::RelativeDevice(ToNarrow(MakeRelativeCitPath(L"citizen/"))), "citizen:/");

	// initialize resource manager
	fwRefContainer<ResourceManager> resman = CreateResourceManager();
	Instance<ResourceManager>::Set(resman.GetRef());

	resman->MakeCurrent();

	launcherTalk.Bind("hi", [resman]()
	{
		resman->GetComponent<fx::ResourceEventManagerComponent>()->QueueEvent2("sdk:gameLaunched", {});
	});
	launcherTalk.Bind("sdk:message", [](const std::string& message)
	{
		auto jsc = fmt::sprintf("window.postMessage(%s, '*')", message);

		auto instance = SDKCefClient::GetInstance();
		if (instance == nullptr)
		{
			return;
		}

		auto browser = instance->GetBrowser();
		if (browser == nullptr)
		{
			return;
		}

		browser->GetMainFrame()->ExecuteJavaScript(jsc, "fxdk://sdk-message", 0);
	});

	proxyLauncherTalk = [&launcherTalk]() -> ipc::Endpoint&
	{
		return launcherTalk;
	};

	HANDLE gameProcessHandle;

	resman->GetComponent<fx::ResourceEventManagerComponent>()->OnTriggerEvent.Connect([&gameProcessHandle] (const std::string& eventName, const std::string& eventPayload, const std::string& eventSource, bool* eventCanceled)
	{
		// is this our event?
		if (eventName == "sdk:openBrowser")
		{
			// deserialize the arguments
			msgpack::unpacked msg;
			msgpack::unpack(msg, eventPayload.c_str(), eventPayload.size());

			msgpack::object obj = msg.get();
			std::string url = obj.as<std::vector<std::string>>()[0];

			CefPostTask(TID_UI, base::Bind(&MakeBrowser, url));
		}
		else if (eventName == "sdk:startGame")
		{
			static HostSharedData<CfxState> hostData("CfxInitState");
			hostData->isReverseGame = true;

			static HostSharedData<ReverseGameData> rgd("CfxReverseGameData");

			HANDLE nestHandles[] = { rgd->inputMutex, rgd->consumeSema, rgd->produceSema };

			// as we start at loading screen, limit to 60fps by default
			rgd->fpsLimit = 60;

			// prepare initial structures
			STARTUPINFOEX startupInfo = { 0 };
			startupInfo.StartupInfo.cb = sizeof(STARTUPINFOEX);

			SIZE_T size = 0;
			InitializeProcThreadAttributeList(NULL, 1, 0, &size);

			std::vector<uint8_t> attListData(size);
			auto attList = (LPPROC_THREAD_ATTRIBUTE_LIST)attListData.data();

			assert(attList);

			InitializeProcThreadAttributeList(attList, 1, 0, &size);
			UpdateProcThreadAttribute(attList, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, &nestHandles, std::size(nestHandles) * sizeof(HANDLE), NULL, NULL);

			startupInfo.lpAttributeList = attList;

			PROCESS_INFORMATION processInfo = { 0 };

			auto processName = MakeCfxSubProcess(L"GameRuntime.exe", L"game");

			BOOL result = CreateProcessW(processName, const_cast<wchar_t*>(va(L"\"%s\" -dkguest -windowed", processName)), nullptr, nullptr, TRUE, CREATE_SUSPENDED | EXTENDED_STARTUPINFO_PRESENT, nullptr, nullptr, &startupInfo.StartupInfo, &processInfo);

			if (result)
			{
				gameProcessHandle = processInfo.hProcess;

				// set the PID and create the game thread
				hostData->gamePid = processInfo.dwProcessId;
				ResumeThread(processInfo.hThread);
			}
		}
	});

	// Provide CEF with command-line arguments.
	CefMainArgs main_args(GetModuleHandle(nullptr));

	// Specify CEF global settings here.
	CefSettings settings;
	settings.no_sandbox = true;

	settings.remote_debugging_port = 13173;
	settings.log_severity = LOGSEVERITY_DEFAULT;

	CefString(&settings.log_file).FromWString(MakeRelativeCitPath(L"cef.log"));

	CefString(&settings.browser_subprocess_path).FromWString(MakeCfxSubProcess(L"SDKBrowser"));

	CefString(&settings.locale).FromASCII("en-US");

	std::wstring resPath = MakeRelativeCitPath(L"bin/cef/");

	std::wstring cachePath = MakeRelativeCitPath(L"cache\\browser\\");
	CreateDirectory(cachePath.c_str(), nullptr);

	CefString(&settings.resources_dir_path).FromWString(resPath);
	CefString(&settings.locales_dir_path).FromWString(resPath);
	CefString(&settings.cache_path).FromWString(cachePath);

	// SDKCefApp implements application-level callbacks for the browser process.
	// It will create the first browser instance in OnContextInitialized() after
	// CEF has initialized.
	CefRefPtr<SDKCefApp> app(new SDKCefApp);

	trace(__FUNCTION__ ": Initializing CEF.\n");

	// Initialize CEF.
	CefInitialize(main_args, settings, app.get(), nullptr);

	trace(__FUNCTION__ ": Initialized CEF.\n");

	std::mutex renderThreadMutex;

	std::thread([&renderThreadMutex]()
	{
		renderThreadMutex.lock();
		RenderThread();
		renderThreadMutex.unlock();
	})
	.detach();

	// setup uv loop
	auto loop = Instance<net::UvLoopManager>::Get()->GetOrCreate("svMain");

	loop->EnqueueCallback([resman, &launcherTalk, loop, &sdkRootPath]()
	{
		resman->AddMounter(new LocalResourceMounter(resman.GetRef()));

		skyr::url_record record;
		record.scheme = "file";

		skyr::url url{ std::move(record) };
		auto sdkRootPathValue = sdkRootPath.GetValue();

		if (sdkRootPathValue == "built-in")
		{
			url.set_pathname(*skyr::percent_encode(ToNarrow(MakeRelativeCitPath(L"citizen/sdk/sdk-root/")), skyr::encode_set::path));
		}
		else
		{
			WCHAR fullSdkRootPath[MAX_PATH];
			GetFullPathName(ToWide(sdkRootPathValue).c_str(), MAX_PATH, fullSdkRootPath, NULL);

			url.set_pathname(*skyr::percent_encode(ToNarrow(fullSdkRootPath), skyr::encode_set::path));
		}

		
		url.set_hash(*skyr::percent_encode("sdk-root", skyr::encode_set::fragment));

		trace("USING SDK_ROOT PATH: %s", url.href());

		resman->AddResource(url.href())
		.then([loop](fwRefContainer<fx::Resource> resource)
		{
			loop->EnqueueCallback([resource]()
			{
				resource->Start();
			});
		});

		auto frameTime = 1000 / 30;

		static uv_timer_t tickTimer;

		uv_timer_init(loop->GetLoop(), &tickTimer);
		uv_timer_start(&tickTimer, UvPersistentCallback(&tickTimer, [resman, &launcherTalk](uv_timer_t*)
								   {
									   resman->Tick();

									   launcherTalk.RunFrame();
								   }),
		frameTime, frameTime);
	});

	// Run the CEF message loop. This will block until CefQuitMessageLoop() is
	// called.
	CefRunMessageLoop();

	terminateRenderThread = true;
	renderThreadMutex.lock();
	renderThreadMutex.unlock();

	trace(__FUNCTION__ ": Shutting down game.\n");

	if (gameProcessHandle)
	{
		TerminateProcess(gameProcessHandle, 0);
		WaitForSingleObject(gameProcessHandle, INFINITE);
	}

	trace(__FUNCTION__ ": Shut down game.\n");

	trace(__FUNCTION__ ": Shutting down CEF.\n");

	// Shut down CEF.
	CefShutdown();

	trace(__FUNCTION__ ": Shut down CEF.\n");
}
}

class SDKMain
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

static SDKMain g_main;
static std::vector<ProgramArguments> g_argumentList;

DECLARE_INSTANCE_TYPE(SDKMain);

static InitFunction initFunction([]()
{
	// initialize console arguments
	std::vector<std::pair<std::string, std::string>> setList;

	auto commandLine = GetCommandLineW();

	{
		wchar_t* s = commandLine;

		if (*s == L'"')
		{
			++s;
			while (*s)
			{
				if (*s++ == L'"')
				{
					break;
				}
			}
		}
		else
		{
			while (*s && *s != L' ' && *s != L'\t')
			{
				++s;
			}
		}

		while (*s == L' ' || *s == L'\t')
		{
			s++;
		}

		try
		{
			std::tie(g_argumentList, setList) = TokenizeCommandLine(ToNarrow(s));
		}
		catch (std::runtime_error& e)
		{
			trace("couldn't parse command line: %s\n", e.what());
		}
	}

	se::ScopedPrincipal principalScope(se::Principal{ "system.console" });

	for (const auto& set : setList)
	{
		console::GetDefaultContext()->ExecuteSingleCommandDirect(ProgramArguments{ "set", set.first, set.second });
	}

	Instance<SDKMain>::Set(&g_main);
});
