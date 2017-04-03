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

#include <Error.h>

#include <deque>

fwRefContainer<fx::ResourceManager> g_resourceManager;

void CfxCollection_AddStreamingFile(const std::string& fileName, rage::ResourceFlags flags);
void CfxCollection_AddStreamingFileByTag(const std::string& tag, const std::string& fileName, rage::ResourceFlags flags);

namespace streaming
{
	void AddMetaToLoadList(bool before, const std::string& meta);
	void AddDefMetaToLoadList(const std::string& meta);
	void AddDataFileToLoadList(const std::string& type, const std::string& path);
	void RemoveDataFileFromLoadList(const std::string& type, const std::string& path);

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

// TODO: replace with fold expressions when moving to C++17 compilers
bool SequenceEquals(ptrdiff_t head = 0)
{
	return true;
}

template<typename... Tail>
bool SequenceEquals(ptrdiff_t head, ptrdiff_t mid, Tail... tail)
{
	return head == mid && SequenceEquals(head, tail...);
}

template<typename... T>
bool RangeLengthMatches(const T&... args)
{
	return SequenceEquals(std::distance(args.begin(), args.end())...);
}

static InitFunction initFunction([] ()
{
	fx::OnAddStreamingResource.Connect([] (const fx::StreamingEntryData& entry)
	{
		CfxCollection_AddStreamingFileByTag(entry.resourceName, entry.filePath, { entry.rscPagesVirtual, entry.rscPagesPhysical });
	});

	fx::Resource::OnInitializeInstance.Connect([] (fx::Resource* resource)
	{
		auto entryList = std::make_shared<std::deque<std::pair<std::string, std::string>>>();

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

			if (!RangeLengthMatches(metaData->GetEntries("data_file"), metaData->GetEntries("data_file_extra")))
			{
				GlobalError("data_file entry count mismatch in resource %s", resource->GetName());
				return;
			}

			streaming::AddDataFileToLoadList("RPF_FILE", "resource_surrogate:/" + resource->GetName() + ".rpf");

			for (auto& pair : MakeIterator(metaData->GetEntries("data_file"), metaData->GetEntries("data_file_extra")))
			{
				const std::string& type = std::get<0>(pair).second;
				const std::string& name = std::get<1>(pair).second;

				rapidjson::Document document;
				document.Parse(name.c_str(), name.length());

				if (!document.HasParseError() && document.IsString())
				{
					auto path = resourceRoot + document.GetString();

					streaming::AddDataFileToLoadList(type, path);
					entryList->push_front({ type, path });
				}
			}

			{
				auto map = metaData->GetEntries("this_is_a_map");

				if (map.begin() != map.end())
				{
					streaming::AddDataFileToLoadList("CFX_PSEUDO_ENTRY", "RELOAD_MAP_STORE");
				}
			}
		}, 500);

		resource->OnStop.Connect([=] ()
		{
			for (const auto& entry : *entryList)
			{
				streaming::RemoveDataFileFromLoadList(entry.first, entry.second);
			}

			entryList->clear();
		}, -500);
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