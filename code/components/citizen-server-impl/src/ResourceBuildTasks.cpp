#include "StdInc.h"

#include <ResourceManager.h>
#include <ResourceEventComponent.h>
#include <ResourceMetaDataComponent.h>
#include <ResourceCallbackComponent.h>

#include <ServerInstanceBase.h>
#include <ServerInstanceBaseRef.h>

#include <fxScripting.h>
#include <ScriptEngine.h>

#include <FilesystemPermissions.h>

#include <chrono>
#include <cstdint>
#include <mutex>

namespace fx
{
static constexpr auto kBuildTaskFilesystemPermissionTimeout = std::chrono::minutes(30);
static constexpr auto kBuildTaskFilesystemPermissionCleanupInterval = std::chrono::seconds(30);

template<typename TCallback>
class ScopeExit
{
public:
	explicit ScopeExit(TCallback callback)
		: m_callback(std::move(callback))
	{
	}

	~ScopeExit()
	{
		if (m_active)
		{
			m_callback();
		}
	}

	void Dismiss()
	{
		m_active = false;
	}

	ScopeExit(const ScopeExit&) = delete;
	ScopeExit& operator=(const ScopeExit&) = delete;

private:
	TCallback m_callback;
	bool m_active = true;
};

template<typename TCallback>
ScopeExit(TCallback) -> ScopeExit<TCallback>;

class BuildTaskProvider
{
public:
	virtual bool ShouldBuild(const std::string& resourceName) = 0;

	virtual void Build(const std::string& resourceName, const std::function<void(bool, const std::string&)>& completionCb) = 0;
};

class ScopedBuildTaskFilesystemPermission
{
public:
	ScopedBuildTaskFilesystemPermission(const std::string& sourceResource, const std::string& targetResource, const std::vector<std::string>& allowedPaths)
		: m_sourceResource(sourceResource), m_targetResource(targetResource)
	{
		if (!m_sourceResource.empty() && !m_targetResource.empty() && m_sourceResource != m_targetResource)
		{
			m_permissionId = fx::ScriptingFilesystemPushBuildTaskPermission(m_sourceResource, m_targetResource, allowedPaths);
		}
	}

	~ScopedBuildTaskFilesystemPermission()
	{
		Release();
	}

	void Release()
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);

		if (m_permissionId == 0)
		{
			return;
		}

		fx::ScriptingFilesystemPopBuildTaskPermission(m_permissionId);
		m_permissionId = 0;
	}

	bool IsSourceResource(const std::string& resourceName) const
	{
		return m_sourceResource == resourceName;
	}

	ScopedBuildTaskFilesystemPermission(const ScopedBuildTaskFilesystemPermission&) = delete;
	ScopedBuildTaskFilesystemPermission& operator=(const ScopedBuildTaskFilesystemPermission&) = delete;

private:
	std::string m_sourceResource;
	std::string m_targetResource;
	std::recursive_mutex m_mutex;
	uint64_t m_permissionId = 0;
};

struct BuildTaskCompletionState
{
	std::recursive_mutex mutex;
	bool completed = false;
};

struct BuildMap : public fwRefCountable
{
	~BuildMap()
	{
		ClearActiveBuilds("Build task target resource was destroyed before invoking its completion callback.");
	}

	void AddActiveBuild(const std::shared_ptr<ScopedBuildTaskFilesystemPermission>& permission, const std::function<bool()>& cancelCb, const std::function<void(const std::string&)>& failureCb)
	{
		std::lock_guard<std::recursive_mutex> lock(m_activeBuildsMutex);
		activeBuilds.push_back({
			permission,
			std::chrono::steady_clock::now() + kBuildTaskFilesystemPermissionTimeout,
			cancelCb,
			failureCb
		});
	}

	void ReleaseActiveBuild(const std::shared_ptr<ScopedBuildTaskFilesystemPermission>& permission)
	{
		std::lock_guard<std::recursive_mutex> lock(m_activeBuildsMutex);

		for (auto it = activeBuilds.begin(); it != activeBuilds.end(); ++it)
		{
			if (it->permission == permission)
			{
				it->permission->Release();
				activeBuilds.erase(it);
				return;
			}
		}

		permission->Release();
	}

