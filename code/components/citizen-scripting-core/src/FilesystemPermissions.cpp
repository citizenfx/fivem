#include "StdInc.h"

#include "FilesystemPermissions.h"

#include "ResourceManager.h"
#include "ResourceMetaDataComponent.h"

#ifdef IS_FXSERVER

#include "fxScripting.h"
#include "Resource.h"

#include <filesystem>
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

bool ScriptingFilesystemAllowWrite(const std::string& path, fx::Resource* resourceOverride)
{
	std::filesystem::path filePath = path;
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
}
#endif
