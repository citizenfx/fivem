/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include <StdInc.h>
#include <ResourceManager.h>
#include <ResourceEventComponent.h>
#include <EventReassemblyComponent.h>

#include "fxScripting.h"
#include "Resource.h"

#include <mutex>

#include <ScriptEngine.h>

#include <CachedResourceMounter.h>
#include <CachedResourceMounterWrap.h>
#include <HttpClient.h>

#include <NetLibrary.h>

#include <nutsnbolts.h>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/error/error.h>

#include <boost/algorithm/string.hpp>

#include <skyr/percent_encode.hpp>
#include <skyr/url.hpp>

#include <CoreConsole.h>

#include <Error.h>

#include <ICoreGameInit.h>

#include <ResourceGameLifetimeEvents.h>

#include <tbb/concurrent_queue.h>

#include <pplawait.h>
#include <experimental/resumable>

#include <json.hpp>

static tbb::concurrent_queue<std::function<void()>> executeNextGameFrame;
static NetAddress g_netAddress;

static std::mutex g_resourceStartRequestMutex;
static std::set<std::string> g_resourceStartRequestSet;

using ResultTuple = std::tuple<tl::expected<fwRefContainer<fx::Resource>, fx::ResourceManagerError>, std::string>;

static std::string CrackResourceName(const std::string& uri)
{
	auto parsed = skyr::make_url(uri);

	if (parsed)
	{
		if (!parsed->host().empty())
		{
			return parsed->host();
		}
	}

	assert(!"Should not be reached.");
	return "MISSING";
}

// helper to UrlEncode for server releases that support this
static auto UrlEncodeWrap(const std::string& base, const std::string& str)
{
	auto baseUrl = skyr::make_url(base);

	if (baseUrl)
	{
		std::string strCopy(str);
		boost::algorithm::replace_all(strCopy, "+", "%2B");

		auto url = skyr::make_url(strCopy, *baseUrl);

		if (url)
		{
			return url->href();
		}
	}

	return base + str;
};

class NetLibraryResourcesComponent : public fwRefCountable, public fx::IAttached<NetLibrary>
{
public:
	virtual void AttachToObject(NetLibrary* netLibrary) override;

private:
	void UpdateResources(const std::string& updateList, const std::function<void()>& doneCb);

	void UpdateOneResource();

	bool RequestResourceFileSet(fx::Resource* resource, const std::string& setName);

private:
	std::queue<std::string> m_resourceUpdateQueue;

	NetLibrary* m_netLibrary;
};

DECLARE_INSTANCE_TYPE(NetLibraryResourcesComponent);

static std::mutex progressMutex;
static std::optional<std::tuple<std::string, int, int, bool>> nextProgress;
static pplx::cancellation_token_source cts;

static void ThrottledConnectionProgress(const std::string& string, int count, int total, bool cancelable)
{
	std::lock_guard<std::mutex> _(progressMutex);
	nextProgress = { string, count, total, cancelable };
}

