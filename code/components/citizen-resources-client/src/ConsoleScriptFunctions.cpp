#include "StdInc.h"
#include <ScriptEngine.h>

#include <CoreConsole.h>

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_CONVAR", [](fx::ScriptContext& context)
	{
		// get variable name and validate it
		std::string varName = context.CheckArgument<const char*>(0);

		// this should not be exposed to script
		// #TODO: 'not exposed to script' convar flags
		if (HashString(varName.c_str()) == HashString("cl_ownershipTicket"))
		{
			context.SetResult(context.CheckArgument<const char*>(1));
			return;
		}

		// get the console context
		auto consoleContext = console::GetDefaultContext();

		// get the variable manager
		auto varMan = consoleContext->GetVariableManager();

		// get the variable
		auto var = varMan->FindEntryRaw(varName);

		if (!var)
		{
			context.SetResult(context.CheckArgument<const char*>(1));
		}
		else
		{
			static std::string varVal;
			varVal = var->GetValue();

			context.SetResult(varVal.c_str());
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_CONVAR_INT", [](fx::ScriptContext& context)
	{
		// get the server's console context
		auto consoleContext = console::GetDefaultContext();

		// get the variable manager
		auto varMan = consoleContext->GetVariableManager();

		// get the variable
		auto var = varMan->FindEntryRaw(context.CheckArgument<const char*>(0));

		if (!var)
		{
			context.SetResult(context.GetArgument<int>(1));
		}
		else
		{
			context.SetResult(atoi(var->GetValue().c_str()));
		}
	});
});
