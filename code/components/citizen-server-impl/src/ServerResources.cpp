#include "StdInc.h"

#include <ResourceManager.h>
#include <ResourceEventComponent.h>
#include <ResourceMetaDataComponent.h>

#include <ServerInstanceBase.h>
#include <ServerInstanceBaseRef.h>

#include <GameServer.h>
#include <ServerEventComponent.h>

#include <RelativeDevice.h>

#include <VFSManager.h>

#include <network/uri.hpp>

#include <PrintListener.h>

class LocalResourceMounter : public fx::ResourceMounter
{
public:
	LocalResourceMounter(fx::ResourceManager* manager)
		: m_manager(manager)
	{
		
	}

	virtual bool HandlesScheme(const std::string& scheme) override
	{
		return (scheme == "file");
	}

	virtual pplx::task<fwRefContainer<fx::Resource>> LoadResource(const std::string& uri) override
	{
		std::error_code ec;
		auto uriParsed = network::make_uri(uri, ec);

		fwRefContainer<fx::Resource> resource;

		if (!ec)
		{
			auto pathRef = uriParsed.path();
			auto fragRef = uriParsed.fragment();

			if (pathRef && fragRef)
			{
				std::vector<char> path;
#ifdef _WIN32
				std::string pr = pathRef->substr(1).to_string();
#else
				std::string pr = pathRef->to_string();
#endif
				network::uri::decode(pr.begin(), pr.end(), std::back_inserter(path));

				resource = m_manager->CreateResource(fragRef->to_string());
				resource->LoadFrom(std::string(path.begin(), path.begin() + path.size()));
			}
		}

		return pplx::task_from_result<fwRefContainer<fx::Resource>>(resource);
	}

private:
	fx::ResourceManager* m_manager;
};

static void HandleServerEvent(fx::ServerInstanceBase* instance, const std::shared_ptr<fx::Client>& client, net::Buffer& buffer)
{
	uint16_t eventNameLength = buffer.Read<uint16_t>();

	std::vector<char> eventNameBuffer(eventNameLength - 1);
	buffer.Read(eventNameBuffer.data(), eventNameBuffer.size());
	buffer.Read<uint8_t>();

	uint32_t dataLength = buffer.GetRemainingBytes();

	std::vector<uint8_t> data(dataLength);
	buffer.Read(data.data(), data.size());

	fwRefContainer<fx::ResourceManager> resourceManager = instance->GetComponent<fx::ResourceManager>();
	fwRefContainer<fx::ResourceEventManagerComponent> eventManager = resourceManager->GetComponent<fx::ResourceEventManagerComponent>();

	eventManager->QueueEvent(
		std::string(eventNameBuffer.begin(), eventNameBuffer.end()),
		std::string(data.begin(), data.end()),
		fmt::sprintf("net:%d", client->GetNetId())
	);
}