	void ClearActiveBuilds(const std::string& failureReason)
	{
		std::vector<std::function<void(const std::string&)>> cleanupCallbacks;

		{
			std::lock_guard<std::recursive_mutex> lock(m_activeBuildsMutex);

			for (auto& activeBuild : activeBuilds)
			{
				if (!activeBuild.cancelCb())
				{
					continue;
				}

				activeBuild.permission->Release();
				cleanupCallbacks.push_back(activeBuild.failureCb);
			}

			activeBuilds.clear();
		}

		for (const auto& cleanupCb : cleanupCallbacks)
		{
			cleanupCb(failureReason);
		}
	}

	void ClearActiveBuildsForSource(const std::string& sourceResource)
	{
		std::vector<std::function<void(const std::string&)>> cleanupCallbacks;

		{
			std::lock_guard<std::recursive_mutex> lock(m_activeBuildsMutex);

			for (auto it = activeBuilds.begin(); it != activeBuilds.end();)
			{
				if (it->permission->IsSourceResource(sourceResource))
				{
					if (!it->cancelCb())
					{
						it = activeBuilds.erase(it);
						continue;
					}

					it->permission->Release();
					cleanupCallbacks.push_back(it->failureCb);
					it = activeBuilds.erase(it);
					continue;
				}

				++it;
			}
		}

		for (const auto& cleanupCb : cleanupCallbacks)
		{
			cleanupCb("Build task provider resource stopped before invoking its completion callback.");
		}
	}

	void ReleaseExpiredBuilds(const std::string& targetResource)
	{
		std::vector<std::function<void(const std::string&)>> timeoutCallbacks;

		{
			std::lock_guard<std::recursive_mutex> lock(m_activeBuildsMutex);

			const auto now = std::chrono::steady_clock::now();
			for (auto it = activeBuilds.begin(); it != activeBuilds.end();)
			{
				if (it->timeout <= now)
				{
					trace("Build task for resource %s expired after build timeout.\n", targetResource.c_str());
					if (!it->cancelCb())
					{
						it = activeBuilds.erase(it);
						continue;
					}

					it->permission->Release();
					timeoutCallbacks.push_back(it->failureCb);
					it = activeBuilds.erase(it);
					continue;
				}

				++it;
			}
		}

		for (const auto& timeoutCb : timeoutCallbacks)
		{
			timeoutCb("Build task timed out before invoking its completion callback.");
		}
	}

	std::vector<std::shared_ptr<BuildTaskProvider>> buildingProviders;

private:
	struct ActiveBuild
	{
		std::shared_ptr<ScopedBuildTaskFilesystemPermission> permission;
		std::chrono::steady_clock::time_point timeout;
		std::function<bool()> cancelCb;
		std::function<void(const std::string&)> failureCb;
	};

	std::recursive_mutex m_activeBuildsMutex;
	std::vector<ActiveBuild> activeBuilds;
};

static std::string TrimBuildTaskPermissionEntry(const std::string& entry)
{
	const auto start = entry.find_first_not_of(" \t\r\n");
	if (start == std::string::npos)
	{
		return {};
	}

	const auto end = entry.find_last_not_of(" \t\r\n");
	return entry.substr(start, end - start + 1);
}

bool ParseBuildTaskPermissionEntry(const std::string& entry, const std::string& builderResourceName, std::string* allowedPath)
{
	if (!allowedPath)
	{
		return false;
	}

	const auto value = TrimBuildTaskPermissionEntry(entry);
	if (value.empty())
	{
		return false;
	}

	const auto separator = value.find(':');
	if (separator == std::string::npos)
	{
		return false;
	}

	const auto sourceResource = TrimBuildTaskPermissionEntry(value.substr(0, separator));
	if (sourceResource != builderResourceName)
	{
		return false;
	}

	*allowedPath = TrimBuildTaskPermissionEntry(value.substr(separator + 1));
	return !allowedPath->empty() && fx::ScriptingFilesystemIsBuildTaskPathSafe(*allowedPath);
}

static bool TryParseBuildTaskPermissionEntry(const std::string& entry, const std::string& builderResourceName, std::string* allowedPath)
{
	return ParseBuildTaskPermissionEntry(entry, builderResourceName, allowedPath);
}

