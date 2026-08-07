#include "StdInc.h"

#include <set>

#include <ResourceManager.h>
#include <ResourceEventComponent.h>
#include <ResourceMetaDataComponent.h>
#include <ResourceCallbackComponent.h>

#include <ServerInstanceBase.h>
#include <ServerInstanceBaseRef.h>

#include <fxScripting.h>
#include <ScriptEngine.h>

namespace fx
{
class BuildTaskProvider
{
public:
	virtual bool ShouldBuild(const std::string& resourceName) = 0;

	virtual void Build(const std::string& resourceName, const std::function<void(bool, const std::string&)>& completionCb) = 0;
};

class ResourceBuildTaskProvider : public BuildTaskProvider
{
public:
	ResourceBuildTaskProvider(fx::ResourceManager* resman, const std::map<std::string, msgpack::object>& map);

	virtual bool ShouldBuild(const std::string& resourceName) override;

	virtual void Build(const std::string& resourceName, const std::function<void(bool, const std::string&)>& completionCb) override;

private:
	fx::ResourceManager* m_resourceManager;

	FunctionRef m_shouldBuildRef;

	FunctionRef m_buildRef;
};

ResourceBuildTaskProvider::ResourceBuildTaskProvider(fx::ResourceManager* resman, const std::map<std::string, msgpack::object>& map)
	: m_resourceManager(resman)
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

	auto cbComponent = m_resourceManager->GetComponent<fx::ResourceCallbackComponent>();
	auto cb = cbComponent->CreateCallback([completionCb](const msgpack::unpacked& unpacked)
	{
		auto obj = unpacked.get().as<std::vector<msgpack::object>>();

		if (!obj.empty())
		{
			bool success = obj[0].as<bool>();
			std::string result = "";
			
			if (obj.size() >= 2)
			{
				result = obj[1].as<std::string>();
			}

			completionCb(success, result);
		}
	});

	m_resourceManager->CallReference<void>(m_buildRef.GetRef(), resourceName, cb);
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

struct BuildMap : public fwRefCountable
{
	std::vector<std::shared_ptr<BuildTaskProvider>> buildingProviders;
};

// resources with a build sequence in flight
//
// every start attempt runs a build check, so a resource can get a second one while it's still building (a dependency
// cascade will do this). that second check must not start another sequence: the running one restarts the resource
// once it's done.
static std::set<Resource*> g_buildingResources;
static std::mutex g_buildingResourcesMutex;

static void FinishBuildSequence(Resource* resource)
{
	std::lock_guard lock(g_buildingResourcesMutex);
	g_buildingResources.erase(resource);
}

// the build map is passed along by reference so the provider list outlives the sequence walking it, no matter what
// happens to the component on the resource itself
static void TriggerBuildSequence(Resource* resource, const fwRefContainer<BuildMap>& buildMap, size_t index)
{
	if (index < buildMap->buildingProviders.size())
	{
		std::shared_ptr<BuildTaskProvider> provider = buildMap->buildingProviders[index];
		provider->Build(resource->GetName(), [resource, buildMap, index](bool success, const std::string& result)
		{
			if (success)
			{
				// if succeeded, continue iteration
				TriggerBuildSequence(resource, buildMap, index + 1);
			}
			else
			{
				FinishBuildSequence(resource);

				trace("Building resource %s failed.\n", resource->GetName());
				trace("Error data: %s\n", result);
			}
		});
	}
	else
	{
		FinishBuildSequence(resource);

		// at the end, try to start the resource again
		trace("Build tasks completed - starting resource %s.\n", resource->GetName());

		resource->Start();
	}
}

static bool TriggerBuild(Resource* resource)
{
	{
		std::lock_guard lock(g_buildingResourcesMutex);

		if (g_buildingResources.find(resource) != g_buildingResources.end())
		{
			// already building, don't start the resource yet
			return false;
		}
	}

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

	{
		std::lock_guard lock(g_buildingResourcesMutex);
		g_buildingResources.insert(resource);
	}

	fwRefContainer<BuildMap> buildMap = new BuildMap();
	buildMap->buildingProviders = std::move(buildingProviders);

	resource->SetComponent(buildMap);

	// kick off a build sequence
	TriggerBuildSequence(resource, buildMap, 0);

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

				fx::RegisterBuildTaskFactory(fullFactoryId, [resourceManager, factoryRef]() -> std::shared_ptr<fx::ResourceBuildTaskProvider>
				{
					msgpack::unpacked unpacked;
					auto factoryObject = resourceManager->CallReferenceUnpacked<std::map<std::string, msgpack::object>>(factoryRef, &unpacked);

					if (!factoryObject.empty())
					{
						return std::make_shared<fx::ResourceBuildTaskProvider>(resourceManager, factoryObject);
					}

					return {};
				});

				resource->OnStop.Connect([fullFactoryId]()
				{
					fx::UnregisterBuildTaskFactory(fullFactoryId);
				});
			}
		}
	});

	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
	{
		resource->OnBeforeStart.Connect([resource]()
		{
			return fx::TriggerBuild(resource);
		}, 1000);
	});
});
