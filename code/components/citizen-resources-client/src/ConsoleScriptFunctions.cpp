#include "StdInc.h"
#include <ScriptEngine.h>

#include <CoreConsole.h>

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_CONVAR", [](fx::ScriptContext& context)
	{
		// get variable name
		const std::string varName = context.CheckArgument<const char*>(0);

		// get default value
		const char* defaultValue = context.CheckArgument<const char*>(1);

		// get the console context
		auto consoleContext = console::GetDefaultContext();

		// get the variable manager
		auto varMan = consoleContext->GetVariableManager();

		// check can it be exposed to script
		if (varMan->GetEntryFlags(varName) & ConVar_ScriptRestricted)
		{
			context.SetResult(defaultValue);
			return;
		}

		// get the variable
		auto var = varMan->FindEntryRaw(varName);

		if (!var)
		{
			context.SetResult(defaultValue);
			return;
		}

		static std::string varVal;
		varVal = var->GetValue();

		context.SetResult(varVal.c_str());
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_CONVAR_INT", [](fx::ScriptContext& context)
	{
		const std::string varName = context.CheckArgument<const char*>(0);
		const int defaultValue = context.GetArgument<int>(1);

		// get the server's console context
		auto consoleContext = console::GetDefaultContext();

		// get the variable manager
		auto varMan = consoleContext->GetVariableManager();

		// check can it be exposed to script
		if (varMan->GetEntryFlags(varName) & ConVar_ScriptRestricted)
		{
			context.SetResult(defaultValue);
			return;
		}

		// get the variable
		auto var = varMan->FindEntryRaw(varName);
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
});
