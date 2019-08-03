/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ScriptEngine.h>

#include <Resource.h>
#include <ResourceManager.h>
#include <fxScripting.h>

#include <ScriptSerialization.h>

#include <CoreConsole.h>
#include <se/Security.h>

struct CommandObject
{
	std::string name;

	CommandObject(const std::string& name)
		: name(name)
	{

	}

	MSGPACK_DEFINE_MAP(name);
};

static InitFunction initFunction([] ()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_CURRENT_RESOURCE_NAME", [] (fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;
		
		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			if (resource)
			{
				context.SetResult(resource->GetName().c_str());
				return;
			}
		}

		context.SetResult(nullptr);
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INVOKING_RESOURCE", [](fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetInvokingScriptRuntime(&runtime)))
		{
			if (runtime.GetRef())
			{
				fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

				if (resource)
				{
					context.SetResult(resource->GetName().c_str());
					return;
				}
			}
		}

		context.SetResult(nullptr);
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_DUPLICITY_VERSION", [](fx::ScriptContext& context)
	{
		context.SetResult(
#ifdef IS_FXSERVER
			true
#else
			false
#endif
		);
	});

	fx::ScriptEngine::RegisterNativeHandler("EXECUTE_COMMAND", [](fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			if (resource)
			{
				auto resourceManager = resource->GetManager();

				se::ScopedPrincipal principal(se::Principal{ fmt::sprintf("resource.%s", resource->GetName()) });

				resourceManager->GetComponent<console::Context>()->ExecuteSingleCommand(context.CheckArgument<const char*>(0));
			}
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("REGISTER_COMMAND", [](fx::ScriptContext& context)
	{
		std::string commandName = context.CheckArgument<const char*>(0);
		std::string commandRef = context.CheckArgument<const char*>(1);

		fx::OMPtr<IScriptRuntime> runtime;

		static std::map<std::string, std::string> outerRefs;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			if (resource)
			{
				auto resourceManager = resource->GetManager();
				auto consoleCxt = resourceManager->GetComponent<console::Context>();

				outerRefs[commandName] = commandRef;

				// restricted? if not, add the command
				if (!context.GetArgument<bool>(2))
				{
					seGetCurrentContext()->AddAccessControlEntry(se::Principal{ "builtin.everyone" }, se::Object{ "command." + commandName }, se::AccessType::Allow);
				}

				consoleCxt->GetCommandManager()->Register(commandName, [=](ConsoleExecutionContext& context)
				{
					try
					{
						auto source = (!context.contextRef.empty()) ? context.contextRef : "0";
						const auto& args = context.arguments.GetArguments();

						resourceManager->CallReference<void>(outerRefs[commandName], atoi(source.c_str()), args, consoleCxt->GetCommandManager()->GetRawCommand());
					}
					catch (std::bad_any_cast& e)
					{
						trace("caught bad_any_cast in command handler for %s\n", commandName);
					}

					return true;
				});
			}
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_REGISTERED_COMMANDS", [](fx::ScriptContext& context)
	{
		std::vector<CommandObject> commandList;

		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			if (resource)
			{
				auto resourceManager = resource->GetManager();
				auto consoleCxt = resourceManager->GetComponent<console::Context>();

				consoleCxt->GetCommandManager()->ForAllCommands([&](const std::string& commandName)
				{
					commandList.emplace_back(commandName);
				});

				context.SetResult(fx::SerializeObject(commandList));
			}
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_INSTANCE_ID", [](fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			context.SetResult(runtime->GetInstanceId());
		}
		else
		{
			context.SetResult(0);
		}
	});

	static std::vector<fx::Resource*> resources;

	fx::ScriptEngine::RegisterNativeHandler("GET_NUM_RESOURCES", [](fx::ScriptContext& context)
	{
		resources.clear();

		auto manager = fx::ResourceManager::GetCurrent();
		manager->ForAllResources([&] (fwRefContainer<fx::Resource> resource)
		{
			resources.push_back(resource.GetRef());
		});

		context.SetResult(resources.size());
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_RESOURCE_BY_FIND_INDEX", [](fx::ScriptContext& context)
	{
		int i = context.GetArgument<int>(0);
		if (i < 0 || i >= resources.size())
		{
			context.SetResult(nullptr);
			return;
		}

		context.SetResult(resources[i]->GetName().c_str());
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_RESOURCE_STATE", [](fx::ScriptContext& context)
	{
		// find the resource
		fx::ResourceManager* resourceManager = fx::ResourceManager::GetCurrent();
		fwRefContainer<fx::Resource> resource = resourceManager->GetResource(context.CheckArgument<const char*>(0));

		if (!resource.GetRef())
		{
			context.SetResult<const char*>("missing");
			return;
		}

		auto state = resource->GetState();

		switch (state)
		{
		case fx::ResourceState::Started:
			context.SetResult<const char*>("started");
			break;
		case fx::ResourceState::Starting:
			context.SetResult<const char*>("starting");
			break;
		case fx::ResourceState::Stopped:
			context.SetResult<const char*>("stopped");
			break;
		case fx::ResourceState::Stopping:
			context.SetResult<const char*>("stopping");
			break;
		case fx::ResourceState::Uninitialized:
			context.SetResult<const char*>("uninitialized");
			break;
		default:
			context.SetResult<const char*>("unknown");
			break;
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_ACE_ALLOWED", [](fx::ScriptContext& context)
	{
		context.SetResult(seCheckPrivilege(context.CheckArgument<const char*>(0)));
	});

	fx::ScriptEngine::RegisterNativeHandler("IS_PRINCIPAL_ACE_ALLOWED", [](fx::ScriptContext& context)
	{
		se::ScopedPrincipalReset reset;
		se::ScopedPrincipal principalScope(se::Principal{ context.CheckArgument<const char*>(0) });

		context.SetResult(seCheckPrivilege(context.CheckArgument<const char*>(1)));
	});
});
