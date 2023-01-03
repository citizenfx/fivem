#include "StdInc.h"

#include <ResourceManager.h>
#include <ResourceEventComponent.h>
#include <ResourceMetaDataComponent.h>
#include <ResourceManagerConstraintsComponent.h>
#include <ResourceScriptingComponent.h>

#include <fxScripting.h>

#include <ServerInstanceBase.h>
#include <ServerInstanceBaseRef.h>

#include "ServerResourceList.h"

#include <GameServer.h>
#include <ServerEventComponent.h>

#include <GameBuilds.h>

#include <RelativeDevice.h>

#include <VFSManager.h>

#include <PrintListener.h>

#include <ResourceStreamComponent.h>
#include <EventReassemblyComponent.h>

#include <KeyedRateLimiter.h>

#include <StructuredTrace.h>

#include <ScriptEngine.h>

#include <cfx_version.h>

#include <boost/algorithm/string.hpp>

// a set of resources that are system-managed and should not be stopped from script
static std::set<std::string> g_managedResources = {
	"spawnmanager",
	"mapmanager",
	"baseevents",
	"chat",
	"sessionmanager",
	"webadmin",
	"monitor"
};

static void HandleServerEvent(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client, net::Buffer& buffer)
{
	uint16_t eventNameLength = buffer.Read<uint16_t>();

	// validate input
	if (eventNameLength <= 0 || eventNameLength > std::numeric_limits<uint16_t>::max())
	{
		return;
	}

	static fx::RateLimiterStore<uint32_t, false> netEventRateLimiterStore{ instance->GetComponent<console::Context>().GetRef() };
	static auto netEventRateLimiter = netEventRateLimiterStore.GetRateLimiter("netEvent", fx::RateLimiterDefaults{ 50.f, 200.f });
	static auto netFloodRateLimiter = netEventRateLimiterStore.GetRateLimiter("netEventFlood", fx::RateLimiterDefaults{ 75.f, 300.f });
	static auto netEventSizeRateLimiter = netEventRateLimiterStore.GetRateLimiter("netEventSize", fx::RateLimiterDefaults{ 128 * 1024.0, 384 * 1024.0 });

	uint32_t netId = client->GetNetId();

	if (!netEventRateLimiter->Consume(netId))
	{
		if (!netFloodRateLimiter->Consume(netId))
		{
			gscomms_execute_callback_on_main_thread([client, instance]()
			{
				instance->GetComponent<fx::GameServer>()->DropClient(client, "Reliable network event overflow.");
			});
		}

		return;
	}

	std::vector<char> eventNameBuffer(eventNameLength - 1);
	buffer.Read(eventNameBuffer.data(), eventNameBuffer.size());
	buffer.Read<uint8_t>();

	uint32_t dataLength = buffer.GetRemainingBytes();

	if (!netEventSizeRateLimiter->Consume(netId, double(dataLength)))
	{
		std::string eventName(eventNameBuffer.begin(), eventNameBuffer.end());
		gscomms_execute_callback_on_main_thread([client, instance, eventName]()
		{
			// if this happens, try increasing rateLimiter_netEventSize_rate and rateLimiter_netEventSize_burst
			// preferably, fix client scripts to not have this large a set of events with high frequency
			instance->GetComponent<fx::GameServer>()->DropClient(client, "Reliable network event size overflow: %s", eventName);
		});

		return;
	}

	std::vector<uint8_t> data(dataLength);
	buffer.Read(data.data(), data.size());

	fwRefContainer<fx::ResourceManager> resourceManager = instance->GetComponent<fx::ResourceManager>();
	fwRefContainer<fx::ResourceEventManagerComponent> eventManager = resourceManager->GetComponent<fx::ResourceEventManagerComponent>();

	eventManager->QueueEvent(
		std::string(eventNameBuffer.begin(), eventNameBuffer.end()),
		std::string(data.begin(), data.end()),
		fmt::sprintf("net:%d", netId)
	);
}

static std::shared_ptr<ConVar<std::string>> g_citizenDir;

