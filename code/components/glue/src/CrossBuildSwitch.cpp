#include <StdInc.h>
#include <CefOverlay.h>
#include <NetLibrary.h>
#include <json.hpp>

#include <CommCtrl.h>

#include "../../client/launcher/InstallerExtraction.h"

int gameCacheTargetBuild;

extern NetLibrary* netLibrary;
extern std::map<std::string, std::string> UpdateGameCache();

extern void RestartGameToOtherBuild(int build);

static std::function<void(const std::string&)> g_submitFn;
static bool g_cancelable;
static bool g_canceled;

void PerformBuildSwitch(int build);

void InitializeBuildSwitch(int build)
{
	if (nui::HasMainUI())
	{
		g_canceled = false;
		g_cancelable = true;

		auto j = nlohmann::json::object({
			{ "build", build },
		});

		nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "connectBuildSwitchRequest", "data": %s })", j.dump()));

		g_submitFn = [build](const std::string& action)
		{
			if (action == "ok")
			{
				PerformBuildSwitch(build);
			}
		};
	}
}

void PerformBuildSwitch(int build)
{
	if (gameCacheTargetBuild != 0)
	{
		return;
	}

	gameCacheTargetBuild = build;

	std::thread([]()
	{
		// let's try to update the game cache
		if (!UpdateGameCache().empty())
		{
			RestartGameToOtherBuild(gameCacheTargetBuild);
		}

		gameCacheTargetBuild = 0;
	}).detach();
}

static HANDLE g_buttonEvent;
static bool g_buttonResponse;

void TaskDialogEmulated(TASKDIALOGCONFIG* config, int* button, void*, void*)
{
	if (config->dwCommonButtons == 0)
	{
		*button = 42;
		netLibrary->OnConnectionError(ToNarrow(config->pszContent).c_str());
	}
	else
	{
		if (nui::HasMainUI())
		{
			auto j = nlohmann::json::object({
				{ "title", ToNarrow(config->pszMainInstruction) },
				{ "content", ToNarrow(config->pszContent) },
			});

			nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "connectBuildSwitch", "data": %s })", j.dump()));
		}

		g_submitFn = [](const std::string& action)
		{
			g_buttonResponse = (action == "ok");
			SetEvent(g_buttonEvent);
		};

		g_buttonEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		WaitForSingleObject(g_buttonEvent, INFINITE);

		*button = g_buttonResponse ? IDYES : IDNO;
	}
}

bool XBR_InterceptCardResponse(const nlohmann::json& j)
{
	if (g_submitFn)
	{
		auto jcp = j;

		g_submitFn(jcp["data"]["action"]);
		g_submitFn = {};

		return true;
	}

	return false;
}

bool XBR_InterceptCancelDefer()
{
	if (g_cancelable)
	{
		g_canceled = true;
		g_cancelable = false;
		return true;
	}

	return false;
}

HWND UI_GetWindowHandle()
{
	// #TODORDR
	return FindWindow(L"grcWindow", NULL);
}

bool UI_IsCanceled()
{
	return g_canceled;
}

bool ExtractInstallerFile(const std::wstring& installerFile, const std::function<void(const InstallerInterface&)>& fileFindCallback)
{
	return false;
}

void UI_DoCreation(bool)
{

}

void UI_DoDestruction()
{

}

static std::string g_topText;
static std::string g_bottomText;
static double g_percentage;

static void UpdateProgressUX()
{
	auto text = fmt::sprintf("%s (%.0f%s)\n%s", g_topText, round(g_percentage), "%", g_bottomText);

	netLibrary->OnConnectionProgress(text, 133, 133);
}

void UI_UpdateProgress(double percentage)
{
	g_percentage = percentage;

	UpdateProgressUX();
}

void UI_UpdateText(int textControl, const wchar_t* text)
{
	(textControl == 0 ? g_topText : g_bottomText) = ToNarrow(text);

	UpdateProgressUX();
}