static void AppendBuildTaskPermissionEntries(const fwRefContainer<ResourceMetaDataComponent>& metadata, const std::string& key, const std::string& builderResourceName, std::vector<std::string>* allowedPaths)
{
	if (!metadata.GetRef())
	{
		return;
	}

	for (const auto& entry : metadata->GetEntries(key))
	{
		std::string allowedPath;
		if (TryParseBuildTaskPermissionEntry(entry.second, builderResourceName, &allowedPath))
		{
			allowedPaths->push_back(allowedPath);
		}
	}
}

static std::vector<std::string> GetBuildTaskAllowedPaths(Resource* targetResource, const std::string& builderResourceName)
{
	std::vector<std::string> allowedPaths;
	if (!targetResource)
	{
		return allowedPaths;
	}

	const auto metadata = targetResource->GetComponent<ResourceMetaDataComponent>();
	if (builderResourceName == "webpack")
	{
		// Compatibility for the built-in webpack provider. Custom providers require target opt-in.
		allowedPaths.push_back("build/");
		allowedPaths.push_back("dist/");
	}
	else if (builderResourceName == "yarn")
	{
		// Compatibility for the built-in yarn provider marker file and package output.
		allowedPaths.push_back(".yarn.installed");
		allowedPaths.push_back("node_modules/");
		allowedPaths.push_back("yarn.lock");
	}

	// Manifest syntax: build_task_permission 'builder-resource:relative/path'
	// Directory permissions must end in a slash, for example 'leap:build/'.
	AppendBuildTaskPermissionEntries(metadata, "build_task_permission", builderResourceName, &allowedPaths);

	return allowedPaths;
}

class ResourceBuildTaskProvider : public BuildTaskProvider
{
public:
	ResourceBuildTaskProvider(fx::ResourceManager* resman, const std::string& ownerResourceName, const std::map<std::string, msgpack::object>& map);

	virtual bool ShouldBuild(const std::string& resourceName) override;

	virtual void Build(const std::string& resourceName, const std::function<void(bool, const std::string&)>& completionCb) override;

private:
	fx::ResourceManager* m_resourceManager;

	std::string m_ownerResourceName;

	FunctionRef m_shouldBuildRef;

	FunctionRef m_buildRef;
};

ResourceBuildTaskProvider::ResourceBuildTaskProvider(fx::ResourceManager* resman, const std::string& ownerResourceName, const std::map<std::string, msgpack::object>& map)
	: m_resourceManager(resman)
	, m_ownerResourceName(ownerResourceName)
{
	auto setCallback = [&](const std::string& cbId) -> FunctionRef
	{
		auto it = map.find(cbId);

		if (it != map.end())
		{
			auto callback = it->second;

			if (callback.type == msgpack::type::EXT)
			{
				if (callback.via.ext.type() == 10 || callback.via.ext.type() == 11)
				{
					return FunctionRef{ std::string{callback.via.ext.data(), callback.via.ext.size } };
				}
			}
		}

		return {};
	};

	m_shouldBuildRef = std::move(setCallback("shouldBuild"));
	m_buildRef = std::move(setCallback("build"));
}

bool ResourceBuildTaskProvider::ShouldBuild(const std::string& resourceName)
{
	if (!m_shouldBuildRef)
	{
		return false;
	}

	return m_resourceManager->CallReference<bool>(m_shouldBuildRef.GetRef(), resourceName);
}