static pplx::task<std::vector<ResultTuple>> DownloadResources(std::vector<std::string> requiredResources, NetLibrary* netLibrary)
{
	struct ProgressData
	{
		std::atomic<int> current;
		int total;
	};

	fx::ResourceManager* manager = Instance<fx::ResourceManager>::Get();

	std::vector<ResultTuple> list;

	auto progressCounter = std::make_shared<ProgressData>();
	progressCounter->current = 0;
	progressCounter->total = requiredResources.size();

	for (auto& resourceUri : requiredResources)
	{
		if (cts.get_token().is_canceled())
		{
			break;
		}
		auto resourceName = CrackResourceName(resourceUri);

		{
			fwRefContainer<fx::Resource> oldResource = manager->GetResource(resourceName);

			if (oldResource.GetRef())
			{
				// removing resources should happen on the main thread to prevent deadlocks in odd ScRTs
				static auto icgi = Instance<ICoreGameInit>::Get();
				if (icgi->GetGameLoaded())
				{
					pplx::task_completion_event<void> tce;

					executeNextGameFrame.push([tce, manager, oldResource]()
					{
						manager->RemoveResource(oldResource);

						tce.set();
					});

					co_await pplx::task<void>{ tce };
				}
				else
				{
					manager->RemoveResource(oldResource);
				}
			}
		}

		auto mounterRef = manager->GetMounterForUri(resourceUri);
		static_cast<fx::CachedResourceMounter*>(mounterRef.GetRef())->AddStatusCallback(resourceName, [resourceName, progressCounter](fx::CachedResourceMounter::StatusType statusType, int downloadCurrent, int downloadTotal)
		{
			if (cts.get_token().is_canceled())
			{
				return;
			}
			std::string_view statusTypeString = (statusType == fx::CachedResourceMounter::StatusType::Downloading) ? "Downloading" : "Verifying";

			ThrottledConnectionProgress(fmt::sprintf("%s %s (%d of %d - %.2f/%.2f MiB)", statusTypeString, resourceName, progressCounter->current, progressCounter->total,
				downloadCurrent / 1024.0f / 1024.0f, downloadTotal / 1024.0f / 1024.0f), progressCounter->current, progressCounter->total, true);
		});

		auto resource = co_await manager->AddResourceWithError(resourceUri);

		if (cts.get_token().is_canceled())
		{
			break;
		}
		// report progress
		int currentCount = progressCounter->current.fetch_add(1) + 1;
		ThrottledConnectionProgress(fmt::sprintf("Mounted %s (%d of %d)", resourceName, currentCount, progressCounter->total), currentCount, progressCounter->total, true);

		// return tuple
		list.emplace_back(resource, resourceName);
	}

	co_return list;
}

void NetLibraryResourcesComponent::UpdateOneResource()
{
	if (!m_resourceUpdateQueue.empty())
	{
		auto resource = m_resourceUpdateQueue.front();
		m_resourceUpdateQueue.pop();

		UpdateResources(resource, [this, resource]()
		{
			std::lock_guard _(g_resourceStartRequestMutex);
			g_resourceStartRequestSet.erase(resource);

			executeNextGameFrame.push([this]()
			{
				UpdateOneResource();
			});
		});
	}
}

