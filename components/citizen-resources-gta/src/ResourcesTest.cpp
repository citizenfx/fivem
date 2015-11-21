/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ResourceManager.h"

#include <fiDevice.h>
#include <CachedResourceMounter.h>

#include <ResourceMetaDataComponent.h>

fwRefContainer<fx::ResourceManager> g_resourceManager;

void CfxCollection_AddStreamingFile(const std::string& fileName, rage::ResourceFlags flags);

namespace streaming
{
	void AddMetaToLoadList(bool before, const std::string& meta);

	void SetNextLevelPath(const std::string& path);
}

static InitFunction initFunction([] ()
{
	fx::OnAddStreamingResource.Connect([] (const fx::StreamingEntryData& entry)
	{
		CfxCollection_AddStreamingFile("cache:/" + entry.resourceName + "/" + entry.fileName, { entry.rscPagesVirtual, entry.rscPagesPhysical });
	});

	fx::Resource::OnInitializeInstance.Connect([] (fx::Resource* resource)
	{
		resource->OnStart.Connect([=] ()
		{
			fwRefContainer<fx::ResourceMetaDataComponent> metaData = resource->GetComponent<fx::ResourceMetaDataComponent>();
			std::string resourceRoot = resource->GetPath();

			for (auto& meta : metaData->GetEntries("before_level_meta"))
			{
				streaming::AddMetaToLoadList(true, resourceRoot + meta.second);
			}

			for (auto& meta : metaData->GetEntries("after_level_meta"))
			{
				streaming::AddMetaToLoadList(false, resourceRoot + meta.second);
			}

			for (auto& meta : metaData->GetEntries("replace_level_meta"))
			{
				streaming::SetNextLevelPath(resourceRoot + meta.second);
			}
		}, 500);
	});

	rage::fiDevice::OnInitialMount.Connect([] ()
	{
		//while (true)
		{
			fwRefContainer<fx::ResourceManager> manager = fx::CreateResourceManager();
			Instance<fx::ResourceManager>::Set(manager.GetRef());

			g_resourceManager = manager;

			// prevent this from getting destructed on exit - that might try doing really weird things to the game
			g_resourceManager->AddRef();
		}
	}, 9000);
});