void ResourceBuildTaskProvider::Build(const std::string& resourceName, const std::function<void(bool, const std::string&)>& completionCb)
{
	if (!m_buildRef)
	{
		completionCb(false, "No build function was configured.");
		return;
	}

	fwRefContainer<BuildMap> buildMap;
	std::vector<std::string> allowedPaths;

	const fwRefContainer<Resource> resource = m_resourceManager->GetResource(resourceName);
	if (resource.GetRef())
	{
		buildMap = resource->GetComponent<BuildMap>();
		allowedPaths = GetBuildTaskAllowedPaths(resource.GetRef(), m_ownerResourceName);
	}

	if (allowedPaths.empty())
	{
		trace("Build task provider %s has no filesystem write permissions for resource %s. Add build_task_permission '%s:build/' to the target resource manifest for build outputs, and additional build_task_permission entries for any extra paths.\n", m_ownerResourceName.c_str(), resourceName.c_str(), m_ownerResourceName.c_str());
	}

	auto permission = std::make_shared<ScopedBuildTaskFilesystemPermission>(m_ownerResourceName, resourceName, allowedPaths);
	auto completionState = std::make_shared<BuildTaskCompletionState>();

	auto markBuildCompleted = [completionState]()
	{
		std::lock_guard<std::recursive_mutex> lock(completionState->mutex);
		if (completionState->completed)
		{
			return false;
		}

		completionState->completed = true;
		return true;
	};

	auto releasePermission = [permission, buildMap]() mutable
	{
		if (buildMap.GetRef())
		{
			buildMap->ReleaseActiveBuild(permission);
		}
		else
		{
			permission->Release();
		}
	};

	auto failBuild = [completionCb](const std::string& result) mutable
	{
		completionCb(false, result);
	};

	auto cancelBuild = [markBuildCompleted]() mutable
	{
		return markBuildCompleted();
	};

	auto completeBuild = [completionCb, releasePermission, markBuildCompleted](bool success, const std::string& result) mutable
	{
		if (!markBuildCompleted())
		{
			return;
		}

		releasePermission();
		completionCb(success, result);
	};

	if (buildMap.GetRef())
	{
		buildMap->AddActiveBuild(permission, cancelBuild, failBuild);
	}

	ScopeExit setupGuard(releasePermission);
	try
	{
		auto cbComponent = m_resourceManager->GetComponent<fx::ResourceCallbackComponent>();
		auto cb = cbComponent->CreateCallback([completeBuild](const msgpack::unpacked& unpacked) mutable
		{
			std::vector<msgpack::object> obj;
			try
			{
				obj = unpacked.get().as<std::vector<msgpack::object>>();
			}
			catch (const std::exception& e)
			{
				completeBuild(false, fmt::sprintf("Build task completion callback returned invalid data: %s", e.what()));
				return;
			}

			if (obj.empty())
			{
				completeBuild(false, "Build task completion callback did not return a result.");
				return;
			}

			bool success = false;
			std::string result = "";

			try
			{
				success = obj[0].as<bool>();

				if (obj.size() >= 2)
				{
					result = obj[1].as<std::string>();
				}
			}
			catch (const std::exception& e)
			{
				completeBuild(false, fmt::sprintf("Build task completion callback returned invalid data: %s", e.what()));
				return;
			}

			completeBuild(success, result);
		});

		m_resourceManager->CallReference<void>(m_buildRef.GetRef(), resourceName, cb);
		setupGuard.Dismiss();
	}
	catch (const std::exception& e)
	{
		completeBuild(false, fmt::sprintf("Build task setup failed: %s", e.what()));
	}
	catch (...)
	{
		completeBuild(false, "Build task setup failed.");
	}
}

using TFactoryFn = std::function<std::shared_ptr<BuildTaskProvider>()>;

static std::map<std::string, TFactoryFn> buildTaskFactories;

static void RegisterBuildTaskFactory(const std::string& id, const TFactoryFn& factory)
{
	buildTaskFactories[id] = factory;
}

static void UnregisterBuildTaskFactory(const std::string& id)
{
	buildTaskFactories.erase(id);
}

template<typename TIterator>
static void TriggerBuildSequence(Resource* resource, TIterator first, TIterator last)
{
	if (first != last)
	{
		std::shared_ptr<BuildTaskProvider> provider = (*first);
		provider->Build(resource->GetName(), [resource, first, last](bool success, const std::string& result)
		{
			if (success)
			{
				// if succeeded, continue iteration
				TriggerBuildSequence(resource, first + 1, last);
			}
			else
			{
				trace("Building resource %s failed.\n", resource->GetName());
				trace("Error data: %s\n", result);
			}
		});
	}
	else
	{
		// at the end, try to start the resource again
		trace("Build tasks completed - starting resource %s.\n", resource->GetName());

		resource->Start();
	}
}

