#include "StdInc.h"

#include <Resource.h>
#include <ResourceManager.h>

#include <ResourceMetaDataComponent.h>
#include <ResourceManagerConstraintsComponent.h>

static InitFunction initFunction([]()
{
	static std::multimap<std::string, std::string> resourceDependencies;
	static std::multimap<std::string, std::string> resourceDependants;

	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* resman)
	{
		resman->OnAfterReset.Connect([]()
		{
			resourceDependants.clear();
			resourceDependencies.clear();
		});
	});

	fx::Resource::OnInitializeInstance.Connect([] (fx::Resource* resource)
	{
		resource->OnBeforeStart.Connect([=] ()
		{
			fx::ResourceManager* manager = resource->GetManager();
			manager->MakeCurrent();

			fwRefContainer<fx::ResourceManagerConstraintsComponent> constraintsComponent = manager->GetComponent<fx::ResourceManagerConstraintsComponent>();
			fwRefContainer<fx::ResourceMetaDataComponent> metaData = resource->GetComponent<fx::ResourceMetaDataComponent>();

			bool success = true;
			std::vector<std::string> errorList;

			auto loadDeps = [&] (const std::string& type, const bool soft = false)
			{
				for (const auto& dependency : metaData->GetEntries(type))
				{
					// Skip constraints when handling soft dependencies, as these can't be soft.
					if (!soft)
					{
						std::string constraintError;
						auto constraintResult = constraintsComponent->MatchConstraint(dependency.second, &constraintError);

						if (constraintResult != fx::ConstraintMatchResult::NoConstraint)
						{
							if (constraintResult == fx::ConstraintMatchResult::Fail)
							{
								errorList.push_back(constraintError);
								success = false;
							}

							continue;
						}
					}

					fwRefContainer<fx::Resource> other = manager->GetResource(dependency.second);

					if (!other.GetRef())
					{
						trace("Could not find %s %s for resource %s.\n", soft ? "soft dependency" : "dependency", dependency.second, resource->GetName());

						// Continue to the next dependency if we're handling soft dependencies, as these dependencies are optional.
						if (soft)
						{
							continue;
						}

						return false;
					}

					if (!soft)
					{
						// store in a list for use in OnStop
						resourceDependants.insert({ other->GetName(), resource->GetName() });
					}

					if (other->GetState() == fx::ResourceState::Starting || other->GetState() == fx::ResourceState::Started)
					{
						continue;
					}

					bool success = other->Start();

					if (!success)
					{
						trace("Could not start %s %s for resource %s.\n", soft ? "soft dependency" : "dependency", dependency.second, resource->GetName());

						// Continue to the next dependency if we're handling soft dependencies, just like we do when one is missing.
						if (soft)
						{
							continue;
						}

						// store for deferred use (e.g. build systems)
						resourceDependencies.insert({ other->GetName(), resource->GetName() });

						return false;
					}
				}

				return true;
			};

			bool innerSuccess = loadDeps("dependency") && loadDeps("dependencie"); // dependencies without s

			if (!innerSuccess || !success)
			{
				if (!errorList.empty())
				{
					trace("Resource '%s' can't run:\n", resource->GetName());
					int i = 1;

					for (const auto& error : errorList)
					{
						trace("% 3d. %s\n", i, error);
						++i;
					}
				}

				return false;
			}

			// Load soft dependencies defined with `soft_dependency` or `soft_dependencies`.
			// We ignore the result, as we don't want to cancel loading the resource if a soft dependency is missing.
			loadDeps("soft_dependency", true) && loadDeps("soft_dependencie", true); // soft_dependencies without s

			return true;
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

		resource->OnStop.Connect([resource]()
		{
			// stop all resources depending on `resource` (potentially recursive, bad for cyclic dependencies)
			auto resDeps = resourceDependants.equal_range(resource->GetName());

			fx::ResourceManager* manager = resource->GetManager();
			manager->MakeCurrent();

			// copy in case the container gets mutated
			std::set<std::pair<std::string, std::string>> deps(resDeps.first, resDeps.second);

			for (auto dep : deps)
			{
				auto dependant = manager->GetResource(dep.second);

				if (dependant.GetRef() && dependant->GetName() != resource->GetName())
				{
					dependant->Stop();
				}
			}

			resourceDependants.erase(resource->GetName());
		}, INT32_MIN);
	});
});
