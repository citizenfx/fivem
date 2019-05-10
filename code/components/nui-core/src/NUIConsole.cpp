#include <StdInc.h>
#include <CefOverlay.h>
#include <CoreConsole.h>

#include <json.hpp>

static void SendConvar(const std::string& name, const std::string& value)
{
	if (nui::HasMainUI())
	{
		auto variableJson = nlohmann::json::object({
			{ "type", "convarSet" },
			{ "name", name },
			{ "value", std::move(value) }
			});

		nui::PostFrameMessage("mpMenu", variableJson.dump());
	}
}

void NuiConsole_SetConvars()
{
	console::GetDefaultContext()->GetVariableManager()->ForAllVariables([](const std::string& name, int flags, const ConsoleVariableManager::THandlerPtr& variable)
	{
		SendConvar(name, variable->GetValue());
	});
}

static InitFunction initFunction([]()
{
	console::GetDefaultContext()->GetVariableManager()->OnConvarModified.Connect([](const std::string& name)
	{
		auto convar = console::GetDefaultContext()->GetVariableManager()->FindEntryRaw(name);

		if (convar)
		{
			SendConvar(name, convar->GetValue());
		}
	});
});
