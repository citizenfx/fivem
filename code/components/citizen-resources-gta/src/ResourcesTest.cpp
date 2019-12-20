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

#include <ResourceCache.h>
#include <ResourceMetaDataComponent.h>

#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/include/std_tuple.hpp>

#include <boost/iterator/zip_iterator.hpp>

#include <boost/algorithm/string.hpp>

#include <rapidjson/document.h>

#include <CoreConsole.h>

#if __has_include(<GlobalEvents.h>)
#include <GlobalEvents.h>
#endif

#include <ResourceGameLifetimeEvents.h>

#include <VFSManager.h>

#include <Error.h>

#include <deque>

fwRefContainer<fx::ResourceManager> g_resourceManager;

#if __has_include(<streaming.h>)
void DLL_IMPORT CfxCollection_AddStreamingFileByTag(const std::string& tag, const std::string& fileName, rage::ResourceFlags flags);
void DLL_IMPORT CfxCollection_RemoveStreamingTag(const std::string& tag);
void DLL_IMPORT CfxCollection_SetStreamingLoadLocked(bool locked);

namespace streaming
{
	void AddMetaToLoadList(bool before, const std::string& meta);
	void AddDefMetaToLoadList(const std::string& meta);
	void AddDataFileToLoadList(const std::string& type, const std::string& path);
	void RemoveDataFileFromLoadList(const std::string& type, const std::string& path);

	void SetNextLevelPath(const std::string& path);
}
#endif

template<typename... T>
auto MakeIterator(const T&&... args)
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

struct ResourceEntryListComponent : public fwRefCountable
{
	std::deque<std::pair<std::string, std::string>> list;
};

DECLARE_INSTANCE_TYPE(ResourceEntryListComponent);

static InitFunction initFunction([] ()
{
#if __has_include(<streaming.h>)
	fx::OnAddStreamingResource.Connect([] (const fx::StreamingEntryData& entry)
	{
		CfxCollection_AddStreamingFileByTag(entry.resourceName, entry.filePath, { entry.rscPagesVirtual, entry.rscPagesPhysical });
	});

	fx::OnLockStreaming.Connect([]()
	{
		CfxCollection_SetStreamingLoadLocked(true);
	});

	fx::OnUnlockStreaming.Connect([]()
	{
		CfxCollection_SetStreamingLoadLocked(false);
	});
#endif

#if __has_include(<GlobalEvents.h>)
	OnMsgConfirm.Connect([]()
	{
		Instance<fx::ResourceManager>::Get()->ForAllResources([](fwRefContainer<fx::Resource> resource)
		{
			resource->GetComponent<fx::ResourceGameLifetimeEvents>()->OnBeforeGameShutdown();
		});
	}, -500);
#endif

	fx::Resource::OnInitializeInstance.Connect([] (fx::Resource* resource)
	{
		resource->SetComponent(new ResourceEntryListComponent());

#if __has_include(<streaming.h>)
		resource->OnStart.Connect([=] ()
		{
			if (resource->GetName() == "_cfx_internal")
			{
				return;
			}

			fwRefContainer<fx::ResourceMetaDataComponent> metaData = resource->GetComponent<fx::ResourceMetaDataComponent>();
			std::string resourceRoot = resource->GetPath();

			// block anything resembling an assault_vehicles resource, this is outdated
			// and causes crashes (FIVEM-CLIENT-1365-18)
			for (auto& entry : metaData->GetEntries("data_file_extra"))
			{
				if (entry.second == "\"data/ai/vehicleweapons_caracara.meta\"")
				{
					trace("Ignoring resource data files for %s - this has been obsoleted by the 1.0.1365 update (mpassault_vehicles).\n", resource->GetName());
					return;
				}
			}

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

			auto entryListComponent = resource->GetComponent<ResourceEntryListComponent>();

			auto view1 = metaData->GetEntries("data_file");
			auto view2 = metaData->GetEntries("data_file_extra");

			for (auto it1 = view1.begin(), end1 = view1.end(), it2 = view2.begin(), end2 = view2.end(); it1 != end1 && it2 != end2; ++it1, ++it2)
			{
				const std::string& type = it1->second;
				const std::string& name = it2->second;

				rapidjson::Document document;
				document.Parse(name.c_str(), name.length());

				if (!document.HasParseError() && document.IsString())
				{
					std::string key = document.GetString();
					auto list = metaData->GlobValueVector(key);

					if (list.empty() && key.find('*') == std::string::npos)
					{
						list.push_back(key);
					}

					for (auto& entry : list)
					{
						auto path = resourceRoot + entry;

						streaming::AddDataFileToLoadList(type, path);
						entryListComponent->list.push_front({ type, path });
					}
				}
			}

			{
				auto map = metaData->GetEntries("this_is_a_map");

				if (map.begin() != map.end())
				{
					// only load resource surrogates for this_is_a_map resources (as they might involve gta_cache files)
					// due to R* packfile/collection limits, *don't* load RPF_FILE anymore
					// instead, we're faking this out for cache

					//streaming::AddDataFileToLoadList("RPF_FILE", "resource_surrogate:/" + resource->GetName() + ".rpf");
					//entryListComponent->list.push_front({ "RPF_FILE", "resource_surrogate:/" + resource->GetName() + ".rpf" });

					streaming::AddDataFileToLoadList("CFX_PSEUDO_ENTRY", "RELOAD_MAP_STORE");
				}
			}

			bool shouldCache = false;

			{
				if (vfs::OpenRead(fmt::sprintf("%s%s_cache_y.dat", resourceRoot, resource->GetName())).GetRef())
				{
					shouldCache = true;
				}
			}

			if (!shouldCache)
			{
				auto cacheList = resource->GetComponent<ResourceCacheEntryList>();

				for (const auto& entry : cacheList->GetEntries())
				{
					if (boost::algorithm::ends_with(entry.first, ".ymf"))
					{
						shouldCache = true;
						break;
					}
				}
			}

			if (shouldCache)
			{
				streaming::AddDataFileToLoadList("CFX_PSEUDO_CACHE", resource->GetName());
			}
		}, 500);

		resource->OnStop.Connect([=] ()
		{
			auto entryListComponent = resource->GetComponent<ResourceEntryListComponent>();
			auto thisList = std::move(entryListComponent->list);

			for (const auto& entry : thisList)
			{
				streaming::RemoveDataFileFromLoadList(entry.first, entry.second);
			}

			CfxCollection_RemoveStreamingTag(resource->GetName());
		}, -500);
#endif
	});

	rage::fiDevice::OnInitialMount.Connect([] ()
	{
		fwRefContainer<fx::ResourceManager> manager = fx::CreateResourceManager();
		manager->SetComponent(console::GetDefaultContext());

		Instance<fx::ResourceManager>::Set(manager.GetRef());

		g_resourceManager = manager;

		// prevent this from getting destructed on exit - that might try doing really weird things to the game
		g_resourceManager->AddRef();
	}, 9000);
});
