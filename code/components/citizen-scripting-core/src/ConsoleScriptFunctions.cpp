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


#include "fxScripting.h"
#include "Utils.h"
#include "om/core.h"


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

});