static void ScanResources(fx::ServerInstanceBase* instance)
{
	auto resMan = instance->GetComponent<fx::ResourceManager>();

	std::string resourceRoot(instance->GetRootPath() + "/resources/");

	std::queue<std::string> pathsToIterate;
	pathsToIterate.push(resourceRoot);

	std::vector<pplx::task<fwRefContainer<fx::Resource>>> tasks;

	while (!pathsToIterate.empty())
	{
		std::string thisPath = pathsToIterate.front();
		pathsToIterate.pop();

		auto vfsDevice = vfs::GetDevice(thisPath);

		vfs::FindData findData;
		auto handle = vfsDevice->FindFirst(thisPath, &findData);

		if (handle != INVALID_DEVICE_HANDLE)
		{
			do
			{
				if (findData.name[0] == '.')
				{
					continue;
				}

				// TODO(fxserver): non-win32
				if (findData.attributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					std::string resPath(thisPath + "/" + findData.name);

					// is this a category?
					if (findData.name[0] == '[' && findData.name[findData.name.size() - 1] == ']')
					{
						pathsToIterate.push(resPath);
					}
					// it's a resource
					else
					{
						auto oldRes = resMan->GetResource(findData.name);

						if (oldRes.GetRef())
						{
							oldRes->GetComponent<fx::ResourceMetaDataComponent>()->LoadMetaData(resPath);
						}
						else
						{
							trace("Found new resource %s in %s\n", findData.name, resPath);

							tasks.push_back(resMan->AddResource(network::uri_builder{}
								.scheme("file")
								.host("")
								.path(resPath)
								.fragment(findData.name)
								.uri().string()));
						}
					}
				}
			} while (vfsDevice->FindNext(handle, &findData));

			vfsDevice->FindClose(handle);
		}
	}

	pplx::when_all(tasks.begin(), tasks.end()).wait();

	instance
		->GetComponent<fx::ResourceManager>()
		->GetComponent<fx::ResourceEventManagerComponent>()
		->TriggerEvent2("onResourceListRefresh", {});
}

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->SetComponent(fx::CreateResourceManager());
		instance->SetComponent(new fx::ServerEventComponent());

		fwRefContainer<fx::ResourceManager> resman = instance->GetComponent<fx::ResourceManager>();
		resman->SetComponent(new fx::ServerInstanceBaseRef(instance));
		resman->SetComponent(instance->GetComponent<console::Context>());

		resman->AddMounter(new LocalResourceMounter(resman.GetRef()));

		fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
		{
			fx::ServerInstanceBase* instance = resource->GetManager()->GetComponent<fx::ServerInstanceBaseRef>()->Get();
			
			resource->OnStart.Connect([=]()
			{
				trace("Started resource %s\n", resource->GetName());

				auto metaData = resource->GetComponent<fx::ResourceMetaDataComponent>();
				auto iv = metaData->GetEntries("server_only");

				if (iv.begin() != iv.end())
				{
					return;
				}

				auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

				net::Buffer outBuffer;
				outBuffer.Write(HashRageString("msgResStart"));
				outBuffer.Write(resource->GetName().c_str(), resource->GetName().length());

				clientRegistry->ForAllClients([&](const std::shared_ptr<fx::Client>& client)
				{
					client->SendPacket(0, outBuffer, ENET_PACKET_FLAG_RELIABLE);
				});
			}, 99999999);

			resource->OnStop.Connect([=]()
			{
				trace("Stopping resource %s\n", resource->GetName());

				auto metaData = resource->GetComponent<fx::ResourceMetaDataComponent>();
				auto iv = metaData->GetEntries("server_only");

				if (iv.begin() != iv.end())
				{
					return;
				}

				auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

				net::Buffer outBuffer;
				outBuffer.Write(HashRageString("msgResStop"));
				outBuffer.Write(resource->GetName().c_str(), resource->GetName().length());

				clientRegistry->ForAllClients([&](const std::shared_ptr<fx::Client>& client)
				{
					client->SendPacket(0, outBuffer, ENET_PACKET_FLAG_RELIABLE);
				});
			}, -1000);
		});

		{
			static auto citizenDir = instance->AddVariable<std::string>("citizen_dir", ConVar_None, "");

			// create cache directory if needed
			auto device = vfs::GetDevice(instance->GetRootPath());
			auto cacheDir = instance->GetRootPath() + "/cache/";

			if (device.GetRef())
			{
				device->CreateDirectory(cacheDir);
			}

			vfs::Mount(new vfs::RelativeDevice(citizenDir->GetValue() + "/"), "citizen:/");
			vfs::Mount(new vfs::RelativeDevice(cacheDir), "cache:/");
		}

		ScanResources(instance);

		static auto commandRef = instance->AddCommand("start", [=](const std::string& resourceName)
		{
			if (resourceName.empty())
			{
				return;
			}

			auto resource = resman->GetResource(resourceName);

			if (!resource.GetRef())
			{
				trace("^3Couldn't find resource %s.^7\n", resourceName);
				return;
			}

			if (!resource->Start())
			{
				if (resource->GetState() == fx::ResourceState::Stopped)
				{
					trace("^3Couldn't start resource %s.^7\n", resourceName);
					return;
				}
			}
		});

		static auto stopCommandRef = instance->AddCommand("stop", [=](const std::string& resourceName)
		{
			if (resourceName.empty())
			{
				return;
			}

			auto resource = resman->GetResource(resourceName);

			if (!resource.GetRef())
			{
				trace("^3Couldn't find resource %s.^7\n", resourceName);
				return;
			}

			if (!resource->Stop())
			{
				if (resource->GetState() != fx::ResourceState::Stopped)
				{
					trace("^3Couldn't stop resource %s.^7\n", resourceName);
					return;
				}
			}
		});

		static auto restartCommandRef = instance->AddCommand("restart", [=](const std::string& resourceName)
		{
			auto resource = resman->GetResource(resourceName);

			if (!resource.GetRef())
			{
				trace("^3Couldn't find resource %s.^7\n", resourceName);
				return;
			}

			if (resource->GetState() != fx::ResourceState::Started)
			{
				trace("Can't restart a stopped resource.\n");
				return;
			}

			auto conCtx = instance->GetComponent<console::Context>();
			conCtx->ExecuteSingleCommandDirect(ProgramArguments{ "stop", resourceName });
			conCtx->ExecuteSingleCommandDirect(ProgramArguments{ "start", resourceName });
		});

		static auto refreshCommandRef = instance->AddCommand("refresh", [=]()
		{
			ScanResources(instance);
		});

		instance->GetComponent<console::Context>()->GetCommandManager()->FallbackEvent.Connect([=](const std::string& commandName, const ProgramArguments& arguments, const std::string& context)
		{
			auto eventComponent = resman->GetComponent<fx::ResourceEventManagerComponent>();

			// assert privilege
			if (!seCheckPrivilege(fmt::sprintf("command.%s", commandName)))
			{
				return true;
			}

			// if canceled, the command was handled, so cancel the fwEvent
			return (eventComponent->TriggerEvent2("rconCommand", {}, commandName, arguments.GetArguments()));
		}, -100);

		static thread_local std::string rawCommand;

		instance->GetComponent<console::Context>()->GetCommandManager()->FallbackEvent.Connect([=](const std::string& commandName, const ProgramArguments& arguments, const std::string& context)
		{
			if (!context.empty())
			{
				auto eventComponent = resman->GetComponent<fx::ResourceEventManagerComponent>();

				try
				{
					return eventComponent->TriggerEvent2("__cfx_internal:commandFallback", { "net:" + context }, rawCommand);
				}
				catch (std::bad_any_cast& e)
				{
					trace("caught bad_any_cast in FallbackEvent handler for %s\n", commandName);
				}
			}

			return true;
		}, 99999);

		auto gameServer = instance->GetComponent<fx::GameServer>();
		gameServer->GetComponent<fx::HandlerMapComponent>()->Add(HashRageString("msgServerEvent"), std::bind(&HandleServerEvent, instance, std::placeholders::_1, std::placeholders::_2));

		gameServer->GetComponent<fx::HandlerMapComponent>()->Add(HashRageString("msgServerCommand"), [=](const std::shared_ptr<fx::Client>& client, net::Buffer& buffer)
		{
			auto cmdLen = buffer.Read<uint16_t>();

			std::vector<char> cmd(cmdLen);
			buffer.Read(cmd.data(), cmdLen);

			std::string printString;

			fx::PrintListenerContext context([&](const std::string_view& print)
			{
				printString += print;
			});

			fx::ScopeDestructor destructor([&]()
			{
				msgpack::sbuffer sb;

				msgpack::packer<msgpack::sbuffer> packer(sb);
				packer.pack_array(1).pack(printString);

				instance->GetComponent<fx::ServerEventComponent>()->TriggerClientEvent("__cfx_internal:serverPrint", sb.data(), sb.size(), { std::to_string(client->GetNetId()) });
			});

			// save the raw command for fallback usage
			rawCommand = std::string(cmd.begin(), cmd.end());

			// invoke
			auto consoleCxt = instance->GetComponent<console::Context>();
			consoleCxt->GetCommandManager()->Invoke(rawCommand, std::to_string(client->GetNetId()));

			// unset raw command
			rawCommand = "";
		});

		gameServer->OnTick.Connect([=]()
		{
			resman->Tick();
		});
	}, 50);
});