static void ScanResources(fx::ServerInstanceBase* instance)
{
	auto resMan = instance->GetComponent<fx::ResourceManager>();
	auto serverResourceList = resMan->GetComponent<fx::resources::ServerResourceList>();

	std::string resourceRoot(instance->GetRootPath() + "/resources/");
	std::string systemResourceRoot(g_citizenDir->GetValue() + "/system_resources/");

	console::Printf("resources", "^2Scanning resources.^7\n");

	fx::resources::ScanResult result;
	serverResourceList->ScanResources(resourceRoot, &result);
	serverResourceList->ScanResources(systemResourceRoot, &result);

	int errorCount = 0, warningCount = 0;

	for (const auto& message : result.messages)
	{
		auto channel = fmt::sprintf("resources:%s", message.resource);

		if (message.type == fx::resources::ScanMessageType::Error)
		{
			errorCount++;
			console::PrintError(channel, "%s\n", message.Format());
		}
		else if (message.type == fx::resources::ScanMessageType::Warning)
		{
			warningCount++;
			console::PrintWarning(channel, "%s\n", message.Format());
		}
		else
		{
			console::Printf(channel, "%s\n", message.Format());
		}
	}

	if (result.reloadedResources > 0)
	{
		console::Printf("resources", "^2Found %d new resources, and refreshed %d/%d resources.^7\n", result.newResources, result.updatedResources, result.reloadedResources);
	}
	else
	{
		console::Printf("resources", "^2Found %d resources.^7\n", result.newResources);
	}

	auto quickPlural = [](int number, std::string_view singular, std::string_view plural)
	{
		if (number == 1)
		{
			return fmt::sprintf("%d %s", number, singular);
		}
		else
		{
			return fmt::sprintf("%d %s", number, plural);
		}
	};

	if (errorCount > 0 && warningCount > 0)
	{
		console::Printf("resources",
			"^1%s and %s were encountered.^7\n",
			quickPlural(warningCount, "warning", "warnings"),
			quickPlural(errorCount, "error", "errors"));
	}
	else if (errorCount > 0)
	{
		console::Printf("resources", "^1%s encountered.^7\n", quickPlural(errorCount, "error was", "errors were"));
	}
	else if (warningCount > 0)
	{
		console::Printf("resources", "^3%s encountered.^7\n", quickPlural(warningCount, "warning was", "warnings were"));
	}

	// mount discovered resources for later use in VFS (e.g. from `exec`)
	for (const auto& resource : result.resources)
	{
		auto mountPath = fmt::sprintf("@%s/", resource->GetName());

		fwRefContainer<vfs::RelativeDevice> relativeDevice = new vfs::RelativeDevice(resource->GetPath() + "/");
		vfs::Unmount(mountPath);
		vfs::Mount(relativeDevice, mountPath);
	}

	// reset rate limiters for clients downloading stuff in bursts
	auto trl = instance->GetComponent<fx::TokenRateLimiter>();
	trl->Update(1.0, std::max(double(result.newResources + result.reloadedResources), 3.0));

	/*NETEV onResourceListRefresh SERVER
	/#*
	 * A server-side event triggered when the `refresh` command completes.
	 #/
	declare function onResourceListRefresh(): void;
	*/
	instance
		->GetComponent<fx::ResourceManager>()
		->GetComponent<fx::ResourceEventManagerComponent>()
		->TriggerEvent2("onResourceListRefresh", {});
}

static class : public fx::EventReassemblySink
{
public:
	virtual void SendPacket(int target, std::string_view packet) override
	{
		auto client = instance->GetComponent<fx::ClientRegistry>()->GetClientByNetID(target);

		if (client)
		{
			net::Buffer outPacket;
			outPacket.Write(HashRageString("msgReassembledEvent"));
			outPacket.Write(packet.data(), packet.size());

			client->SendPacket(1, outPacket);
		}
	}