void NetLibraryResourcesComponent::UpdateResources(const std::string& updateList, const std::function<void()>& doneCb)
{
	NetAddress address = g_netAddress;

	// fetch configuration
	HttpClient* httpClient = Instance<HttpClient>::Get();

	// build request
	std::map<std::string, std::string> postMap;
	postMap["method"] = "getConfiguration";

	if (!updateList.empty())
	{
		postMap["resources"] = updateList;
	}

	HttpRequestOptions options;

	std::string connectionToken;
	if (Instance<ICoreGameInit>::Get()->GetData("connectionToken", &connectionToken))
	{
		options.headers["X-CitizenFX-Token"] = connectionToken;
	}

	std::string addressAddress = address.GetAddress();
	uint32_t addressPort = address.GetPort();

	auto curServerUrl = fmt::sprintf("https://%s/", m_netLibrary->GetCurrentPeer().ToString());

	// #TODO: remove this once server version with `18d5259f60dd203b5705130491ddda4e95665171` becomes mandatory
	auto curServerUrlNonTls = fmt::sprintf("http://%s/", m_netLibrary->GetCurrentPeer().ToString());

	options.progressCallback = [](const ProgressInfo& progress)
	{
		if (progress.downloadTotal != 0)
		{
			ThrottledConnectionProgress(fmt::sprintf("Downloading content manifest (%.2f/%.2f kB)",
										progress.downloadNow / 1000.0, progress.downloadTotal / 1000.0),
			0, 1, false);
		}
		else if (progress.downloadNow != 0)
		{
			ThrottledConnectionProgress(fmt::sprintf("Downloading content manifest (%.2f kB)",
										progress.downloadNow / 1000.0),
			0, 1, false);
		}
	};

	ThrottledConnectionProgress("Downloading content manifest...", 0, 1, false);

	httpClient->DoPostRequest(fmt::sprintf("%sclient", curServerUrlNonTls), httpClient->BuildPostString(postMap), options,
		[this, httpClient, address, curServerUrl, updateList, doneCb](bool result, const char* data, size_t size)
	{
		// keep a reference to the HTTP client
		auto httpClientRef = httpClient;
		auto addressClone = address; // due to non-const-safety

		// handle failure
		if (!result)
		{
			m_netLibrary->SetRichError(nlohmann::json::object({
															{ "fault", "server" },
															{ "action", "#ErrorAction_TryAgainContactOwner" },
															})
									 .dump());

			static ConVar<bool> streamerMode("ui_streamerMode", ConVar_None, false);
			std::string errorData = fmt::sprintf(" Error state: %s", std::string{ data, size });

			if (streamerMode.GetValue())
			{
				errorData = "";
			}

			GlobalError("Obtaining configuration from server failed.%s", errorData);
			return;
		}

		ThrottledConnectionProgress("Loading content manifest...", 0, 1, false);

		// 'get' the server host
		std::string serverHost = addressClone.GetAddress() + va(":%d", addressClone.GetPort());

		// start parsing the result
		rapidjson::Document node;
		node.Parse(data, size);

		if (node.HasParseError())
		{
			auto err = node.GetParseError();

			m_netLibrary->SetRichError(nlohmann::json::object({
															{ "fault", "server" },
															{ "action", "#ErrorAction_TryAgainContactOwner" },
															})
									 .dump());

			trace("Failed to parse content manifest:\n%s\nError code: %s (offset: %d)\n", data, rapidjson::GetParseError_En(err), node.GetErrorOffset());
			GlobalError("Failed to parse content manifest: %s (at offset %d) - see the console log for details", rapidjson::GetParseError_En(err), node.GetErrorOffset());

			return;
		}

		if (!node.IsObject())
		{
			GlobalError("Obtaining configuration from server failed. JSON data was not an object.");

			return;
		}

		if (node.HasMember("error") && node["error"].IsString())
		{
			m_netLibrary->SetRichError(nlohmann::json::object({
															{ "fault", "server" },
															{ "action", "#ErrorAction_TryAgainContactOwner" },
															})
									 .dump());

			GlobalError("Obtaining configuration from server failed. Error text: %s", node["error"].GetString());

			return;
		}

		// more stuff from downloadmgr
		bool hasValidResources = true;

		if (!node.HasMember("resources") || !node["resources"].IsArray())
		{
			hasValidResources = false;
		}

		if (hasValidResources)
		{
			if (!node.HasMember("fileServer") || !node["fileServer"].IsString())
			{
				hasValidResources = false;
			}
		}

		fx::ResourceManager* manager = Instance<fx::ResourceManager>::Get();

		std::vector<std::string> requiredResources;

		manager->OnContentManifestDownloaded(node);

		fx::OnLockStreaming();

		if (hasValidResources)
		{
			auto& resources = node["resources"];

			std::string origBaseUrl = node["fileServer"].GetString();
			std::stringstream formattedResources;
			size_t formatCount = 0;

			for (auto it = resources.Begin(); it != resources.End(); it++)
			{
				auto& resource = *it;

				std::string baseUrl = origBaseUrl;

				if (it->HasMember("fileServer") && (*it)["fileServer"].IsString())
				{
					baseUrl = (*it)["fileServer"].GetString();
				}

				boost::algorithm::replace_all(baseUrl, "http://%s/", curServerUrl);
				boost::algorithm::replace_all(baseUrl, "https://%s/", curServerUrl);

				// define the resource in the mounter
				std::string resourceName = resource["name"].GetString();

				// get the mounter, first
				auto uri = "global://" + resourceName;

				{
					auto rit = resource.FindMember("uri");

					if (rit != resource.MemberEnd())
					{
						const auto& value = rit->value;

						if (value.IsString())
						{
							uri = value.GetString();
						}
					}
				}

				fwRefContainer<fx::CachedResourceMounter> mounter = manager->GetMounterForUri(uri);

				if (!mounter.GetRef())
				{
					trace("Resource URI %s has no mounter.\n", uri);
					GlobalError("Could not get resource mounter for resource %s.", resourceName);
					break;
				}

				// ok
				std::string resourceBaseUrl = fmt::sprintf("%s/%s/", baseUrl, resourceName);

				mounter->RemoveResourceEntries(resourceName);

				auto& files = resource["files"];
				for (auto i = files.MemberBegin(); i != files.MemberEnd(); i++)
				{
					fwString filename = i->name.GetString();

					mounter->AddResourceEntry(resourceName, filename, i->value.GetString(), UrlEncodeWrap(resourceBaseUrl, filename));
				}

				if (resource.HasMember("streamFiles"))
				{
					auto& streamFiles = resource["streamFiles"];

					for (auto i = streamFiles.MemberBegin(); i != streamFiles.MemberEnd(); i++)
					{
						fwString filename = i->name.GetString();
						fwString hash = i->value["hash"].GetString();

						fx::StreamingEntryData entry;

						if (i->value.HasMember("rscPagesPhysical"))
						{
							uint32_t rscPagesPhysical = i->value["rscPagesPhysical"].GetUint();
							uint32_t rscPagesVirtual = i->value["rscPagesVirtual"].GetUint();
							uint32_t rscVersion = i->value["rscVersion"].GetUint();

							entry.rscVersion = rscVersion;
							entry.rscPagesPhysical = rscPagesPhysical;
							entry.rscPagesVirtual = rscPagesVirtual;
						}
						else
						{
							entry.rscVersion = 0;
							entry.rscPagesVirtual = i->value["size"].GetUint();
							entry.rscPagesPhysical = 0;
						}

						uint32_t size = i->value["size"].GetUint();
						auto rawSize = size;

						// handle >16 MiB resources
						if (size >= (16 * 1024 * 1024))
						{
#ifdef GTA_FIVE
							// marker size (for pgStreamer) to indicate that this will be read from the file header
							size = 0xFFFFFF;
#else
							// not supported (or at least - not known how to support)
							continue;
#endif
						}

						mounter->AddResourceEntry(resourceName, filename, hash, UrlEncodeWrap(resourceBaseUrl, filename), size, {
																																{ "rscVersion", std::to_string(entry.rscVersion) },
																																{ "rscPagesPhysical", std::to_string(entry.rscPagesPhysical) },
																																{ "rscPagesVirtual", std::to_string(entry.rscPagesVirtual) },
																																{ "rawSize", std::to_string(rawSize) },
																																});

						entry.filePath = mounter->FormatPath(resourceName, filename);
						entry.resourceName = resourceName;

						fx::OnAddStreamingResource(entry);
					}
				}

				formattedResources << resourceName << " ";

				if (formatCount >= 10)
				{
					formattedResources << "\n";
					formatCount = 0;
				}

				requiredResources.push_back(uri);
			}

			trace("Required resources: %s\n", formattedResources.str());
		}

		// failure should reset the requested resource, instead
		if (requiredResources.empty() && !updateList.empty())
		{
			std::lock_guard _(g_resourceStartRequestMutex);
			g_resourceStartRequestSet.erase(updateList);
		}

		// create download task cancellation token, so we can cancel downloads if we want
		cts = pplx::cancellation_token_source();

		DownloadResources(requiredResources, m_netLibrary).then([this, doneCb](std::vector<ResultTuple> resources)
		{
			for (auto& resourceData : resources)
			{
				auto resource = std::get<0>(resourceData);

				if (!resource)
				{
					m_netLibrary->SetRichError(nlohmann::json::object({
																	{ "fault", "server" },
																	{ "action", "#ErrorAction_TryAgainContactOwner" },
																	})
											 .dump());

					GlobalError("Couldn't load resource %s: %s", std::get<std::string>(resourceData), resource.error().Get());

					executeNextGameFrame.push([]
					{
						fx::OnUnlockStreaming();
					});

					return;
				}
			}

			for (auto& resourceData : resources)
			{
				auto resource = std::get<0>(resourceData).value_or(nullptr);

				if (resource.GetRef())
				{
					std::string resourceName = resource->GetName();

					executeNextGameFrame.push([this, resource, resourceName]()
					{
						if (!resource->Start())
						{
							m_netLibrary->SetRichError(nlohmann::json::object({
																			{ "fault", "server" },
																			{ "action", "#ErrorAction_TryAgainContactOwner" },
																			})
													 .dump());

							GlobalError("Couldn't start resource %s.", resourceName.c_str());
						}
					});
				}
			}

			// mark DownloadsComplete on the next frame so all resources will have started
			{
				executeNextGameFrame.push([this]()
				{
					m_netLibrary->DownloadsComplete();
				});
			}

			executeNextGameFrame.push([]
			{
				fx::OnUnlockStreaming();
			});

			doneCb();
		},
		cts.get_token());
	});
}