#include <ScriptEngine.h>
#include <optional>

void fx::ServerEventComponent::TriggerClientEvent(const std::string_view& eventName, const void* data, size_t dataLen, const std::optional<std::string_view>& targetSrc)
{
	// build the target event
	net::Buffer outBuffer;
	outBuffer.Write(0x7337FD7A);

	// source netId
	outBuffer.Write<uint16_t>(-1);

	// event name
	outBuffer.Write<uint16_t>(eventName.size() + 1);
	outBuffer.Write(eventName.data(), eventName.size());
	outBuffer.Write<uint8_t>(0);

	// payload
	outBuffer.Write(data, dataLen);

	// get the game server and client registry
	auto gameServer = m_instance->GetComponent<fx::GameServer>();
	auto clientRegistry = m_instance->GetComponent<fx::ClientRegistry>();

	// do we have a specific client to send to?
	if (targetSrc)
	{
		int targetNetId = atoi(targetSrc->data());
		auto client = clientRegistry->GetClientByNetID(targetNetId);

		if (client)
		{
			// TODO(fxserver): >MTU size?
			client->SendPacket(0, outBuffer, ENET_PACKET_FLAG_RELIABLE);
		}
	}
	else
	{
		clientRegistry->ForAllClients([&](const std::shared_ptr<fx::Client>& client)
		{
			client->SendPacket(0, outBuffer, ENET_PACKET_FLAG_RELIABLE);
		});
	}
}

