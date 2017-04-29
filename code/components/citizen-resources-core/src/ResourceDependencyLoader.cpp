#include "StdInc.h"

#include <Resource.h>
#include <ResourceManager.h>

#include <ResourceMetaDataComponent.h>

static InitFunction initFunction([]()
{
	fx::Resource::OnInitializeInstance.Connect([] (fx::Resource* resource)
	{
		resource->OnStart.Connect([=] ()
		{
			fx::ResourceManager* manager = Instance<fx::ResourceManager>::Get();
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

					bool success = other->Start();

					if (!success)
					{
						trace("Could not start dependency %s for resource %s.\n", dependency.second, resource->GetName());
						return false;
					}
				}

				return true;
			};

			return loadDeps("dependency") && loadDeps("dependencie"); // dependencies without s
		}, -99999);
	});
});