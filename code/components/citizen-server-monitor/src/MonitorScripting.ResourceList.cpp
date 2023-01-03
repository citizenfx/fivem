#include "StdInc.h"

#include "ScriptEngine.h"

#include "Resource.h"
#include "ResourceManager.h"
#include "ServerResourceList.h"

#include "ResourceCallbackComponent.h"
#include "ResourceMetaDataComponent.h"

#include "IteratorView.h"

#include <filesystem>

DLL_IMPORT fwRefContainer<fx::ResourceMounter> MakeServerResourceMounter(const fwRefContainer<fx::ResourceManager>& resman);

class ResourceScanMessage
{
public:
	std::string type;
	std::string resource;
	std::string identifier;
	std::vector<std::string> args;
	std::string message;

	ResourceScanMessage() = default;

	explicit ResourceScanMessage(const fx::resources::ScanMessage& result)
	{
		if (result.type == fx::resources::ScanMessageType::Error)
		{
			type = "error";
		}
		else if (result.type == fx::resources::ScanMessageType::Warning)
		{
			type = "warning";
		}
		else
		{
			type = "info";
		}

		resource = result.resource;
		identifier = result.identifier;
		args = result.args;
		message = result.Format();
	}

	MSGPACK_DEFINE_MAP(type, resource, identifier, args, message);
};

class ResourceDetail
{
public:
	std::string path;
	std::map<std::string, std::vector<std::string>> metadata;

	MSGPACK_DEFINE_MAP(path, metadata);
};

class ResourceScanResult
{
public:
	explicit ResourceScanResult(const fx::resources::ScanResult& result)
	{
		newResources = result.newResources;
		updatedResources = result.updatedResources;
		reloadedResources = result.reloadedResources;

		for (const auto& message : result.messages)
		{
			messages.push_back(ResourceScanMessage{ message });
		}
	}

	int newResources = 0;
	int updatedResources = 0;
	int reloadedResources = 0;
	std::map<std::string, ResourceDetail> resources;
	std::vector<ResourceScanMessage> messages;

	MSGPACK_DEFINE_MAP(newResources, updatedResources, reloadedResources, resources, messages);
};

static InitFunction initFunction([]
{
	fx::ScriptEngine::RegisterNativeHandler("SCAN_RESOURCE_ROOT", [](fx::ScriptContext& context)
	{
		std::string resourceRoot = context.CheckArgument<const char*>(0);
		auto cbRef = fx::FunctionRef{ context.CheckArgument<const char*>(1) };

		auto rm = fx::ResourceManager::GetCurrent();
		std::thread([rm, resourceRoot, cbRef = std::move(cbRef)]
		{
			fwRefContainer localManager = fx::CreateResourceManager();
			localManager->AddMounter(MakeServerResourceMounter(localManager));
			localManager->SetComponent(fx::resources::ServerResourceList::Create());

			fx::resources::ScanResult result;
			auto list = localManager->GetComponent<fx::resources::ServerResourceList>();
			list->ScanResources(resourceRoot, &result);

			ResourceScanResult resultObject{ result };

			for (const auto& resource : result.resources)
			{
				ResourceDetail detail;
				detail.path = std::filesystem::u8path(resource->GetPath()).lexically_normal().u8string();

				auto metaDataComponent = resource->GetComponent<fx::ResourceMetaDataComponent>();
				detail.metadata = metaDataComponent->GetAllEntries();

				resultObject.resources[resource->GetName()] = std::move(detail);
			}

			rm->CallReference<void>(cbRef.GetRef(), resultObject);
		})
		.detach();
	});
});