static InitFunction initFunction2([]()
{
	fx::ScriptEngine::RegisterNativeHandler("TRIGGER_CLIENT_EVENT_INTERNAL", [](fx::ScriptContext& context)
	{
		std::string_view eventName = context.CheckArgument<const char*>(0);
		std::optional<std::string_view> targetSrc;

		{
			auto targetSrcIdx = context.CheckArgument<const char*>(1);

			if (strcmp(targetSrcIdx, "-1") != 0)
			{
				targetSrc = targetSrcIdx;
			}
		}

		const void* data = context.GetArgument<const void*>(2);
		uint32_t dataLen = context.GetArgument<uint32_t>(3);

		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		instance->GetComponent<fx::ServerEventComponent>()->TriggerClientEvent(eventName, data, dataLen, targetSrc);
	});

	fx::ScriptEngine::RegisterNativeHandler("START_RESOURCE", [](fx::ScriptContext& context)
	{
		auto resourceManager = fx::ResourceManager::GetCurrent();
		fwRefContainer<fx::Resource> resource = resourceManager->GetResource(context.CheckArgument<const char*>(0));

		bool success = false;

		if (resource.GetRef())
		{
			success = resource->Start();
		}

		context.SetResult(success);
	});

	fx::ScriptEngine::RegisterNativeHandler("STOP_RESOURCE", [](fx::ScriptContext& context)
	{
		auto resourceManager = fx::ResourceManager::GetCurrent();
		fwRefContainer<fx::Resource> resource = resourceManager->GetResource(context.CheckArgument<const char*>(0));

		bool success = false;

		if (resource.GetRef())
		{
			success = resource->Stop();
		}

		context.SetResult(success);
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_GAME_TYPE", [](fx::ScriptContext& context)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's console context
		auto consoleContext = instance->GetComponent<console::Context>();

		se::ScopedPrincipal principalScope(se::Principal{ "system.console" });

		// set gametype variable
		consoleContext->ExecuteSingleCommandDirect(ProgramArguments{ "set", "gametype", context.CheckArgument<const char*>(0) });
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_MAP_NAME", [](fx::ScriptContext& context)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's console context
		auto consoleContext = instance->GetComponent<console::Context>();

		se::ScopedPrincipal principalScope(se::Principal{ "system.console" });

		// set mapname variable
		consoleContext->ExecuteSingleCommandDirect(ProgramArguments{ "set", "mapname", context.CheckArgument<const char*>(0) });
	});

	fx::ScriptEngine::RegisterNativeHandler("ENABLE_ENHANCED_HOST_SUPPORT", [](fx::ScriptContext& context)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's console context
		auto consoleContext = instance->GetComponent<console::Context>();

		se::ScopedPrincipal principalScope(se::Principal{ "system.console" });

		// set mapname variable
		consoleContext->ExecuteSingleCommandDirect(ProgramArguments{ "set", "sv_enhancedHostSupport", context.GetArgument<bool>(0) ? "1" : "0" });
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_CONVAR", [](fx::ScriptContext& context)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's console context
		auto consoleContext = instance->GetComponent<console::Context>();

		// get the variable manager
		auto varMan = consoleContext->GetVariableManager();

		// get the variable
		auto var = varMan->FindEntryRaw(context.CheckArgument<const char*>(0));

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
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's console context
		auto consoleContext = instance->GetComponent<console::Context>();

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

	fx::ScriptEngine::RegisterNativeHandler("SET_CONVAR", [](fx::ScriptContext& context)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's console context
		auto consoleContext = instance->GetComponent<console::Context>();

		se::ScopedPrincipal principalScope(se::Principal{ "system.console" });

		// set variable
		consoleContext->ExecuteSingleCommandDirect(ProgramArguments{ "set", context.CheckArgument<const char*>(0), context.CheckArgument<const char*>(1) });
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_CONVAR_SERVER_INFO", [](fx::ScriptContext& context)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's console context
		auto consoleContext = instance->GetComponent<console::Context>();

		se::ScopedPrincipal principalScope(se::Principal{ "system.console" });

		// set variable
		consoleContext->ExecuteSingleCommandDirect(ProgramArguments{ "sets", context.CheckArgument<const char*>(0), context.CheckArgument<const char*>(1) });
	});

	fx::ScriptEngine::RegisterNativeHandler("SET_CONVAR_REPLICATED", [](fx::ScriptContext& context)
	{
		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto instance = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get();

		// get the server's console context
		auto consoleContext = instance->GetComponent<console::Context>();

		se::ScopedPrincipal principalScope(se::Principal{ "system.console" });

		// set variable
		consoleContext->ExecuteSingleCommandDirect(ProgramArguments{ "setr", context.CheckArgument<const char*>(0), context.CheckArgument<const char*>(1) });
	});
});
