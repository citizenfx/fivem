#include <StdInc.h>

#if __has_include(<GameInput.h>)
#include <GameInput.h>

#include <ScriptEngine.h>
#include <ScriptSerialization.h>

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

			game::RegisterBindingForTag(resource->GetName(), commandString, description, defaultMapper, defaultParameter);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_KEY_MAPPING_HIDE_RESOURCES", [](fx::ScriptContext& context)
	{
		bool hide = context.GetArgument<bool>(0);

		fx::OMPtr<IScriptRuntime> runtime;
		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			game::SetKeyMappingHideResources(hide);
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_BIND_INFO_FROM_COMMAND", [](fx::ScriptContext& context)
	{
		std::string commandString = context.CheckArgument<const char*>(0);

		fx::OMPtr<IScriptRuntime> runtime;
		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			auto bindInfo = game::GetBindingInfo(commandString);
			
			std::map<std::string, msgpack::type::variant> obj;
			obj["tag"]        = bindInfo.tag;
			obj["description"]= bindInfo.description;
			obj["keyName"]    = bindInfo.keyName;
			obj["sourceName"] = bindInfo.sourceName;
			obj["parameter"]  = bindInfo.parameter;
			obj["found"]      = bindInfo.found;
			obj["hasKey"]     = bindInfo.hasKey;

			context.SetResult(fx::SerializeObject(obj));
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_BIND_KEY_FOR_COMMAND", [](fx::ScriptContext& context)
	{
		std::string commandString = context.CheckArgument<const char*>(0);

		fx::OMPtr<IScriptRuntime> runtime;
		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			auto sourceName = context.CheckArgument<const char*>(1);
			auto keyName = context.CheckArgument<const char*>(2);
			context.SetResult(game::RebindCommand(commandString, sourceName, keyName));
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("UNBIND_KEY_FOR_COMMAND", [](fx::ScriptContext& context)
	{
		std::string commandString = context.CheckArgument<const char*>(0);

		fx::OMPtr<IScriptRuntime> runtime;
		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			context.SetResult(game::UnbindCommand(commandString));
		}
	});

	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
	{
		resource->OnStart.Connect([resource]()
		{
			game::SetBindingTagActive(resource->GetName(), true);
		}, INT32_MIN);

		resource->OnStop.Connect([resource]()
		{
			game::SetBindingTagActive(resource->GetName(), false);
		}, INT32_MAX);
	});
});
#endif
