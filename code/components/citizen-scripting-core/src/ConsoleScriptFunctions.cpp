/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include "ResourceManager.h"
#include "ScriptEngine.h"

#include <CoreConsole.h>

#if IS_FXSERVER
#include "ServerInstanceBaseRef.h"
#endif

#include <regex>
#include <boost/algorithm/string/replace.hpp>

#include "fxScripting.h"
#include "ResourceCallbackComponent.h"
#include "SharedFunction.h"
#include "Utils.h"
#include "om/core.h"

// copied from conhost-v2
static std::regex MakeRegex(const std::string& pattern)
{
	std::string re = pattern;
	re = std::regex_replace(re, std::regex{ "[.^$|()\\[\\]{}?\\\\]" }, "\\$&");

	boost::algorithm::replace_all(re, " ", "|");
	boost::algorithm::replace_all(re, "+", "|");
	boost::algorithm::replace_all(re, "*", ".*");

	return std::regex{ "^(?:" + re + ")$", std::regex::icase };
}

ConsoleVariableManager* GetVariableManager() {
#if IS_FXSERVER
	// get the current resource manager
	auto resourceManager = fx::ResourceManager::GetCurrent();

	// get the owning server instance
	auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

	// get the server's console context
	auto consoleContext = instance->GetComponent<console::Context>();

	return consoleContext->GetVariableManager();
#else
	// get the console context
	auto consoleContext = console::GetDefaultContext();

	// get the variable manager
	return consoleContext->GetVariableManager();
#endif
}

bool IsConVarScriptRestricted(ConsoleVariableManager* varMan, const std::string& varName)
{
	return varMan->GetEntryFlags(varName) & ConVar_ScriptRestricted;
}

static InitFunction initFunction([]()
{


	fx::ScriptEngine::RegisterNativeHandler("GET_CONVAR", [=](fx::ScriptContext& context)
	{
		// get variable name
		const std::string varName = context.CheckArgument<const char*>(0);
		const auto defaultValue = context.GetArgument<const char*>(1);

		const auto varMan = GetVariableManager();

		// check can it be exposed to script
		if (IsConVarScriptRestricted(varMan, varName))
		{
			// gets and returns default value
			context.SetResult(defaultValue);
			return;
		}

		// get the variable
		const auto var = varMan->FindEntryRaw(varName);

		if (!var)
		{
			// gets and returns default value
			context.SetResult(defaultValue);
			return;
		}

		static std::string varVal;
		varVal = var->GetValue();

		context.SetResult(varVal.c_str());
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_CONVAR_INT", [=](fx::ScriptContext& context)
	{
		const std::string varName = context.CheckArgument<const char*>(0);
		const int defaultValue = context.GetArgument<int>(1);

		// get the variable manager
		const auto varMan = GetVariableManager();

		// check can it be exposed to script
		if (IsConVarScriptRestricted(varMan, varName))
		{
			context.SetResult(defaultValue);
			return;
		}

		// get the variable
		const auto var = varMan->FindEntryRaw(varName);
		if (!var)
		{
			context.SetResult(defaultValue);
			return;
		}

		auto value = var->GetValue();
		if (value == "true")
		{
			value = "1";
		}

		context.SetResult(atoi(value.c_str()));
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_CONVAR_FLOAT", [=](fx::ScriptContext& context)
	{
		const std::string varName = context.CheckArgument<const char*>(0);
		const float defaultValue = context.GetArgument<float>(1);

		// get the variable manager
		const auto varMan = GetVariableManager();

		// get the variable
		const auto var = varMan->FindEntryRaw(varName);

		// check can it be exposed to script
		if (IsConVarScriptRestricted(varMan, varName))
		{
			// gets and returns default value
			context.SetResult(defaultValue);
			return;
		}

		if (!var)
		{
			context.SetResult<float>(defaultValue);
		}
		else
		{
			context.SetResult<float>(atof(var->GetValue().c_str()));
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("ADD_CONVAR_CHANGE_LISTENER", [=](fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (!FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			return;
		}

		fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

		if (!resource)
		{
			return;
		}

		const auto varName = context.GetArgument<const char*>(0);

		const std::regex variableRegex = (varName) ? MakeRegex(varName) : std::regex {".*" };

		auto cbRef = fx::FunctionRef{ context.CheckArgument<const char*>(1) };

		const auto varMan = GetVariableManager();

		const auto rm = fx::ResourceManager::GetCurrent();

		auto cookie = varMan->OnConvarModified.Connect(make_shared_function([rm, variableRegex, cbRef = std::move(cbRef), varMan](const std::string& varName)
		{
			if (IsConVarScriptRestricted(varMan, varName))
			{
				return true;
			}

			if (std::regex_match(varName, variableRegex))
			{
				// TODO: actually provide the value changed, msgpack in console stuff was very annoying tho
				rm->CallReference<void>(cbRef.GetRef(), varName, "");
			}

			return true;
		}));

		resource->OnStop.Connect([varMan, cookie]()
		{
			varMan->OnConvarModified.Disconnect(cookie);
		});

		context.SetResult(cookie);
	});


	fx::ScriptEngine::RegisterNativeHandler("REMOVE_CONVAR_CHANGE_LISTENER", [=](fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (!FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			return;
		}


		fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

		if (!resource)
		{
			return;
		}

		auto cookie = context.GetArgument<int>(0);

		auto varMan = GetVariableManager();

		varMan->OnConvarModified.Disconnect(cookie);
	});
});
