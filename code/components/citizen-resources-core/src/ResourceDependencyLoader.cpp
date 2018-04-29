#include "StdInc.h"

#include <Resource.h>
#include <ResourceManager.h>

#include <ResourceMetaDataComponent.h>

static InitFunction initFunction([]()
{
	fx::Resource::OnInitializeInstance.Connect([] (fx::Resource* resource)
	{
		static std::multimap<std::string, std::string> resourceDependencies;

		resource->OnBeforeStart.Connect([=] ()
		{
			fx::ResourceManager* manager = resource->GetManager();
			manager->MakeCurrent();

			fwRefContainer<fx::ResourceMetaDataComponent> metaData = resource->GetComponent<fx::ResourceMetaDataComponent>();

			auto loadDeps = [&] (const std::string& type)
			{
				for (const auto& dependency : metaData->GetEntries(type))
				{
					fwRefContainer<fx::Resource> other = manager->GetResource(dependency.second);

					if (!other.GetRef())
					{
						trace("Could not find dependency %s for resource %s.\n", dependency.second, resource->GetName());
						return false;
					}

					if (other->GetState() == fx::ResourceState::Starting || other->GetState() == fx::ResourceState::Started)
					{
						continue;
					}

					bool success = other->Start();

					if (!success)
					{
						trace("Could not start dependency %s for resource %s.\n", dependency.second, resource->GetName());

						resourceDependencies.insert({ other->GetName(), resource->GetName() });

						return false;
					}
				}

				return true;
			};

			return loadDeps("dependency") && loadDeps("dependencie"); // dependencies without s
		}, -9999);

		resource->OnStart.Connect([=]()
		{
			auto pendingDeps = resourceDependencies.equal_range(resource->GetName());

			fx::ResourceManager* manager = resource->GetManager();
			manager->MakeCurrent();

			// copy in case the container gets mutated
			std::set<std::pair<std::string, std::string>> deps(pendingDeps.first, pendingDeps.second);
			
			for (auto dep : deps)
			{
				auto dependant = manager->GetResource(dep.second);

				if (dependant.GetRef())
				{
					dependant->Start();
				}
			}

			resourceDependencies.erase(resource->GetName());
		}, 9999999);
	});
});