	virtual bool LimitEvent(int source) override
	{
		static fx::RateLimiterStore<uint32_t, false> netEventRateLimiterStore{ instance->GetComponent<console::Context>().GetRef() };
		static auto netEventRateLimiter = netEventRateLimiterStore.GetRateLimiter("netEvent", fx::RateLimiterDefaults{ 50.f, 200.f });
		static auto netFloodRateLimiter = netEventRateLimiterStore.GetRateLimiter("netEventFlood", fx::RateLimiterDefaults{ 75.f, 300.f });

		if (!netEventRateLimiter->Consume(source))
		{
			if (!netFloodRateLimiter->Consume(source))
			{
				gscomms_execute_callback_on_main_thread([this, source]()
				{
					auto client = instance->GetComponent<fx::ClientRegistry>()->GetClientByNetID(source);

					if (client)
					{
						instance->GetComponent<fx::GameServer>()->DropClient(client, "Unreliable network event overflow.");
					}
				}, true);
			}

			return true;
		}

		return false;
	}

public:
	fx::ServerInstanceBase* instance;
} g_reassemblySink;

inline std::string ToNarrow(const std::string& str)
{
	return str;
}

std::shared_mutex g_resourceStartOrderLock;
std::list<std::string> g_resourceStartOrder;

extern fwRefContainer<fx::ResourceMounter> MakeServerResourceMounter(const fwRefContainer<fx::ResourceManager>& resman);

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		{
			g_citizenDir = instance->AddVariable<std::string>("citizen_dir", ConVar_None, ToNarrow(MakeRelativeCitPath(L"citizen")));

			// create cache directory if needed
			auto device = vfs::GetDevice(instance->GetRootPath());
			auto cacheDir = instance->GetRootPath() + "/cache/";

			if (device.GetRef())
			{
				device->CreateDirectory(cacheDir);

				// precreate cache/files/ so that later components won't have to
				device->CreateDirectory(cacheDir + "files/");
			}

			vfs::Mount(new vfs::RelativeDevice(g_citizenDir->GetValue() + "/"), "citizen:/");
			vfs::Mount(new vfs::RelativeDevice(cacheDir), "cache:/");
		}

		instance->SetComponent(fx::CreateResourceManager());
		instance->SetComponent(new fx::ServerEventComponent());
		instance->SetComponent(new fx::TokenRateLimiter(1.0f, 3.0f));

		fwRefContainer<fx::ResourceManager> resman = instance->GetComponent<fx::ResourceManager>();
		resman->SetComponent(new fx::ServerInstanceBaseRef(instance));
		resman->SetComponent(instance->GetComponent<console::Context>());
		resman->SetComponent(fx::EventReassemblyComponent::Create());

		{
			auto concom = resman->GetComponent<fx::ResourceManagerConstraintsComponent>();
			concom->SetEnforcingConstraints(true);

			std::string serverVersion = GIT_TAG;
			constexpr const auto prefixLen = std::string_view{ "v1.0.0." }.length();

			concom->RegisterConstraint("server", atoi((serverVersion.length() > prefixLen) ? (serverVersion.c_str() + prefixLen) : "99999"));
			concom->RegisterConstraint("onesync", [instance](std::string_view onesyncCheck, std::string* error) -> bool
			{
				if (fx::IsOneSync())
				{
					return true;
				}

				if (error)
				{
					*error = "OneSync needs to be enabled";
				}

				return false;
			});

			concom->RegisterConstraint("gameBuild", [instance](std::string_view gameBuildCheck, std::string* error) -> bool
			{
				auto enforcedBuild = fx::GetEnforcedGameBuildNumber();

				if (enforcedBuild)
				{
					fx::GameBuild minBuild;

					if (ConsoleArgumentType<fx::GameBuild>::Parse(std::string(gameBuildCheck), &minBuild))
					{
						int minBuildNum = atoi(minBuild.c_str());

						if (enforcedBuild >= minBuildNum)
						{
							return true;
						}
						else
						{
							if (error)
							{
								*error = fmt::sprintf("sv_enforceGameBuild needs to be at least %d (current is %d)", minBuildNum, enforcedBuild);
							}
						}
					}
				}

				return false;
			});

			concom->RegisterConstraint("native", [](std::string_view nativeHash, std::string* error)
			{
				if (nativeHash.length() > 2 && nativeHash.substr(0, 2) == "0x")
				{
					uint64_t nativeNum = strtoull(std::string(nativeHash.substr(2)).c_str(), nullptr, 16);

					if (fx::ScriptEngine::GetNativeHandler(nativeNum))
					{
						return true;
					}

					if (error)
					{
						*error = fmt::sprintf("native 0x%X isn't supported", nativeNum);
					}
				}

				return false;
			});
		}

		// TODO: not instanceable
		auto rac = resman->GetComponent<fx::EventReassemblyComponent>();

		instance
			->GetComponent<fx::GameServer>()
			->GetComponent<fx::HandlerMapComponent>()
			->Add(HashRageString("msgReassembledEvent"), [rac](const fx::ClientSharedPtr& client, net::Buffer& buffer)
			{
				rac->HandlePacket(client->GetNetId(), std::string_view{ (char*)(buffer.GetBuffer() + buffer.GetCurOffset()), buffer.GetRemainingBytes() });
			});

		g_reassemblySink.instance = instance;
		rac->SetSink(&g_reassemblySink);

		instance->GetComponent<fx::ClientRegistry>()->OnClientCreated.Connect([rac](const fx::ClientSharedPtr& client)
		{
			fx::Client* unsafeClient = client.get();
			unsafeClient->OnAssignNetId.Connect([rac, unsafeClient]()
			{
				if (unsafeClient->GetNetId() < 0xFFFF)
				{
					rac->RegisterTarget(unsafeClient->GetNetId());
					
					unsafeClient->OnDrop.Connect([rac, unsafeClient]()
					{
						rac->UnregisterTarget(unsafeClient->GetNetId());
					});
				}
			});
		});

		instance->GetComponent<fx::GameServer>()->OnNetworkTick.Connect([rac]()
		{
			rac->NetworkTick();
		});

		resman->AddMounter(MakeServerResourceMounter(resman));
		resman->SetComponent(new fx::resources::ServerResourceList);

		fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
		{
			fx::ServerInstanceBase* instance = resource->GetManager()->GetComponent<fx::ServerInstanceBaseRef>()->Get();
			
			// resource game filtering
			resource->OnStart.Connect([instance, resource]()
			{
				auto gameServer = instance->GetComponent<fx::GameServer>();
				auto md = resource->GetComponent<fx::ResourceMetaDataComponent>();
				auto games = md->GetEntries("game");

				bool allowed = true;

				// store the game name
				std::string gameNameString;
				switch (gameServer->GetGameName())
				{
				case fx::GameName::GTA4:
					gameNameString = "gta4";
					break;
				case fx::GameName::GTA5:
					gameNameString = "gta5";
					break;
				case fx::GameName::RDR3:
					gameNameString = "rdr3";
					break;
				default:
					gameNameString = "unknown";
				}

				// if is FXv2
				auto isCfxV2 = md->GetEntries("is_cfxv2");

				if (isCfxV2.begin() != isCfxV2.end())
				{
					// validate game name
					std::set<std::string> gameSet;

					for (const auto& game : games)
					{
						gameSet.insert(game.second);
					}

					bool isCommon = (gameSet.find("common") != gameSet.end());
					bool isGame = (gameSet.find(gameNameString) != gameSet.end());

					if (isCommon && isGame)
					{
						console::PrintWarning("resources", "Resource %s specifies both `common` and a specific game. This is considered ill-formed.\n", resource->GetName());
						allowed = false;
					}
					else if (!isCommon && !isGame)
					{
						console::PrintWarning("resources", "Resource %s does not support the current game (%s).\n", resource->GetName(), gameNameString);
						allowed = false;
					}

					if (!*md->IsManifestVersionBetween("adamant", ""))
					{
						console::PrintWarning("resources", "Resource %s does not specify an `fx_version` in fxmanifest.lua.\n", resource->GetName());
						allowed = false;
					}

					if (isGame && gameNameString == "rdr3")
					{
						auto warningEntries = md->GetEntries("rdr3_warning");

						static const std::string warningString = "I acknowledge that this is a prerelease build of RedM, and I am aware my resources *will* become incompatible once RedM ships.";

						if (warningEntries.begin() == warningEntries.end() || warningEntries.begin()->second != warningString)
						{
							console::PrintWarning("resources", "Resource %s does not contain the RedM pre-release warning in fxmanifest.lua.\nPlease add ^2rdr3_warning '%s'^3 to fxmanifest.lua in this resource.\n", resource->GetName(), warningString);
							allowed = false;
						}
					}
				}
				else
				{
					// allow non-FXv2 for GTA5 only
					if (gameNameString != "gta5")
					{
						console::PrintWarning("resources", "Resource %s is not using CitizenFXv2 manifest. This is not allowed for the current game (%s).\n", resource->GetName(), gameNameString);
						allowed = false;
					}
				}

				return allowed;
			}, INT32_MIN);

			resource->OnStart.Connect([=]()
			{
				trace("Started resource %s\n", resource->GetName());

				auto metaData = resource->GetComponent<fx::ResourceMetaDataComponent>();
				auto iv = metaData->GetEntries("server_only");

				if (iv.begin() != iv.end())
				{
					return;
				}

				{
					std::unique_lock<std::shared_mutex> _(g_resourceStartOrderLock);
					g_resourceStartOrder.push_back(resource->GetName());
				}

				auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();
				auto trl = instance->GetComponent<fx::TokenRateLimiter>();

				net::Buffer outBuffer;
				outBuffer.Write(HashRageString("msgResStart"));
				outBuffer.Write(resource->GetName().c_str(), resource->GetName().length());

				clientRegistry->ForAllClients([&](const fx::ClientSharedPtr& client)
				{
					client->SendPacket(0, outBuffer, NetPacketType_Reliable);

					trl->ReturnToken(client->GetConnectionToken());
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

				{
					std::unique_lock<std::shared_mutex> _(g_resourceStartOrderLock);
					g_resourceStartOrder.erase(std::remove_if(g_resourceStartOrder.begin(), g_resourceStartOrder.end(), [&resource](const std::string& name)
											   {
												   return name == resource->GetName();
											   }),
					g_resourceStartOrder.end());
				}

				auto clientRegistry = instance->GetComponent<fx::ClientRegistry>();

				net::Buffer outBuffer;
				outBuffer.Write(HashRageString("msgResStop"));
				outBuffer.Write(resource->GetName().c_str(), resource->GetName().length());

				clientRegistry->ForAllClients([&](const fx::ClientSharedPtr& client)
				{
					client->SendPacket(0, outBuffer, NetPacketType_Reliable);
				});
			}, -1000);
		});

		ScanResources(instance);

		static auto isCategory = [](std::string_view resourceName)
		{
			return (!resourceName.empty() && resourceName[0] == '[' && resourceName[resourceName.size() - 1] == ']');
		};

		static auto findByComponent = [resman](const std::string& resourceName) -> std::set<std::string>
		{
			auto resourceList = resman->GetComponent<fx::resources::ServerResourceList>();
			auto resources = resourceList->FindByPathComponent(resourceName);

			if (resources.empty())
			{
				trace("^3Couldn't find resource category %s.^7\n", resourceName);
			}

			return resources;
		};

		static auto commandRef = instance->AddCommand("start", [=](const std::string& resourceName)
		{
			if (resourceName.empty())
			{
				return;
			}

			if (isCategory(resourceName))
			{
				for (const auto& resource : findByComponent(resourceName))
				{
					auto conCtx = instance->GetComponent<console::Context>();
					conCtx->ExecuteSingleCommandDirect(ProgramArguments{ "start", resource });
				}

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

			if (isCategory(resourceName))
			{
				for (const auto& resource : findByComponent(resourceName))
				{
					auto conCtx = instance->GetComponent<console::Context>();
					conCtx->ExecuteSingleCommandDirect(ProgramArguments{ "stop", resource });
				}

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

		static bool configured = false;

		static auto ensureCommandRef = instance->AddCommand("ensure", [=](const std::string& resourceName)
		{
			auto doEnsure = [&](const std::string& resourceName)
			{
				auto resource = resman->GetResource(resourceName);

				if (!resource.GetRef())
				{
					trace("^3Couldn't find resource %s.^7\n", resourceName);
					return false;
				}

				auto conCtx = instance->GetComponent<console::Context>();

				// don't allow `ensure` to restart a resource if we're still configuring (e.g. executing a startup script)
				// this'll lead to issues when, say, the following script runs:
				//     ensure res2
				//     ensure res1
				//
				// if res2 depends on res1, res1 restarting will lead to res2 stopping but not being started again
				// #TODO: restarting behavior of stopped dependencies at runtime
				if (configured && resource->GetState() == fx::ResourceState::Started)
				{
					conCtx->ExecuteSingleCommandDirect(ProgramArguments{ "stop", resourceName });
				}

				conCtx->ExecuteSingleCommandDirect(ProgramArguments{ "start", resourceName });

				return true;
			};

			if (isCategory(resourceName))
			{
				for (const auto& resource : findByComponent(resourceName))
				{
					if (!doEnsure(resource))
					{
						// break out of loop if failed
						break;
					}
				}

				return;
			}

			// if not a category, just plain ensure
			doEnsure(resourceName);
		});

		instance->OnInitialConfiguration.Connect([]()
		{
			configured = true;
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

		static std::string rawCommand;

		instance->GetComponent<console::Context>()->GetCommandManager()->FallbackEvent.Connect([=](const std::string& commandName, const ProgramArguments& arguments, const std::string& context)
		{
			if (!context.empty())
			{
				auto eventComponent = resman->GetComponent<fx::ResourceEventManagerComponent>();

				try
				{
					return eventComponent->TriggerEvent2("__cfx_internal:commandFallback", { "internal-net:" + context }, rawCommand);
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

		gameServer->GetComponent<fx::HandlerMapComponent>()->Add(HashRageString("msgServerCommand"), [=](const fx::ClientSharedPtr& client, net::Buffer& buffer)
		{
			static fx::RateLimiterStore<uint32_t, false> netEventRateLimiterStore{ instance->GetComponent<console::Context>().GetRef() };
			static auto netEventRateLimiter = netEventRateLimiterStore.GetRateLimiter("netCommand", fx::RateLimiterDefaults{ 7.f, 14.f });
			static auto netFloodRateLimiter = netEventRateLimiterStore.GetRateLimiter("netCommandFlood", fx::RateLimiterDefaults{ 25.f, 45.f });

			uint32_t netId = client->GetNetId();

			if (!netEventRateLimiter->Consume(netId))
			{
				if (!netFloodRateLimiter->Consume(netId))
				{
					gscomms_execute_callback_on_main_thread([client, instance]()
					{
						instance->GetComponent<fx::GameServer>()->DropClient(client, "Reliable server command overflow.");
					});
				}

				return;
			}

			auto cmdLen = buffer.Read<uint16_t>();

			std::vector<char> cmd(cmdLen);
			buffer.Read(cmd.data(), cmdLen);

			std::string printString;

			fx::PrintListenerContext context([&printString](std::string_view print)
			{
				printString += print;
			});

			fx::PrintFilterContext filterContext([&client](ConsoleChannel& channel, std::string_view print)
			{
				channel = fmt::sprintf("forward:%d/%s", client->GetNetId(), channel);
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
			client->SendPacket(0, outBuffer, NetPacketType_Reliable);
		}
	}
	else
	{
		clientRegistry->ForAllClients([&](const fx::ClientSharedPtr& client)
		{
			client->SendPacket(0, outBuffer, NetPacketType_Reliable);
		});
	}
}

static InitFunction initFunction2([]()
{
	fx::ScriptEngine::RegisterNativeHandler("PRINT_STRUCTURED_TRACE", [](fx::ScriptContext& context)
	{
		std::string_view jsonData = context.CheckArgument<const char*>(0);

		try
		{
			auto j = nlohmann::json::parse(jsonData);

			auto resourceName = nlohmann::json(nullptr);
			fx::OMPtr<IScriptRuntime> runtime;

			if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
			{
				fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

				if (resource)
				{
					resourceName = resource->GetName();
				}
			}

			StructuredTrace({ "type", "script_structured_trace" }, { "payload", j }, { "resource", resourceName });
		}
		catch (std::exception& e)
		{

		}
	});

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

	fx::ScriptEngine::RegisterNativeHandler("TRIGGER_LATENT_CLIENT_EVENT_INTERNAL", [](fx::ScriptContext& context)
	{
		std::string eventName = context.CheckArgument<const char*>(0);
		auto targetSrcIdx = context.CheckArgument<const char*>(1);

		const void* data = context.GetArgument<const void*>(2);
		uint32_t dataLen = context.GetArgument<uint32_t>(3);

		int bps = context.GetArgument<int>(4);

		// get the current resource manager
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// get the owning server instance
		auto rac = resourceManager->GetComponent<fx::EventReassemblyComponent>();

		rac->TriggerEvent(std::stoi(targetSrcIdx), std::string_view{ eventName.c_str(), eventName.size() + 1 }, std::string_view{ reinterpret_cast<const char*>(data), dataLen }, bps);
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
			fx::OMPtr<IScriptRuntime> runtime;
			std::string currentResourceName;

			if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
			{
				currentResourceName = reinterpret_cast<fx::Resource*>(runtime->GetParentObject())->GetName();
			}

			if (g_managedResources.find(resource->GetName()) != g_managedResources.end() || resource->GetName() == currentResourceName)
			{
				success = false;
			}
			else
			{
				success = resource->Stop();
			}
		}

		context.SetResult(success);
	});

	fx::ScriptEngine::RegisterNativeHandler("SCHEDULE_RESOURCE_TICK", [](fx::ScriptContext& context)
	{
		auto resourceManager = fx::ResourceManager::GetCurrent();

		// #TODOMONITOR: make helper
		auto monitorVar = resourceManager->GetComponent<fx::ServerInstanceBaseRef>()->Get()->GetComponent<console::Context>()->GetVariableManager()->FindEntryRaw("monitorMode");

		if (monitorVar && monitorVar->GetValue() != "0")
		{
			return;
		}

		fwRefContainer<fx::Resource> resource = resourceManager->GetResource(context.CheckArgument<const char*>(0));

		gscomms_execute_callback_on_main_thread([resource]()
		{
			if (resource.GetRef())
			{
				resource->GetManager()->MakeCurrent();

				// #TODOTICKLESS: handle bookmark-based resources
				resource->GetComponent<fx::ResourceScriptingComponent>()->Tick();
			}
		}, true);
	});

	fx::ScriptEngine::RegisterNativeHandler("REGISTER_RESOURCE_ASSET", [](fx::ScriptContext& context)
	{
		auto resourceManager = fx::ResourceManager::GetCurrent();
		fwRefContainer<fx::Resource> resource = resourceManager->GetResource(context.CheckArgument<const char*>(0));

		if (!resource.GetRef())
		{
			throw std::runtime_error("Invalid resource name passed to REGISTER_RESOURCE_ASSET.");
		}

		auto diskName = context.CheckArgument<const char*>(1);

		auto streamComponent = resource->GetComponent<fx::ResourceStreamComponent>();
		auto sf = streamComponent->AddStreamingFile(resource->GetPath() + "/" + diskName);

		if (!sf)
		{
			throw std::runtime_error("Failed to register streaming file in REGISTER_RESOURCE_ASSET.");
		}

		static std::string tempStr;
		tempStr = sf->GetCacheString();
		
		context.SetResult<const char*>(tempStr.c_str());
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
			auto value = var->GetValue();

			if (value == "true")
			{
				value = "1";
			}

			context.SetResult(atoi(value.c_str()));
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
