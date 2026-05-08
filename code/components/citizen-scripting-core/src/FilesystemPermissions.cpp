#include "StdInc.h"

#include "FilesystemPermissions.h"

#include "ResourceManager.h"
#include "ResourceMetaDataComponent.h"

#ifdef IS_FXSERVER

#include "fxScripting.h"
#include "Resource.h"

#include <filesystem>
#include <mutex>
#include <optional>
#include <unordered_map>
#include <unordered_set>

#include "ServerInstanceBase.h"

namespace fx
{
enum class FilesystemPermission : uint8_t
{
	None,
	Read,
	Write,
};

typedef std::string Source;
typedef std::string Target;

struct ResourceTupleHash
{
	std::size_t operator()(const std::tuple<Source, Target>& t) const noexcept
	{
		return std::hash<Source>()(std::get<0>(t)) ^ (std::hash<Target>()(std::get<1>(t)) << 1);
	}
};

struct ResourceTupleEqual
{
	bool operator()(const std::tuple<Source, Target>& t1, const std::tuple<Source, Target>& t2) const
	{
		return t1 == t2;
	}
};

static bool g_permissionModifyAllowed{ true };

static std::unordered_map<std::tuple<Source, Target>, FilesystemPermission, ResourceTupleHash, ResourceTupleEqual>
g_permissions{};

static std::recursive_mutex g_buildTaskPermissionMutex;

struct BuildTaskFilesystemPathPermission
{
	std::string path;
	bool isDirectory;
};

struct BuildTaskFilesystemPermission
{
	std::string sourceResource;
	std::string targetResource;
	std::vector<BuildTaskFilesystemPathPermission> allowedPaths;
};

static uint64_t g_nextBuildTaskPermissionId = 1;
static std::unordered_map<uint64_t, BuildTaskFilesystemPermission> g_buildTaskPermissions{};

static std::unordered_set<std::string> g_workerPermissions{};
static std::unordered_set<std::string> g_childProcessPermissions{};

static std::tuple<std::string, std::filesystem::path> GetResourcePath(const std::filesystem::path& path)
{
	auto it = path.begin();
	if (it == path.end())
	{
		return { "", "" };
	}

	std::string resourceName = it->string();
	if (resourceName.empty() || resourceName[0] != '@')
	{
		return { "", "" };
	}

	// remove '@'
	resourceName.erase(0, 1);

	// skip resource name
	++it;

	// sum up the remaining path
	std::filesystem::path remainingPath;
	for (; it != path.end(); ++it)
	{
		remainingPath /= *it;
	}

	return { resourceName, remainingPath };
}

static bool HasTrailingDirectorySeparator(const std::string& path)
{
	return !path.empty() && (path.back() == '/' || path.back() == '\\');
}

static std::string NormalizeRelativeResourcePath(const std::filesystem::path& path)
{
	if (path.empty() || path.is_absolute() || path.has_root_name() || path.has_root_directory())
	{
		return {};
	}

	for (const auto& part : path)
	{
		if (part == "..")
		{
			return {};
		}
	}

	const auto normalized = path.lexically_normal();
	if (normalized.empty())
	{
		return {};
	}

	std::string result;
	for (const auto& part : normalized)
	{
		const std::string partString = part.generic_string();
		if (partString.empty() || partString == ".")
		{
			continue;
		}

		if (partString == "..")
		{
			return {};
		}

		if (!result.empty())
		{
			result += '/';
		}

		result += partString;
	}

	return result;
}

static std::optional<BuildTaskFilesystemPathPermission> NormalizeBuildTaskAllowedPath(const std::string& path)
{
	const std::filesystem::path permissionPath = path;
	auto firstPart = permissionPath.begin();
	if (firstPart != permissionPath.end())
	{
		const std::string firstPartString = firstPart->generic_string();
		if (!firstPartString.empty() && firstPartString[0] == '@')
		{
			return {};
		}
	}

	const bool isDirectory = HasTrailingDirectorySeparator(path);
	const auto normalizedPath = NormalizeRelativeResourcePath(permissionPath);
	if (normalizedPath.empty())
	{
		return {};
	}

	return BuildTaskFilesystemPathPermission{
		normalizedPath,
		isDirectory
	};
}

bool ScriptingFilesystemIsBuildTaskPathSafe(const std::string& path)
{
	return NormalizeBuildTaskAllowedPath(path).has_value();
}

static bool IsBuildTaskPathAllowed(const std::string& requestedPath, const BuildTaskFilesystemPathPermission& allowedPath)
{
	if (requestedPath == allowedPath.path)
	{
		return true;
	}

	if (!allowedPath.isDirectory)
	{
		return false;
	}

	return requestedPath.rfind(allowedPath.path + '/', 0) == 0;
}

static bool ScriptingFilesystemHasBuildTaskPermission(const std::string& sourceResource, const std::string& targetResource, const std::filesystem::path& targetPath)
{
	std::lock_guard<std::recursive_mutex> lock(g_buildTaskPermissionMutex);

	const auto normalizedTargetPath = NormalizeRelativeResourcePath(targetPath);
	if (normalizedTargetPath.empty())
	{
		return false;
	}

	for (const auto& [_, permission] : g_buildTaskPermissions)
	{
		if (permission.sourceResource != sourceResource || permission.targetResource != targetResource)
		{
			continue;
		}

		for (const auto& allowedPath : permission.allowedPaths)
		{
			if (IsBuildTaskPathAllowed(normalizedTargetPath, allowedPath))
			{
				return true;
			}
		}
	}

	return false;
}

uint64_t ScriptingFilesystemPushBuildTaskPermission(const std::string& sourceResource, const std::string& targetResource, const std::vector<std::string>& allowedPaths)
{
	if (sourceResource.empty() || targetResource.empty() || sourceResource == targetResource)
	{
		return 0;
	}

	std::vector<BuildTaskFilesystemPathPermission> normalizedAllowedPaths;
	for (const auto& path : allowedPaths)
	{
		if (const auto normalizedPath = NormalizeBuildTaskAllowedPath(path))
		{
			normalizedAllowedPaths.push_back(*normalizedPath);
		}
	}

	if (normalizedAllowedPaths.empty())
	{
		return 0;
	}

	std::lock_guard<std::recursive_mutex> lock(g_buildTaskPermissionMutex);

	const uint64_t permissionId = g_nextBuildTaskPermissionId++;
	if (g_nextBuildTaskPermissionId == 0)
	{
		g_nextBuildTaskPermissionId = 1;
	}

	g_buildTaskPermissions[permissionId] = BuildTaskFilesystemPermission{
		sourceResource,
		targetResource,
		std::move(normalizedAllowedPaths)
	};

	return permissionId;
}

void ScriptingFilesystemPopBuildTaskPermission(uint64_t permissionId)
{
	if (permissionId == 0)
	{
		return;
	}

	std::lock_guard<std::recursive_mutex> lock(g_buildTaskPermissionMutex);
	g_buildTaskPermissions.erase(permissionId);
}

bool ScriptingFilesystemAllowWrite(const std::string& path, fx::Resource* resourceOverride)
{
	auto [resourceName, resourceFilePath] = GetResourcePath(path);
	if (resourceName.empty() || resourceFilePath.empty())
	{
		// invalid path
		return false;
	}

	std::string currentResourceName{};
	fx::Resource* currentResource = nullptr;
	if (resourceOverride != nullptr)
	{
		currentResource = resourceOverride;
		currentResourceName = resourceOverride->GetName();
	}
	else
	{
		fx::OMPtr<IScriptRuntime> runtime;
		if (!FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			return false;
		}

		currentResource = static_cast<fx::Resource*>(runtime->GetParentObject());
		currentResourceName = (currentResource) ? currentResource->GetName() : "";
	}

	if (currentResourceName == resourceName)
	{
		// return true if the file is inside the same resource
		return true;
	}

	if (ScriptingFilesystemHasBuildTaskPermission(currentResourceName, resourceName, resourceFilePath))
	{
		return true;
	}

	auto permission = g_permissions.find({ currentResourceName, resourceName });
	if (permission != g_permissions.end() && permission->second == FilesystemPermission::Write)
	{
		auto manager = ResourceManager::GetCurrent();
		if (!manager)
		{
			return false;
		}

		fwRefContainer<Resource> resource = manager->GetResource(resourceName);
		// if the resource is not initialized yet, we accept the write even when we can't check the author
		if (!resource.GetRef() || resource->GetState() == ResourceState::Uninitialized)
		{
			return true;
		}

		fwRefContainer<ResourceMetaDataComponent> resourceMetaDataComponent = resource->GetComponent<
		ResourceMetaDataComponent>();
		fwRefContainer<ResourceMetaDataComponent> currentResourceMetaDataComponent = currentResource->GetComponent<
		ResourceMetaDataComponent>();
		if (!resourceMetaDataComponent.GetRef() || !currentResourceMetaDataComponent.GetRef())
		{
			// should always be present
			return false;
		}

		auto resourceAuthor = resourceMetaDataComponent->GetEntries("author");
		auto currentResourceAuthor = currentResourceMetaDataComponent->GetEntries("author");
		if (resourceAuthor.begin() == resourceAuthor.end() || currentResourceAuthor.begin() == currentResourceAuthor.end())
		{
			// author should be set when write is required to be done
			return false;
		}

		if (resourceAuthor.begin()->second != currentResourceAuthor.begin()->second)
		{
			// if the authors don't match, then writing is not allowed
			return false;
		}

		// resource was explicitly allowed to modify the resource
		return true;
	}

	return false;
}

bool ScriptingWorkerAllowSpawn(fx::Resource* resourceOverride)
{
	fx::OMPtr<IScriptRuntime> runtime;
	fx::Resource* currentResource = nullptr;
	if (resourceOverride != nullptr)
	{
		currentResource = resourceOverride;
	}
	else
	{
		if (!FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			return false;
		}

		currentResource = static_cast<fx::Resource*>(runtime->GetParentObject());
	}
	
	auto permission = g_workerPermissions.find(currentResource->GetName());
	return permission != g_workerPermissions.end();
}

bool ScriptingChildProcessAllowSpawn(fx::Resource* resourceOverride)
{
	fx::OMPtr<IScriptRuntime> runtime;
	fx::Resource* currentResource = nullptr;
	if (resourceOverride != nullptr)
	{
		currentResource = resourceOverride;
	}
	else
	{
		if (!FX_SUCCEEDED(fx::GetCurrentScriptRuntime(&runtime)))
		{
			return false;
		}

		currentResource = static_cast<fx::Resource*>(runtime->GetParentObject());
	}
	auto permission = g_childProcessPermissions.find(currentResource->GetName());
	return permission != g_childProcessPermissions.end();
}
}

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		static ConsoleCommand addFSPermCmd("add_filesystem_permission", [](const std::string& sourceResource, const std::string& operation, const std::string& targetResource)
		{
			if (!fx::g_permissionModifyAllowed)
			{
				console::PrintWarning(_CFX_NAME_STRING(_CFX_COMPONENT_NAME),
				"add_filesystem_permission is only executable before the server finished execution.\n");
				return;
			}

			fx::FilesystemPermission filesystemPermission;
			if (operation == "write")
			{
				filesystemPermission = fx::FilesystemPermission::Write;
			}
			else if (operation == "read")
			{
				filesystemPermission = fx::FilesystemPermission::Read;
			}
			else
			{
				return;
			}

			fx::g_permissions[{ sourceResource, targetResource }] = filesystemPermission;
		});

		static ConsoleCommand addWorkerPermCmd("add_unsafe_worker_permission", [](const std::string& sourceResource)
		{
			if (!fx::g_permissionModifyAllowed)
			{
				console::PrintWarning(_CFX_NAME_STRING(_CFX_COMPONENT_NAME),
				"add_unsafe_worker_permission is only executable before the server finished execution.\n");
				return;
			}

			fx::g_workerPermissions.insert(sourceResource);
		});

		static ConsoleCommand addChildProcessPermCmd("add_unsafe_child_process_permission", [](const std::string& sourceResource)
		{
			if (!fx::g_permissionModifyAllowed)
			{
				console::PrintWarning(_CFX_NAME_STRING(_CFX_COMPONENT_NAME),
				"add_unsafe_child_process_permission is only executable before the server finished execution.\n");
				return;
			}

			fx::g_childProcessPermissions.insert(sourceResource);
		});

		instance->OnInitialConfiguration.Connect([]()
		{
			fx::g_permissionModifyAllowed = false;
		});
	});
});
#else
namespace fx
{
bool ScriptingFilesystemAllowWrite(const std::string& path, fx::Resource* resourceOverride)
{
	return true;
}

uint64_t ScriptingFilesystemPushBuildTaskPermission(const std::string& sourceResource, const std::string& targetResource, const std::vector<std::string>& allowedPaths)
{
	return 0;
}

void ScriptingFilesystemPopBuildTaskPermission(uint64_t permissionId)
{
}

bool ScriptingFilesystemIsBuildTaskPathSafe(const std::string& path)
{
	return true;
}
}
#endif
