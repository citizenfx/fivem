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

namespace fx
{
enum class ConVarPermission: uint8_t
{
	None,
	Read
};

typedef std::string ResourceName;
typedef std::string ConVarName;

struct ResourceTupleHash
{
	std::size_t operator()(const std::tuple<ResourceName, ConVarName>& t) const noexcept
	{
		return std::hash<ResourceName>()(std::get<0>(t)) ^ (std::hash<ConVarName>()(std::get<1>(t)) << 1);
	}
};

struct ResourceTupleEqual
{
	bool operator()(const std::tuple<ResourceName, ConVarName>& t1, const std::tuple<ResourceName, ConVarName>& t2) const
	{
		return t1 == t2;
	}
};

static bool g_permissionModifyAllowed{true};

static std::unordered_map<std::tuple<ResourceName, ConVarName>, ConVarPermission, ResourceTupleHash, ResourceTupleEqual>
g_permissions{};
// for compatibility reasons only convars that get permissions setted are restricted
// so they get added to this set for fast check if permission check is required
static std::unordered_set<ConVarName> g_restrictedConVars {};
}

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

#if IS_FXSERVER
bool CanReadConVar(const std::string& varName)
{
	if (!fx::g_restrictedConVars.count(varName))
	{
		return true;
	}
	
	fx::OMPtr<IScriptRuntime> runtime;
	std::string currentResourceName;
	if (!FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
	{
		return false;
	}

	fx::Resource* resource = static_cast<fx::Resource*>(runtime->GetParentObject());

	if (!resource)
	{
		return false;
	}

	auto permission = fx::g_permissions.find({resource->GetName(), varName});
	if (permission == fx::g_permissions.end() || permission->second != fx::ConVarPermission::Read)
	{
		return false;
	}

	return true;
}
#else
bool CanReadConVar(const std::string& varName)
{
	return true;
}
#endif

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

	if (!CanReadConVar(varName))
	{
		context.SetResult(defaultValue);
		return;
	}

	context.SetResult(returnValue);
}

static InitFunction initFunction([]()
{
#if IS_FXSERVER
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		static ConsoleCommand addFSPermCmd("add_convar_permission", [](const std::string& resource, const std::string& operation, const std::string& conVar)
		{
			if (!fx::g_permissionModifyAllowed)
			{
				console::PrintWarning(_CFX_NAME_STRING(_CFX_COMPONENT_NAME),
				"add_convar_permission is only executable before the server finished execution.\n"
				);
				return;
			}
			
			if (operation != "read")
			{
				// make operation configurable for future usage
				return;
			}

			fx::g_restrictedConVars.insert(conVar);

			fx::g_permissions[{resource, conVar}] = fx::ConVarPermission::Read;
		});
	});
#endif
	
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
			if (!CanReadConVar(varName))
			{
				return true;
			}

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
