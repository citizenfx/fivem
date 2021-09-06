#include "StdInc.h"

#include <ResourceManager.h>
#include <ResourceEventComponent.h>
#include <ResourceMetaDataComponent.h>
#include <ResourceManagerConstraintsComponent.h>

#include <fxScripting.h>

#include <ServerInstanceBase.h>
#include <ServerInstanceBaseRef.h>

#include <GameServer.h>
#include <ServerEventComponent.h>

#include <GameBuilds.h>

#include <RelativeDevice.h>

#include <VFSManager.h>

#include <skyr/url.hpp>
#include <skyr/percent_encode.hpp>

#include <PrintListener.h>

#include <ResourceStreamComponent.h>
#include <EventReassemblyComponent.h>

#include <KeyedRateLimiter.h>

#include <StructuredTrace.h>

#include <filesystem>

#include <ScriptEngine.h>

#include <ManifestVersion.h>
#include <cfx_version.h>

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
		auto uriParsed = skyr::make_url(uri);

		fwRefContainer<fx::Resource> resource;

		if (uriParsed)
		{
			auto pathRef = uriParsed->pathname();
			auto fragRef = uriParsed->hash().substr(1);

			if (!pathRef.empty() && !fragRef.empty())
			{
#ifdef _WIN32
				std::string pr = pathRef.substr(1);
#else
				std::string pr = pathRef;
#endif

				resource = m_manager->CreateResource(fragRef, this);
				if (!resource->LoadFrom(*skyr::percent_decode(pr)))
				{
					m_manager->RemoveResource(resource);
					resource = nullptr;
				}
			}
		}

		return pplx::task_from_result<fwRefContainer<fx::Resource>>(resource);
	}

private:
	fx::ResourceManager* m_manager;
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
static std::map<std::string, std::set<std::string>> g_resourcesByComponent;

