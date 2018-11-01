#pragma once

#include <Resource.h>

namespace fx
{
	class ResourceStreamComponent : public fwRefCountable, public IAttached<fx::Resource>
	{
	public:
		struct Entry
		{
			char entryName[128];
			char hashString[128];
			char onDiskPath[512];
			uint32_t rscPagesVirtual;
			uint32_t rscPagesPhysical;
			uint32_t rscFlags;
			uint32_t rscVersion;
			uint32_t size;
			bool isResource;

			std::string GetCacheString();
		};

		struct StorageEntry : public Entry
		{
			inline StorageEntry(const Entry& entry)
				: Entry(entry), isAutoScan(false)
			{

			}

			bool isAutoScan;
		};

	public:
		const std::map<std::string, StorageEntry>& GetStreamingList();

		StorageEntry* AddStreamingFile(const Entry& entry);

		StorageEntry* AddStreamingFile(const std::string& entryName, const std::string& fullPath, const std::string& cacheString);

		StorageEntry* AddStreamingFile(const std::string& fullPath);

		virtual void AttachToObject(fx::Resource* object) override;

	private:
		bool ShouldUpdateSet();

		bool UpdateSet();

	private:
		fx::Resource* m_resource;

		std::map<std::string, StorageEntry> m_resourcePairs;
	};
}

DECLARE_INSTANCE_TYPE(fx::ResourceStreamComponent);