bool NetLibraryResourcesComponent::RequestResourceFileSet(fx::Resource* resource, const std::string& setName)
{
	auto mounter = resource->GetComponent<fx::CachedResourceMounterWrap>();

	std::string error;
	bool result = mounter->MountOverlay(setName, &error);

	if (!error.empty())
	{
		throw std::runtime_error(va("%s", error));
	}

	return result;
}

void NetLibraryResourcesComponent::AttachToObject(NetLibrary* netLibrary)
{
	m_netLibrary = netLibrary;

	// download trigger
	netLibrary->OnInitReceived.Connect([this](NetAddress address)
	{
		g_netAddress = address;

		ThrottledConnectionProgress("Unloading content...", 0, 1, false);

		fx::ResourceManager* resourceManager = Instance<fx::ResourceManager>::Get();
		resourceManager->ResetResources();

		UpdateResources("", []() {});

		// reinit the reassembler
		auto reassembler = Instance<fx::ResourceManager>::Get()->GetComponent<fx::EventReassemblyComponent>();
		reassembler->UnregisterTarget(0);
		reassembler->RegisterTarget(0);
	});

	netLibrary->OnConnectionErrorEvent.Connect([](const char* error)
	{
		{
			std::lock_guard<std::mutex> _(progressMutex);
			nextProgress = {};
		}

		executeNextGameFrame.push([]()
		{
			fx::ResourceManager* resourceManager = Instance<fx::ResourceManager>::Get();
			resourceManager->ResetResources();
		});
	},
	-500);

	auto& earlyGameFrame =
#if defined(HAS_EARLY_GAME_FRAME)
	OnEarlyGameFrame
#else
	OnGameFrame
#endif
	;

	earlyGameFrame.Connect([this]()
	{
		std::function<void()> func;

		while (executeNextGameFrame.try_pop(func))
		{
			if (func)
			{
				func();
			}
		}

		static uint64_t lastDownloadTime;

		if ((GetTickCount64() - lastDownloadTime) > 33)
		{
			std::lock_guard<std::mutex> _(progressMutex);

			if (nextProgress)
			{
				auto [string, count, total, cancelable] = *nextProgress;
				m_netLibrary->OnConnectionProgress(string, count, total, cancelable);

				nextProgress = {};
			}

			lastDownloadTime = GetTickCount64();
		}
	});

	OnGameFrame.Connect([]()
	{
		auto reassembler = Instance<fx::ResourceManager>::Get()->GetComponent<fx::EventReassemblyComponent>();
		reassembler->NetworkTick();
	});

	netLibrary->AddReliableHandler(
	"msgReassembledEvent", [](const char* buf, size_t len)
	{
		auto reassembler = Instance<fx::ResourceManager>::Get()->GetComponent<fx::EventReassemblyComponent>();
		reassembler->HandlePacket(0, std::string_view{ buf, len });
	},
	true);

	netLibrary->AddReliableHandler("msgNetEvent", [](const char* buf, size_t len)
	{
		net::Buffer buffer(reinterpret_cast<const uint8_t*>(buf), len);

		// get the source net ID
		uint16_t sourceNetID = buffer.Read<uint16_t>();

		// get length of event name and read the event name
		static char eventName[65536];

		uint16_t nameLength = buffer.Read<uint16_t>();
		buffer.Read(eventName, nameLength);

		// read the data
		size_t dataLen = len - nameLength - (sizeof(uint16_t) * 2);
		std::vector<char> eventData(dataLen);

		buffer.Read(&eventData[0], dataLen);

		// convert the source net ID to a string
		std::string source = "net:" + std::to_string(sourceNetID);

		// get the resource manager and eventing component
		static fx::ResourceManager* resourceManager = Instance<fx::ResourceManager>::Get();
		static fwRefContainer<fx::ResourceEventManagerComponent> eventManager = resourceManager->GetComponent<fx::ResourceEventManagerComponent>();

		// and queue the event
		eventManager->QueueEvent(std::string(eventName), std::string(&eventData[0], eventData.size()), source);
	});

	netLibrary->AddReliableHandler("msgResStop", [](const char* buf, size_t len)
	{
		std::string resourceName(buf, len);

		fx::ResourceManager* resourceManager = Instance<fx::ResourceManager>::Get();
		resourceManager->MakeCurrent();

		auto resource = resourceManager->GetResource(resourceName);

		if (resource.GetRef() == nullptr)
		{
			trace("Server requested resource %s to be stopped, but we don't know that resource\n", resourceName.c_str());
			return;
		}

#if 0
			if (resource->GetState() != ResourceStateRunning)
			{
				trace("Server requested resource %s to be stopped, but it's not running\n", resourceName.c_str());
				return;
			}
#endif

		resource->Stop();
	});

	netLibrary->AddReliableHandler("msgResStart", [this](const char* buf, size_t len)
	{
		std::string resourceName(buf, len);

		fx::ResourceManager* resourceManager = Instance<fx::ResourceManager>::Get();
		resourceManager->MakeCurrent();

		auto resource = resourceManager->GetResource(resourceName);

		if (resource.GetRef() != nullptr)
		{
#if 0
				if (resource->GetState() != ResourceStateStopped)
				{
					trace("Server requested resource %s to be started, but it's not stopped\n", resourceName.c_str());
					return;
				}
#endif
		}

		std::lock_guard _(g_resourceStartRequestMutex);
		if (g_resourceStartRequestSet.find(resourceName) == g_resourceStartRequestSet.end())
		{
			g_resourceStartRequestSet.insert(resourceName);
			m_resourceUpdateQueue.push(resourceName);

			executeNextGameFrame.push([this]()
			{
				UpdateOneResource();
			});
		}
	});

	fx::ScriptEngine::RegisterNativeHandler("TRIGGER_SERVER_EVENT_INTERNAL", [netLibrary](fx::ScriptContext& context)
	{
		std::string eventName = context.GetArgument<const char*>(0);
		size_t payloadSize = context.GetArgument<uint32_t>(2);

		std::string eventPayload = std::string(context.GetArgument<const char*>(1), payloadSize);

		netLibrary->OnTriggerServerEvent(eventName, eventPayload);

		net::Buffer buffer;
		buffer.Write<uint16_t>(eventName.size() + 1);
		buffer.Write(eventName.c_str(), eventName.size() + 1);

		buffer.Write(eventPayload.c_str(), eventPayload.size());

		netLibrary->SendReliableCommand("msgServerEvent", reinterpret_cast<const char*>(buffer.GetBuffer()), buffer.GetCurOffset());
	});

	fx::ScriptEngine::RegisterNativeHandler("TRIGGER_LATENT_SERVER_EVENT_INTERNAL", [netLibrary](fx::ScriptContext& context)
	{
		std::string eventName = context.GetArgument<const char*>(0);
		size_t payloadSize = context.GetArgument<uint32_t>(2);

		std::string_view eventPayload = std::string_view(context.GetArgument<const char*>(1), payloadSize);

		int bps = context.GetArgument<int>(3);

		auto reassembler = Instance<fx::ResourceManager>::Get()->GetComponent<fx::EventReassemblyComponent>();
		reassembler->TriggerEvent(0, std::string_view{ eventName.c_str(), eventName.size() + 1 }, eventPayload, bps);
	});

	fx::ScriptEngine::RegisterNativeHandler("REQUEST_RESOURCE_FILE_SET", [this](fx::ScriptContext& context)
	{
		std::string setName = context.CheckArgument<const char*>(0);

		bool result = false;
		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());
			result = RequestResourceFileSet(resource, setName);
		}

		context.SetResult(result);
	});

	netLibrary->OnFinalizeDisconnect.Connect([](NetAddress)
	{
		// cancel download tasks
		cts.cancel();

		executeNextGameFrame.push([]()
		{
			Instance<fx::ResourceManager>::Get()->ForAllResources([](fwRefContainer<fx::Resource> resource)
			{
				resource->GetComponent<fx::ResourceGameLifetimeEvents>()->OnGameDisconnect();
			});
		});
	});

	static bool inSessionReset = false;

	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		AddCrashometry("reset_resources", "true");

		inSessionReset = true;
		Instance<fx::ResourceManager>::Get()->ResetResources();
		inSessionReset = false;
	});

	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* resman)
	{
		resman->GetComponent<fx::ResourceEventManagerComponent>()->OnTriggerEvent.Connect([](const std::string& eventName, const std::string&, const std::string&, bool*)
		{
			// onResourceStop doesn't make sense during shutdown since user scripts can't do anything of note
			// and will probably crash/otherwise error out
			if (inSessionReset && eventName == "onResourceStop")
			{
				return false;
			}

			return true;
		});
	},
	INT32_MAX);

	console::GetDefaultContext()->GetCommandManager()->FallbackEvent.Connect([netLibrary](const std::string& cmd, const ProgramArguments& args, const std::string& context)
	{
		if (netLibrary->GetConnectionState() != NetLibrary::CS_ACTIVE)
		{
			return true;
		}

		std::string s = console::GetDefaultContext()->GetCommandManager()->GetRawCommand();

		net::Buffer buffer;
		buffer.Write<uint16_t>(s.size());
		buffer.Write(s.c_str(), std::min(s.size(), static_cast<size_t>(INT16_MAX)));
		buffer.Write<uint32_t>(HashString(context.c_str()));

		netLibrary->SendReliableCommand("msgServerCommand", reinterpret_cast<const char*>(buffer.GetBuffer()), buffer.GetCurOffset());

		return false;
	},
	99999);
}

static NetLibrary* g_netLibrary;

static class : public fx::EventReassemblySink
{
	virtual void SendPacket(int target, std::string_view packet) override
	{
		g_netLibrary->SendUnreliableCommand("msgReassembledEvent", packet.data(), packet.size());
	}
} g_eventSink;

static InitFunction initFunction([]()
{
	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* manager)
	{
		manager->AddMounter(fx::GetCachedResourceMounter(manager, "rescache:/"));

		manager->SetComponent(fx::EventReassemblyComponent::Create());

		manager->GetComponent<fx::EventReassemblyComponent>()->SetSink(&g_eventSink);
	});

	NetLibrary::OnNetLibraryCreate.Connect([](NetLibrary* netLibrary)
	{
		netLibrary->SetComponent(new NetLibraryResourcesComponent);

		g_netLibrary = netLibrary;
	});
});