static void ScanResources(fx::ServerInstanceBase* instance)
{
	// mapping of names to paths
	static std::map<std::string, std::string> scanData;

	auto resMan = instance->GetComponent<fx::ResourceManager>();

	std::string resourceRoot(instance->GetRootPath() + "/resources/");
	std::string systemResourceRoot(g_citizenDir->GetValue() + "/system_resources/");

	auto resourceRootPath = std::filesystem::u8path(resourceRoot).lexically_normal();
	auto systemResourceRootPath = std::filesystem::u8path(systemResourceRoot).lexically_normal();

	std::queue<std::string> pathsToIterate;
	pathsToIterate.push(systemResourceRoot);
	pathsToIterate.push(resourceRoot);

	std::vector<pplx::task<fwRefContainer<fx::Resource>>> tasks;

	// save scanned resource names so we don't scan them twice
	std::set<std::string> scannedNow;
	size_t newResources = 0;
	size_t reloadedResources = 0;

	trace("^2Scanning resources.^7\n", newResources);

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
				if (findData.name == "." || findData.name == "..")
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
					else if (scannedNow.find(findData.name) == scannedNow.end())
					{
						const auto& resourceName = findData.name;
						scannedNow.insert(resourceName);

						auto oldRes = resMan->GetResource(resourceName, false);
						auto oldScanData = scanData.find(resourceName);

						// did the path change? if so, unload the old resource
						if (oldRes.GetRef() && oldScanData != scanData.end() && oldScanData->second != resPath)
						{
							// remove from by-component lists
							for (auto& list : g_resourcesByComponent)
							{
								list.second.erase(resourceName);
							}

							// unmount relative device
							vfs::Unmount(fmt::sprintf("@%s/", resourceName));

							// stop and remove resource
							oldRes->Stop();
							resMan->RemoveResource(oldRes);

							// undo ptr
							oldRes = {};
						}

						if (oldRes.GetRef())
						{
							oldRes->GetComponent<fx::ResourceMetaDataComponent>()->LoadMetaData(resPath);
							reloadedResources++;
						}
						else
						{
							console::DPrintf("resources", "Found new resource %s in %s\n", resourceName, resPath);
							newResources++;

							auto path = std::filesystem::u8path(resPath);

							// determine which root we're relative to
							std::error_code ec;
							auto refPath = path.lexically_normal();

							std::filesystem::path* rootRef = nullptr;

							auto [relEnd, _] = std::mismatch(resourceRootPath.begin(), resourceRootPath.end(), refPath.begin());
							auto rpEnd = --resourceRootPath.end();

							if (relEnd != rpEnd)
							{
								auto [relEnd, _] = std::mismatch(systemResourceRootPath.begin(), systemResourceRootPath.end(), refPath.begin());	
								auto rpEnd = --systemResourceRootPath.end();

								if (relEnd == rpEnd)
								{
									rootRef = &systemResourceRootPath;
								}
							}
							else
							{
								rootRef = &resourceRootPath;
							}
							
							// get the relative path to the root
							std::vector<std::string> components;

							if (rootRef)
							{
								auto relPath = std::filesystem::relative(path, *rootRef, ec);

								if (!ec)
								{
									for (const auto& component : relPath)
									{
										auto name = component.filename().u8string();

										if (name[0] == '[' && name[name.size() - 1] == ']')
										{
											components.push_back(name);
										}
									}
								}
							}

							// mount the resource for later use in VFS (e.g. from `exec`)
							fwRefContainer<vfs::RelativeDevice> relativeDevice = new vfs::RelativeDevice(resPath + "/");
							vfs::Mount(relativeDevice, fmt::sprintf("@%s/", resourceName));
							scanData[resourceName] = resPath;

							skyr::url_record record;
							record.scheme = "file";

							skyr::url url{ std::move(record) };
							url.set_pathname(*skyr::percent_encode(resPath, skyr::encode_set::path));
							url.set_hash(*skyr::percent_encode(resourceName, skyr::encode_set::fragment));

							auto task = resMan->AddResource(url.href())
										.then([components = std::move(components)](fwRefContainer<fx::Resource> resource)
										{
											if (resource.GetRef())
											{
												for (const auto& component : components)
												{
													g_resourcesByComponent[component].insert(resource->GetName());
												}
											}

											return resource;
										});

							tasks.push_back(task);
						}
					}
				}
			} while (vfsDevice->FindNext(handle, &findData));

			vfsDevice->FindClose(handle);
		}
	}

	pplx::when_all(tasks.begin(), tasks.end()).wait();

	if (reloadedResources > 0)
	{
		trace("^2Found %d resources, and refreshed %d resources.^7\n", newResources, reloadedResources);
	}
	else
	{
		trace("^2Found %d resources.^7\n", newResources);
	}

	auto trl = instance->GetComponent<fx::TokenRateLimiter>();
	trl->Update(1.0, std::max(double(newResources + reloadedResources), 3.0));

	// check for outdated
	std::set<std::string> nonManifestResources;

	resMan->ForAllResources([&nonManifestResources](const fwRefContainer<fx::Resource>& resource)
	{
		auto md = resource->GetComponent<fx::ResourceMetaDataComponent>();

		auto fxV2 = md->IsManifestVersionBetween("adamant", "");
		auto fxV1 = md->IsManifestVersionBetween(ManifestVersion{ "44febabe-d386-4d18-afbe-5e627f4af937" }.guid, guid_t{ 0 });

		if (!fxV2 || !*fxV2)
		{
			if (!fxV1 || !*fxV1)
			{
				if (!md->GlobEntriesVector("client_script").empty())
				{
					nonManifestResources.insert(resource->GetName());
				}
			}
		}
	});

	if (!nonManifestResources.empty())
	{
		trace("^1Some resources have an outdated resource manifest:^7\n");

		for (auto& name : nonManifestResources)
		{
			trace("    - %s\n", name);
		}

		trace("\nPlease update these resources.\n");
	}

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

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
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

		resman->AddMounter(new LocalResourceMounter(resman.GetRef()));

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
					client->SendPacket(0, outBuffer, NetPacketType_ReliableReplayed);

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
					client->SendPacket(0, outBuffer, NetPacketType_ReliableReplayed);
				});
			}, -1000);
		});

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

		ScanResources(instance);

		static auto commandRef = instance->AddCommand("start", [=](const std::string& resourceName)
		{
			if (resourceName.empty())
			{
				return;
			}

			if (resourceName[0] == '[' && resourceName[resourceName.size() - 1] == ']')
			{
				for (const auto& resource : g_resourcesByComponent[resourceName])
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

			if (resourceName[0] == '[' && resourceName[resourceName.size() - 1] == ']')
			{
				for (const auto& resource : g_resourcesByComponent[resourceName])
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
			if (resourceName[0] == '[' && resourceName[resourceName.size() - 1] == ']')
			{
				for (const auto& resource : g_resourcesByComponent[resourceName])
				{
					auto conCtx = instance->GetComponent<console::Context>();
					conCtx->ExecuteSingleCommandDirect(ProgramArguments{ "ensure", resource });
				}

				return;
			}

			auto resource = resman->GetResource(resourceName);

			if (!resource.GetRef())
			{
				trace("^3Couldn't find resource %s.^7\n", resourceName);
				return;
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

void fx::ServerEventComponent::TriggerClientEvent(const std::string_view& eventName, const void* data, size_t dataLen, const std::optional<std::string_view>& targetSrc, bool replayed)
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
			client->SendPacket(0, outBuffer, (!replayed) ? NetPacketType_Reliable : NetPacketType_ReliableReplayed);
		}
	}
	else
	{
		clientRegistry->ForAllClients([&](const fx::ClientSharedPtr& client)
		{
			client->SendPacket(0, outBuffer, (!replayed) ? NetPacketType_Reliable : NetPacketType_ReliableReplayed);
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

			StructuredTrace({ "type", "script_structured_trace" }, { "payload", j });
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
				resource->Tick();
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
