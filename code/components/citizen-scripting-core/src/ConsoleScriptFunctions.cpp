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

template<typename T>
void GetConVar(fx::ScriptContext& context)
{
	// get variable name
	const std::string varName = context.CheckArgument<const char*>(0);
	const auto defaultValue = context.GetArgument<T>(1);

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

	if (std::is_integral_v<T> || std::is_same_v<T, bool>)
	{
		// convert the string to lower case so if the variables were capitalized
		// for some reason we'll still get proper values
		LowerString(varVal);
		if (varVal == "true")
		{
			varVal = "1";
		}
		else if (varVal == "false")
		{
			varVal = "0";
		}
	}

	T returnValue = defaultValue;

	try
	{
		if constexpr (std::is_integral_v<T> || std::is_same_v<T, bool>)
		{
			returnValue = std::stoi(varVal);
		}
		if constexpr (std::is_floating_point_v<T>)
		{
			returnValue = std::stof(varVal);
		}
		if constexpr (std::is_same_v<T, const char*>)
		{
			returnValue = varVal.c_str();
		}
	}
	catch (std::exception& _v)
	{
		// We failed to convert, set to default value
		context.SetResult(defaultValue);
		return;
	}

	context.SetResult(returnValue);
}

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_CONVAR", GetConVar<const char*>);

	fx::ScriptEngine::RegisterNativeHandler("GET_CONVAR_INT", GetConVar<int>);

	fx::ScriptEngine::RegisterNativeHandler("GET_CONVAR_FLOAT", GetConVar<float>);

	fx::ScriptEngine::RegisterNativeHandler("GET_CONVAR_BOOL", GetConVar<bool>);

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
