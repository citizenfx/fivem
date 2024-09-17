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

#include <StateBagComponent.h>

#include <ScriptSerialization.h>

#include <ResourceCallbackComponent.h>

#include <CoreConsole.h>
#include <se/Security.h>

#include <SharedFunction.h>

struct CommandObject
{
	std::string name;
	int32_t arity;

	CommandObject(const std::string& name, size_t arity)
		: name(name), arity(arity)
	{

	}

	MSGPACK_DEFINE_MAP(name, arity);
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

				if (consoleCxt->GetCommandManager()->HasCommand(commandName))
				{
					return;
				}

				// restricted? if not, add the command
				if (!context.GetArgument<bool>(2))
				{
					seGetCurrentContext()->AddAccessControlEntry(se::Principal{ "builtin.everyone" }, se::Object{ "command." + commandName }, se::AccessType::Allow);
				}

				int commandToken = consoleCxt->GetCommandManager()->Register(commandName, [=](ConsoleExecutionContext& context)
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

				resource->OnStop.Connect([consoleCxt, commandToken]()
				{
					consoleCxt->GetCommandManager()->Unregister(commandToken);
				}, INT32_MAX);
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

				consoleCxt->GetCommandManager()->ForAllCommands2([&commandList](const console::CommandMetadata& command)
				{
					commandList.emplace_back(command.GetName(), (command.GetArity() == -1) ? -1 : int32_t(command.GetArity()));
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

#ifdef _DEBUG
	fx::ScriptEngine::RegisterNativeHandler("_TEST_ERROR_NATIVE", [](fx::ScriptContext& context) 
	{
		*(volatile int*)0 = 0xDEAD;
	});

	fx::ScriptEngine::RegisterNativeHandler("_TEST_EXCEPTION_NATIVE", [](fx::ScriptContext& context)
	{
		throw std::exception("_TEST_EXCEPTION_NATIVE called!");
	});
#endif

	fx::ScriptEngine::RegisterNativeHandler("GET_STATE_BAG_VALUE", [](fx::ScriptContext& context)
	{
		auto bagName = context.CheckArgument<const char*>(0);
		auto keyName = context.CheckArgument<const char*>(1);

		auto rm = fx::ResourceManager::GetCurrent();
		auto sbac = rm->GetComponent<fx::StateBagComponent>();

		if (auto bag = sbac->GetStateBag(bagName); bag)
		{
			auto entry = bag->GetKey(keyName);

			if (entry)
			{
				static thread_local std::string tmp;
				tmp = *entry;

				context.SetResult(fx::scrObject{ tmp.c_str(), tmp.size() });
				return;
			}
		}

		context.SetResult(fx::SerializeObject(msgpack::type::nil_t{}));
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_STATE_BAG_VALUE", [](fx::ScriptContext& context)
	{
		auto bagName = context.CheckArgument<const char*>(0);
		auto keyName = context.CheckArgument<const char*>(1);
		auto keyValue = context.CheckArgument<const char*>(2);
		auto keySize = context.GetArgument<uint32_t>(3);
		auto replicated = context.GetArgument<bool>(4);

		auto rm = fx::ResourceManager::GetCurrent();
		auto sbac = rm->GetComponent<fx::StateBagComponent>();

		if (auto bag = sbac->GetStateBag(bagName); bag)
		{
			bag->SetKey(0, keyName, std::string_view{ keyValue, keySize }, replicated);
			context.SetResult(true);

			return;
		}

		context.SetResult(false);
	});

	fx::ScriptEngine::RegisterNativeHandler("ADD_STATE_BAG_CHANGE_HANDLER", [](fx::ScriptContext& context)
	{
		fx::OMPtr<IScriptRuntime> runtime;

		static std::map<std::string, std::string> outerRefs;

		if (!FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			return;
		}

		fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

		if (!resource)
		{
			return;
		}

		auto keyName = context.GetArgument<const char*>(0);
		auto bagName = context.GetArgument<const char*>(1);

		std::string keyNameRef = (keyName) ? keyName : "";
		std::string bagNameRef = (bagName) ? bagName : "";
		auto cbRef = fx::FunctionRef{ context.CheckArgument<const char*>(2) };

		auto rm = fx::ResourceManager::GetCurrent();
		auto sbac = rm->GetComponent<fx::StateBagComponent>();

		auto cookie = sbac->OnStateBagChange.Connect(make_shared_function([rm, keyNameRef, bagNameRef, cbRef = std::move(cbRef)](int source, std::string_view bagName, std::string_view key, const msgpack::object& value, bool replicated)
		{
			if (keyNameRef.empty() || key == keyNameRef)
			{
				if (bagNameRef.empty() || bagName == bagNameRef)
				{
					rm->CallReference<void>(cbRef.GetRef(), std::string{ bagName }, std::string{ key }, value, source, replicated);
				}
			}

			return true;
		}));

		resource->OnStop.Connect([sbac, cookie]()
		{
			sbac->OnStateBagChange.Disconnect(cookie);
		});

		context.SetResult(cookie);
	});

	fx::ScriptEngine::RegisterNativeHandler("REMOVE_STATE_BAG_CHANGE_HANDLER", [](fx::ScriptContext& context)
	{
		auto cookie = context.GetArgument<int>(0);
		auto rm = fx::ResourceManager::GetCurrent();
		auto sbac = rm->GetComponent<fx::StateBagComponent>();

		sbac->OnStateBagChange.Disconnect(size_t(cookie));
	});

	fx::ScriptEngine::RegisterNativeHandler("STATE_BAG_HAS_KEY", [](fx::ScriptContext& context)
	{
		auto bagName = context.CheckArgument<const char*>(0);
		auto keyName = context.CheckArgument<const char*>(1);

		auto rm = fx::ResourceManager::GetCurrent();
		auto sbac = rm->GetComponent<fx::StateBagComponent>();

		auto bag = sbac->GetStateBag(bagName);

		if (!bag)
		{
			context.SetResult(false);
			return;
		}

		context.SetResult(bag->HasKey(keyName));
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_STATE_BAG_KEYS", [](fx::ScriptContext& context)
	{
		auto bagName = context.CheckArgument<const char*>(0);

		auto rm = fx::ResourceManager::GetCurrent();
		auto sbac = rm->GetComponent<fx::StateBagComponent>();

		std::vector<std::string> keys;

		if (auto bag = sbac->GetStateBag(bagName))
		{
			keys = bag->GetKeys();
		}

		context.SetResult(fx::SerializeObject(keys));
	});
});
