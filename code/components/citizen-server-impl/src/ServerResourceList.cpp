#include "StdInc.h"
#include "ServerResourceList.h"

#include "CoreConsole.h"

#include "Resource.h"
#include "ResourceManager.h"
#include "ResourceMetaDataComponent.h"

#include "ManifestVersion.h"
#include "VFSManager.h"

#include <boost/algorithm/string.hpp>

#include <skyr/percent_encode.hpp>
#include <skyr/url.hpp>

#include <filesystem>
#include <queue>

namespace fx::resources
{
fwRefContainer<ServerResourceList> ServerResourceList::Create()
{
	return new ServerResourceList();
}

void ServerResourceList::AttachToObject(fx::ResourceManager* object)
{
	m_manager = object;
}

void ServerResourceList::ScanResources(const std::string& resourceRoot, ScanResult* outResult /* = nullptr */)
{
	m_currentResult = outResult;

	auto resourceRootPath = std::filesystem::u8path(resourceRoot).lexically_normal();

	std::queue<std::string> pathsToIterate;
	pathsToIterate.push(resourceRoot);

	std::vector<pplx::task<fwRefContainer<fx::Resource>>> tasks;

	// save scanned resource names so we don't scan them twice
	std::map<std::string, std::string> scannedNow;
	std::set<std::string> updatedNow;
	size_t newResources = 0;
	size_t updatedResources = 0;
	size_t reloadedResources = 0;

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

				// common things to skip
				if (findData.name == ".git")
				{
					continue;
				}

				if (findData.attributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					std::string resPath(thisPath + "/" + findData.name);

					// is this a category?
					if (findData.name[0] == '[' && findData.name[findData.name.size() - 1] == ']')
					{
						pathsToIterate.push(resPath);

						// check if it has a fxmanifest/__resource
						if (vfsDevice->GetAttributes(resPath + "/fxmanifest.lua") != -1 || vfsDevice->GetAttributes(resPath + "/__resource.lua") != -1)
						{
							AddError(ScanMessageType::Warning, findData.name.substr(1, findData.name.length() - 2), "category_manifest", { findData.name });
						}
					}
					// it's a resource
					else if (scannedNow.find(findData.name) == scannedNow.end())
					{
						const auto& resourceName = findData.name;
						scannedNow.emplace(resourceName, resPath);

						auto oldRes = m_manager->GetResource(resourceName, false);
						auto oldScanData = m_scanData.find(resourceName);

						// did the path change? if so, unload the old resource
						if (oldRes.GetRef() && oldScanData != m_scanData.end() && oldScanData->second != resPath)
						{
							{
								std::unique_lock _(m_resourcesByComponentMutex);

								// remove from by-component lists
								for (auto& list : m_resourcesByComponent)
								{
									list.second.erase(resourceName);
								}
							}

							// unmount relative device
							vfs::Unmount(fmt::sprintf("@%s/", resourceName));

							// stop and remove resource
							oldRes->Stop();
							m_manager->RemoveResource(oldRes);

							// undo ptr
							oldRes = {};
						}

						if (oldRes.GetRef())
						{
							auto metaDataComponent = oldRes->GetComponent<fx::ResourceMetaDataComponent>();

							// filter function to remove _extra entries (Lua table ordering determinism)
							auto filterMetadata = [](auto&& metadataIn)
							{
								for (auto it = metadataIn.begin(); it != metadataIn.end();)
								{
									if (boost::algorithm::ends_with(it->first, "_extra"))
									{
										it = metadataIn.erase(it);
									}
									else
									{
										++it;
									}
								}

								return std::move(metadataIn);
							};

							// save the old metadata for comparison
							auto oldMetaData = filterMetadata(metaDataComponent->GetAllEntries());

							// load new metadata
							metaDataComponent->LoadMetaData(resPath);

							// compare differences
							auto newMetaData = filterMetadata(metaDataComponent->GetAllEntries());
							bool different = (newMetaData.size() != oldMetaData.size()) || (newMetaData != oldMetaData);

							// if different, track as updated
							if (different)
							{
								updatedNow.insert(resourceName);
								updatedResources++;
							}

							// track resource as reloaded
							reloadedResources++;
						}
						else
						{
							console::DPrintf("resources", "Found new resource %s in %s\n", resourceName, resPath);
							newResources++;
							updatedNow.insert(resourceName);

							auto path = std::filesystem::u8path(resPath);

							// get parent path components
							std::error_code ec;
							std::vector<std::string> components;

							auto relPath = std::filesystem::relative(path, resourceRootPath, ec);

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

							m_scanData[resourceName] = resPath;

							skyr::url_record record;
							record.scheme = "file";

							skyr::url url{ std::move(record) };
							url.set_pathname(*skyr::percent_encode(resPath, skyr::encode_set::path));
							url.set_hash(*skyr::percent_encode(resourceName, skyr::encode_set::fragment));

							auto task = m_manager->AddResource(url.href())
										.then([this, components = std::move(components)](fwRefContainer<fx::Resource> resource)
										{
											if (resource.GetRef())
											{
												std::unique_lock _(m_resourcesByComponentMutex);

												for (const auto& component : components)
												{
													m_resourcesByComponent[component].insert(resource->GetName());
												}
											}

											return resource;
										});

							tasks.push_back(task);
						}
					}
					else // scannedNow already contained the resource
					{
						auto scannedEntry = scannedNow.find(findData.name);
						AddError(ScanMessageType::Warning, findData.name, "duplicate_resource", { resPath, scannedEntry->second });
					}
				}
			} while (vfsDevice->FindNext(handle, &findData));