static bool TriggerBuild(Resource* resource)
{
	// gather providers
	std::vector<std::shared_ptr<BuildTaskProvider>> providers;

	for (auto& factory : buildTaskFactories)
	{
		auto provider = factory.second();

		if (provider)
		{
			providers.push_back(provider);
		}
	}

	// check (synchronously) if they should build this resource
	std::vector<std::shared_ptr<BuildTaskProvider>> buildingProviders;

	for (auto& provider : providers)
	{
		if (provider->ShouldBuild(resource->GetName()))
		{
			buildingProviders.push_back(provider);
		}
	}

	// if there's no providers that want to build, just return instantly
	if (buildingProviders.empty())
	{
		return true;
	}

	trace("Running build tasks on resource %s - it'll restart once completed.\n", resource->GetName());

	resource->SetComponent(new BuildMap());

	auto buildMap = resource->GetComponent<BuildMap>();
	buildMap->buildingProviders = std::move(buildingProviders);

	// kick off a build sequence
	TriggerBuildSequence(resource, std::begin(buildMap->buildingProviders), std::end(buildMap->buildingProviders));

	return false;
}
}

DECLARE_INSTANCE_TYPE(fx::BuildMap);

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("REGISTER_RESOURCE_BUILD_TASK_FACTORY", [](fx::ScriptContext& context)
	{
		std::string factoryId = context.CheckArgument<const char*>(0);
		std::string factoryRef = context.CheckArgument<const char*>(1);

		fx::OMPtr<IScriptRuntime> runtime;

		if (FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			fx::Resource* resource = reinterpret_cast<fx::Resource*>(runtime->GetParentObject());

			if (resource)
			{
				std::string fullFactoryId = fmt::sprintf("%s_%s", factoryId, resource->GetName());

				auto resourceManager = resource->GetManager();

				const std::string ownerResourceName = resource->GetName();
				fx::RegisterBuildTaskFactory(fullFactoryId, [resourceManager, ownerResourceName, factoryRef]() -> std::shared_ptr<fx::ResourceBuildTaskProvider>
				{
					msgpack::unpacked unpacked;
					auto factoryObject = resourceManager->CallReferenceUnpacked<std::map<std::string, msgpack::object>>(factoryRef, &unpacked);

					if (!factoryObject.empty())
					{
						return std::make_shared<fx::ResourceBuildTaskProvider>(resourceManager, ownerResourceName, factoryObject);
					}

					return {};
				});

				resource->OnStop.Connect([resourceManager, fullFactoryId, ownerResourceName]()
				{
					fx::UnregisterBuildTaskFactory(fullFactoryId);

					resourceManager->ForAllResources([&](const fwRefContainer<fx::Resource>& targetResource)
					{
						auto buildMap = targetResource->GetComponent<fx::BuildMap>();
						if (buildMap.GetRef())
						{
							buildMap->ClearActiveBuildsForSource(ownerResourceName);
						}
					});
				});
			}
		}
	});

	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
	{
		resource->OnStop.Connect([resource]()
		{
			auto buildMap = resource->GetComponent<fx::BuildMap>();
			if (buildMap.GetRef())
			{
				buildMap->ClearActiveBuilds("Build task target resource stopped before invoking its completion callback.");
			}
		});

		resource->OnRemove.Connect([resource]()
		{
			auto buildMap = resource->GetComponent<fx::BuildMap>();
			if (buildMap.GetRef())
			{
				buildMap->ClearActiveBuilds("Build task target resource was removed before invoking its completion callback.");
			}
		});

		resource->OnBeforeStart.Connect([resource]()
		{
			return fx::TriggerBuild(resource);
		}, 1000);
	});

	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* manager)
	{
		auto nextPermissionCleanup = std::chrono::steady_clock::time_point{};
		manager->OnTick.Connect([manager, nextPermissionCleanup]() mutable
		{
			const auto now = std::chrono::steady_clock::now();
			if (now < nextPermissionCleanup)
			{
				return;
			}

			nextPermissionCleanup = now + kBuildTaskFilesystemPermissionCleanupInterval;

			manager->ForAllResources([](const fwRefContainer<fx::Resource>& resource)
			{
				auto buildMap = resource->GetComponent<fx::BuildMap>();
				if (buildMap.GetRef())
				{
					buildMap->ReleaseExpiredBuilds(resource->GetName());
				}
			});
		});
	});

});
