#include "StdInc.h"
#include <ResourceConfigurationCacheComponent.h>

#include <json.hpp>
#include <shared_mutex>

#include <ResourceFilesComponent.h>
#include <ResourceStreamComponent.h>

namespace fx
{
void ResourceConfigurationCacheComponent::Invalidate()
{
	m_dirty = true;
}

void ResourceConfigurationCacheComponent::GetConfiguration(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator, const std::function<std::optional<std::string>(const std::string&)>& getFileServer)
{
	if (!m_dirty || !m_resource)
	{
		std::lock_guard lock {m_lock};
		value.CopyFrom(m_configuration, allocator);
		return;
	}

	std::lock_guard lock {m_lock};
	
	m_dirty = false;
	m_configuration = {};
	m_configuration.SetObject();

	rapidjson::Value resourceFiles;
	resourceFiles.SetObject();

	fwRefContainer<ResourceFilesComponent> files = m_resource->GetComponent<ResourceFilesComponent>();

	for (const auto& entry : files->GetFileHashPairs())
	{
		auto key = rapidjson::Value{
			entry.first.data(), static_cast<rapidjson::SizeType>(entry.first.size()), m_configuration.GetAllocator()
		};
		auto value = rapidjson::Value{
			entry.second.data(), static_cast<rapidjson::SizeType>(entry.second.size()), m_configuration.GetAllocator()
		};

		resourceFiles.AddMember(std::move(key), std::move(value), m_configuration.GetAllocator());
	}

	rapidjson::Value resourceStreamFiles;
	resourceStreamFiles.SetObject();

	// todo: do the streaming list change at runtime?
	fwRefContainer<ResourceStreamComponent> streamFiles = m_resource->GetComponent<ResourceStreamComponent>();

	for (const auto& entry : streamFiles->GetStreamingList())
	{
		if (!entry.second.isAutoScan)
		{
			continue;
		}

		rapidjson::Value obj;
		obj.SetObject();

		obj.AddMember("hash", rapidjson::Value{ entry.second.hashString, m_configuration.GetAllocator() }, m_configuration.GetAllocator());
		obj.AddMember("rscFlags", rapidjson::Value{ entry.second.rscFlags }, m_configuration.GetAllocator());
		obj.AddMember("rscVersion", rapidjson::Value{ entry.second.rscVersion }, m_configuration.GetAllocator());
		obj.AddMember("size", rapidjson::Value{ entry.second.size }, m_configuration.GetAllocator());

		if (entry.second.isResource)
		{
			obj.AddMember("rscPagesVirtual", rapidjson::Value{ entry.second.rscPagesVirtual }, m_configuration.GetAllocator());
			obj.AddMember("rscPagesPhysical", rapidjson::Value{ entry.second.rscPagesPhysical }, m_configuration.GetAllocator());
		}

		auto key = rapidjson::Value{
			entry.first.c_str(), static_cast<rapidjson::SizeType>(entry.first.length()), m_configuration.GetAllocator()
		};
		resourceStreamFiles.AddMember(std::move(key), std::move(obj), m_configuration.GetAllocator());
	}

	m_configuration.AddMember("name", rapidjson::StringRef(m_resource->GetName().c_str(), m_resource->GetName().length()),
	m_configuration.GetAllocator());
	m_configuration.AddMember("files", std::move(resourceFiles), m_configuration.GetAllocator());
	m_configuration.AddMember("streamFiles", std::move(resourceStreamFiles), m_configuration.GetAllocator());

	if (const auto fileServer = getFileServer(m_resource->GetName()))
	{
		m_configuration.AddMember("fileServer", rapidjson::Value{ fileServer.value().data(), static_cast<rapidjson::SizeType>(fileServer.value().size()), m_configuration.GetAllocator() }, m_configuration.GetAllocator());
	}

	value.CopyFrom(m_configuration, allocator);
}

void ResourceConfigurationCacheComponent::AttachToObject(Resource* object)
{
	m_resource = object;

	object->OnStart.Connect([=]()
	{
		Invalidate();
	},
	501);

	object->OnStop.Connect([=]()
	{
		Invalidate();
	},
	501);
}
}

static InitFunction initFunction([]()
{
	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
	{
		resource->SetComponent(new fx::ResourceConfigurationCacheComponent());
	});
});
