#include <StdInc.h>

#if __has_include(<GameInput.h>)
#include <GameInput.h>

#include <ScriptEngine.h>

#include <Resource.h>
#include <fxScripting.h>

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("REGISTER_KEY_MAPPING", [](fx::ScriptContext& context)
	{
		std::string commandString = context.CheckArgument<const char*>(0);
		std::string description = context.CheckArgument<const char*>(1);
		std::string defaultMapper = context.CheckArgument<const char*>(2);
		std::string defaultParameter = context.CheckArgument<const char*>(3);

		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			game::SetBindingTagActive(resource->GetName(), true);

			resource->OnStop.Connect([resource]()
			{
				game::SetBindingTagActive(resource->GetName(), false);
			});

			game::RegisterBindingForTag(resource->GetName(), commandString, description, defaultMapper, defaultParameter);
		}
	});
});
#endif
