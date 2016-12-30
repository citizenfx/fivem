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

#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/include/std_tuple.hpp>

#include <boost/iterator/zip_iterator.hpp>

#include <rapidjson/document.h>

fwRefContainer<fx::ResourceManager> g_resourceManager;

void CfxCollection_AddStreamingFile(const std::string& fileName, rage::ResourceFlags flags);

namespace streaming
{
	void AddMetaToLoadList(bool before, const std::string& meta);
	void AddDefMetaToLoadList(const std::string& meta);
	void AddDataFileToLoadList(const std::string& type, const std::string& path);

	void SetNextLevelPath(const std::string& path);
}

template<typename... T>
auto MakeIterator(const T&... args)
{
	return fx::GetIteratorView
	(
		boost::make_zip_iterator(std::make_tuple(std::begin(args)...)),
		boost::make_zip_iterator(std::make_tuple(std::end(args)...))
	);
}

static InitFunction initFunction([] ()
{
	fx::OnAddStreamingResource.Connect([] (const fx::StreamingEntryData& entry)
	{
        // don't allow overriding manifest.ymf
        if (entry.fileName == "_manifest.ymf")
        {
            return;
        }

		CfxCollection_AddStreamingFile("cache:/" + entry.resourceName + "/" + entry.fileName, { entry.rscPagesVirtual, entry.rscPagesPhysical });
	});

	fx::Resource::OnInitializeInstance.Connect([] (fx::Resource* resource)
	{
		resource->OnStart.Connect([=] ()
		{
			fwRefContainer<fx::ResourceMetaDataComponent> metaData = resource->GetComponent<fx::ResourceMetaDataComponent>();
			std::string resourceRoot = resource->GetPath();

			for (auto& meta : metaData->GetEntries("init_meta"))
			{
				streaming::AddDefMetaToLoadList(resourceRoot + meta.second);
			}

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
			
			for (auto& pair : MakeIterator(metaData->GetEntries("data_file"), metaData->GetEntries("data_file_extra")))
			{
				const std::string& type = std::get<0>(pair).second;
				const std::string& name = std::get<1>(pair).second;

				rapidjson::Document document;
				document.Parse(name.c_str(), name.length());

				if (!document.HasParseError() && document.IsString())
				{
					streaming::AddDataFileToLoadList(type, resourceRoot + document.GetString());
				}
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