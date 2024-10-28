#include <StdInc.h>
#include <CefOverlay.h>
#include <NetLibrary.h>
#include <Utils.h>
#include <json.hpp>

#include <CrossBuildRuntime.h>
#include <PureModeState.h>
#include <PoolSizesState.h>

#include <CommCtrl.h>

#include "../../client/launcher/InstallerExtraction.h"

int gameCacheTargetBuild;
bool gameCacheReplaceExecutable;

extern NetLibrary* netLibrary;
extern std::map<std::string, std::string> UpdateGameCache();

extern void RestartGameToOtherBuild(int build, int pureLevel, std::wstring poolSizesIncreaseSetting, bool replaceExecutable);

static std::function<void(const std::string&)> g_submitFn;
static bool g_cancelable;
static bool g_canceled;
static bool g_hadError;

void PerformStateSwitch(int build, int pureLevel, std::wstring poolSizesIncreaseSetting, bool replaceExecutable);

void InitializeBuildSwitch(int build, int pureLevel, std::wstring poolSizesIncreaseSetting, bool replaceExecutable)
{
	if (nui::HasMainUI())
	{
		g_canceled = false;
		g_cancelable = true;
		g_hadError = false;

		std::string currentPoolSizesIncreaseSetting = "";
		if (!fx::PoolSizeManager::GetIncreaseRequest().empty())
		{
			currentPoolSizesIncreaseSetting = nlohmann::json(fx::PoolSizeManager::GetIncreaseRequest()).dump();
		}

		auto j = nlohmann::json::object({
			{ "build", build },
			{ "pureLevel", pureLevel },
			{ "poolSizesIncrease", ToNarrow(poolSizesIncreaseSetting) },
			{ "replaceExecutable", replaceExecutable },
			{ "currentBuild", xbr::GetRequestedGameBuild() },
			{ "currentPureLevel", fx::client::GetPureLevel() },
			{ "currentPoolSizesIncrease", std::move(currentPoolSizesIncreaseSetting) },
			{ "currentReplaceExecutable", xbr::GetReplaceExecutable() }
		});

		nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "connectBuildSwitchRequest", "data": %s })", j.dump()));

		g_submitFn = [build, pureLevel, poolSizesIncreaseSetting = std::move(poolSizesIncreaseSetting), replaceExecutable](const std::string& action)
		{
			if (action == "ok")
			{
				PerformStateSwitch(build, pureLevel, std::move(poolSizesIncreaseSetting), replaceExecutable);
			}
		};
	}
}

void PerformStateSwitch(int build, int pureLevel, std::wstring poolSizesIncreaseSetting, bool replaceExecutable)
{
	if (gameCacheTargetBuild != 0)
	{
		return;
	}

	gameCacheTargetBuild = build;
	gameCacheReplaceExecutable = replaceExecutable;

	std::thread([pureLevel, poolSizesIncreaseSetting = std::move(poolSizesIncreaseSetting), replaceExecutable]()
	{
		// let's try to update the game cache
		if (!UpdateGameCache().empty())
		{
			RestartGameToOtherBuild(gameCacheTargetBuild, pureLevel, std::move(poolSizesIncreaseSetting), replaceExecutable);
		}
		// display a generic error if we failed
		else if (!g_hadError && !g_canceled)
		{
			netLibrary->OnConnectionError("Changing game build failed: An unknown error occurred");
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

		g_hadError = true;
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
		if (g_submitFn)
		{
			g_submitFn("cancel");
			g_submitFn = {};
		}

		g_canceled = true;
		g_cancelable = false;
		return true;
	}

	return false;
}

HWND UI_GetWindowHandle()
{
	return CoreGetGameWindow();
}

bool UI_IsCanceled()
{
	return g_canceled;
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
	// don't submit more progress when we're canceled
	if (g_canceled)
	{
		return;
	}

	auto text = fmt::sprintf("%s (%.0f%s)\n%s", g_topText, round(g_percentage), "%", g_bottomText);

	netLibrary->OnConnectionProgress(text, 0, 100, true);
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

void UI_DisplayError(const wchar_t* error)
{
	trace("Game cache error: %s\n", ToNarrow(error));

	g_hadError = true;
	netLibrary->OnConnectionError(fmt::sprintf("Changing game build failed: %s", ToNarrow(error)).c_str());
}

void UI_SetSnailState(bool)
{

}