			vfsDevice->FindClose(handle);
		}
	}

	auto combinedTask = pplx::when_all(tasks.begin(), tasks.end());
	combinedTask.wait();

	if (outResult)
	{
		for (const auto& resource : combinedTask.get())
		{
			if (resource.GetRef())
			{
				outResult->resources.push_back(resource);
			}
		}
	}

	// check for outdated
	m_manager->ForAllResources([this, &updatedNow](const fwRefContainer<fx::Resource>& resource)
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
					auto resourceName = resource->GetName();

					// only alert if a resource updated this iteration
					if (updatedNow.find(resourceName) != updatedNow.end())
					{
						AddError(ScanMessageType::Warning, resourceName, "outdated_manifest", {});
					}
				}
			}
		}
	});

	if (outResult)
	{
		outResult->newResources += newResources;
		outResult->reloadedResources += reloadedResources;
		outResult->updatedResources += updatedResources;
	}

	m_currentResult = nullptr;
}

void ServerResourceList::AddError(ScanMessageType type, const std::string& resource, const std::string& identifier, std::vector<std::string>&& args)
{
	if (m_currentResult)
	{
		std::unique_lock _(m_resultMutex);

		ScanMessage message;
		message.type = type;
		message.resource = resource;
		message.identifier = identifier;
		message.args = std::move(args);

		m_currentResult->messages.push_back(std::move(message));
	}
}

std::set<std::string> ServerResourceList::FindByPathComponent(const std::string& pathComponent)
{
	std::shared_lock _(m_resourcesByComponentMutex);

	auto category = m_resourcesByComponent.find(pathComponent);

	if (category == m_resourcesByComponent.end())
	{
		return {};
	}

	return category->second;
}

std::string ScanMessage::Format() const
{
	if (identifier == "outdated_manifest")
	{
		return fmt::sprintf("%s has an outdated manifest (__resource.lua instead of fxmanifest.lua)", resource);
	}
	else if (identifier == "duplicate_resource")
	{
		auto cleanPath = [](const std::string& resPath)
		{
			try
			{
				return std::filesystem::u8path(resPath).lexically_normal().u8string();
			}
			catch (...)
			{
				return resPath;
			}
		};

		return fmt::sprintf("%s exists in more than one place (%s is used, the duplicate is %s)", resource, cleanPath(args[1]), cleanPath(args[0]));
	}
	else if (identifier == "load_failed")
	{
		return fmt::sprintf("%s failed to load: %s", resource, args[0]);
	}
	else if (identifier == "no_manifest")
	{
		return fmt::sprintf("%s does not have a resource manifest (fxmanifest.lua)", resource);
	}
	else if (identifier == "category_manifest")
	{
		return fmt::sprintf("%s is a category, but has a resource manifest (if this is meant to be a resource, don't use [] around the name)", args[0]);
	}

	return fmt::sprintf("[%s]", identifier);
}
}
