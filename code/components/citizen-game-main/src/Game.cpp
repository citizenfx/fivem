#include "StdInc.h"
#include <Game.h>

#include <CoreConsole.h>

#include <GameWindow.h>

#include <CfxState.h>
#include <CfxSubProcess.h>
#include <HostSharedData.h>
#include <ReverseGameData.h>

#include <CefOverlay.h>

#include <LauncherIPC.h>

#include <Error.h>

#include <json.hpp>

using nlohmann::json;

static HANDLE g_gp;
static std::function<void()> doNext;

namespace citizen
{
struct ModData
{
	std::string installPath;
	std::string executableName;

	std::string GetExecutablePath()
	{
		return fmt::sprintf("%s\\%s", installPath, executableName);
	}
};

void to_json(json& j, const ModData& m)
{
	j = json{
		{"installPath", m.installPath},
		{"executableName", m.executableName}
	};
}

void from_json(const json& j, ModData& m)
{
	j.at("installPath").get_to(m.installPath);
	j.at("executableName").get_to(m.executableName);
}

class GameImpl : public GameImplBase
{
public:
	GameImpl(Game* game)
		: m_ep("launcherTalk", true), m_game(game)
	{
		m_ep.Bind("connectionError", [this](const std::string& error)
		{
			json j = error;

			nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "connectFailed", "message": %s })", j.dump()));
		});

		auto cmp = [this](const std::string& message, int progress, int totalProgress)
		{
			json j = json{
				{"message", message},
				{"count", progress},
				{"total", totalProgress}
			};

			if (nui::HasMainUI())
			{
				nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "connectStatus", "data": %s })", j.dump()));
			}
		};

		m_ep.Bind("connectionProgress", [this, cmp](const std::string& message, int progress, int totalProgress)
		{
			cmp(message, progress, totalProgress);
		});

		m_ep.Bind("disconnected", [this]()
		{
			static ConVar<std::string> uiUrlVar("ui_url", ConVar_None, "https://nui-game-internal/ui/app/index.html");
			static HostSharedData<ReverseGameData> rgd("CfxReverseGameData");

			TerminateProcess(g_gp, 0);

			nui::SetMainUI(true);
			nui::CreateFrame("mpMenu", uiUrlVar.GetValue());

			rgd->inited = false;
		});

		m_ep.Bind("loading", [this]()
		{
			nui::SetMainUI(false);
			nui::DestroyFrame("mpMenu");
		});

		static std::string connIp;

		m_ep.Bind("hi", [this, cmp]()
		{
			cmp("Target game launched.", 0, 1);

			// call isn't reentrant currently
			doNext = [this]()
			{
				m_ep.Call("connectTo", connIp);

				connIp = "";
			};
		});

		nui::OnInvokeNative.Connect([this, cmp](const wchar_t* type, const wchar_t* arg)
		{
			if (!_wcsicmp(type, L"connectTo"))
			{
				std::wstring hostnameStrW = arg;
				std::string hostnameStr(hostnameStrW.begin(), hostnameStrW.end());

				connIp = hostnameStr;

				RunMod("fivem", "");

				cmp("Starting the target game.", 0, 1);
			}
			else if (!_wcsicmp(type, L"exit"))
			{
				// queue an ExitProcess on the next game frame
				doNext = []()
				{
					CefShutdown();

					TerminateProcess(GetCurrentProcess(), 0);
				};
			}
		});
	}

	ipc::Endpoint& GetIPC()
	{
		return m_ep;
	}

	void Run();

	void RunMod(const std::string& modId, const std::string& args);

private:
	std::string m_gameState;

	ipc::Endpoint m_ep;

	Game* m_game;

	std::map<std::string, ModData> m_mods;
};

void GameImpl::Run()
{
	auto fn = MakeRelativeCitPath(L"cgl-temp-games.json");

	FILE* f = _wfopen(fn.c_str(), L"rb");

	if (!f)
	{
		FatalError("%s doesn't exist. Please configure it.", ToNarrow(fn));
		return;
	}

	fseek(f, 0, SEEK_END);
	int e = ftell(f);
	fseek(f, 0, SEEK_SET);

	std::vector<char> v(e);
	fread(v.data(), 1, v.size(), f);
	fclose(f);

	m_mods = json::parse(v).get_to(m_mods);
	
	static HostSharedData<ReverseGameData> rgd("CfxReverseGameData");
	rgd->isLauncher = true;

	SECURITY_ATTRIBUTES sa = { 0 };
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;

	int surfaceLimit = 1;

	auto inputMutex = CreateMutex(&sa, FALSE, NULL);
	auto queueSema = CreateSemaphore(&sa, 0, surfaceLimit, NULL);
	auto queueSema2 = CreateSemaphore(&sa, surfaceLimit, surfaceLimit, NULL);

	rgd->inputMutex = inputMutex;
	rgd->consumeSema = queueSema;
	rgd->produceSema = queueSema2;
	rgd->surfaceLimit = surfaceLimit;
	rgd->produceIdx = 1;

	auto window = GameWindow::Create("Compositing Launcher", 1280, 720, m_game);
	auto windowRef = window.get();

	std::thread([this, windowRef]()
	{
		while (true)
		{
			m_ep.RunFrame();

			if (doNext)
			{
				doNext();
				doNext = {};
			}

			windowRef->ProcessEventsOnce();
			windowRef->Render();
		}
	}).detach();

	window->ProcessEvents();
}

void GameImpl::RunMod(const std::string& modId, const std::string& args)
{
	auto applicationName = ToWide(m_mods[modId].GetExecutablePath());

	static HostSharedData<ReverseGameData> rgd("CfxReverseGameData");
	HANDLE nestHandles[] = { rgd->inputMutex, rgd->consumeSema, rgd->produceSema };

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

	BOOL result = CreateProcess(applicationName.c_str(), (wchar_t*)va(L"\"%s\" %s", applicationName, ToWide(args)), nullptr, nullptr, TRUE, EXTENDED_STARTUPINFO_PRESENT, nullptr, nullptr, &startupInfo.StartupInfo, &processInfo);

	if (result)
	{
		static HostSharedData<CfxState> hostData("CfxInitState");
		hostData->gamePid = processInfo.dwProcessId;

		std::thread([processInfo]()
		{
			g_gp = processInfo.hProcess;

			WaitForSingleObject(processInfo.hProcess, INFINITE);
			CloseHandle(processInfo.hProcess);

			static ConVar<std::string> uiUrlVar("ui_url", ConVar_None, "https://nui-game-internal/ui/app/index.html");

			nui::SetMainUI(true);
			nui::CreateFrame("mpMenu", uiUrlVar.GetValue());

			rgd->inited = false;
		}).detach();

		CloseHandle(processInfo.hThread);
	}
}

Game::Game()
	: m_impl(new GameImpl(this))
{

}

void Game::Run()
{
	((GameImpl*)m_impl.get())->Run();
}
}
