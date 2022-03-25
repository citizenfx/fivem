#include <StdInc.h>
#include <CefOverlay.h>
#include <json.hpp>

static std::function<void(const std::string&)> g_submitFn;

static bool g_cancelable;
static bool g_hadError;

// Someone should clean this up later, since this is a bit of a hack ( and may cause some bugs ) - we reference CrossBuildSwitch.cpp variable here.
extern bool g_canceled;

extern void RestartGameToSwitchShMode(bool shAllowed);

void InitializeShModeSwitch(bool shAllowed)
{

	if (nui::HasMainUI())
	{
		auto j = nlohmann::json::object({
		{ "enable", shAllowed },
		});

		nui::PostFrameMessage("mpMenu", fmt::sprintf(R"({ "type": "connectShModeSwitchRequest", "data": %s })", j.dump()));

		g_submitFn = [shAllowed](const std::string& action)
		{
			if (action == "ok")
			{
				RestartGameToSwitchShMode(shAllowed);
			}
		};
	}
}

bool SHMR_InterceptCardResponse(const nlohmann::json& j)
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

bool SHMR_InterceptCancelDefer()
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
