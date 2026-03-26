#include "StdInc.h"

#include <Resource.h>
#include <ResourceManager.h>

#include <ResourceMetaDataComponent.h>
#include <ResourceManagerConstraintsComponent.h>

// Returns true if installed >= required using numeric segment comparison.
// Segments are split on '.', compared as integers left-to-right, missing segments treated as 0.
static bool IsVersionSufficient(const std::string& installed, const std::string& required)
{
	auto splitVersion = [](const std::string& ver)
	{
		std::vector<int> segments;
		size_t start = 0;

		while (start < ver.size())
		{
			auto dot = ver.find('.', start);
			if (dot == std::string::npos)
			{
				dot = ver.size();
			}

			try
			{
				segments.push_back(std::stoi(ver.substr(start, dot - start)));
			}
			catch (...)
			{
				segments.push_back(0);
			}

			start = dot + 1;
		}

		return segments;
	};

	auto installedParts = splitVersion(installed);
	auto requiredParts = splitVersion(required);
	auto maxLen = std::max(installedParts.size(), requiredParts.size());

	for (size_t i = 0; i < maxLen; i++)
	{
		int a = (i < installedParts.size()) ? installedParts[i] : 0;
		int b = (i < requiredParts.size()) ? requiredParts[i] : 0;

		if (a != b)
		{
			return a > b;
		}
	}

	return true; // equal
}

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

			auto loadDeps = [&] (const std::string& type)
			{
				for (const auto& dependency : metaData->GetEntries(type))
				{
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

					// Parse optional version constraint from "resourceName:version" format
					std::string depName = dependency.second;
					std::string requiredVersion;

					auto colonPos = depName.find(':');
					if (colonPos != std::string::npos)
					{
						requiredVersion = depName.substr(colonPos + 1);
						depName = depName.substr(0, colonPos);
					}

					fwRefContainer<fx::Resource> other = manager->GetResource(depName);

					if (!other.GetRef())
					{
						trace("Could not find dependency %s for resource %s.\n", depName, resource->GetName());
						return false;
					}

					// store in a list for use in OnStop
					resourceDependants.insert({ other->GetName(), resource->GetName() });

					if (other->GetState() == fx::ResourceState::Starting || other->GetState() == fx::ResourceState::Started)
					{
						// already running, skip to version check
					}
					else
					{
						bool success = other->Start();

						if (!success)
						{
							trace("Could not start dependency %s for resource %s.\n", depName, resource->GetName());

							// store for deferred use (e.g. build systems)
							resourceDependencies.insert({ other->GetName(), resource->GetName() });

							return false;
						}
					}

					// Check minimum version constraint if specified
					if (!requiredVersion.empty())
					{
						auto otherMetaData = other->GetComponent<fx::ResourceMetaDataComponent>();
						auto versionEntries = otherMetaData->GetEntries("version");

						bool hasVersion = false;
						std::string installedVersion;

						for (const auto& versionEntry : versionEntries)
						{
							hasVersion = true;
							installedVersion = versionEntry.second;
							break;
						}

						if (!hasVersion)
						{
							trace("Warning: resource %s requires %s ^3v%s^7, but %s does not specify a version.\n",
								resource->GetName(), depName, requiredVersion, depName);
						}
						else if (!IsVersionSufficient(installedVersion, requiredVersion))
						{
							trace("Could not start resource %s: requires %s ^2v%s^7 or newer, but ^3v%s^7 is installed.\n",
								resource->GetName(), depName, requiredVersion, installedVersion);
							return false;
						}
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
