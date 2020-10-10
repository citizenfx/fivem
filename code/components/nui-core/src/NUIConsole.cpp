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
			{ "value", value }
			});

		nui::PostFrameMessage("mpMenu", variableJson.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace));
	}
}

void NuiConsole_SetConvars()
{
	auto vars = nlohmann::json::array();

	console::GetDefaultContext()->GetVariableManager()->ForAllVariables([&vars](const std::string& name, int flags, const ConsoleVariableManager::THandlerPtr& variable)
	{
		vars.push_back(nlohmann::json::object(
		{ { "key", name }, { "value", variable->GetValue() } }));
	});

	if (nui::HasMainUI())
	{
		nui::PostFrameMessage("mpMenu", nlohmann::json::object({ { "type", "convarsSet" },
															   { "vars", std::move(vars) } })
										.dump(-1, ' ', false, nlohmann::detail::error_handler_t::replace));
	}
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
