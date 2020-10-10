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

		struct RuntimeEntry : public Entry
		{
			inline RuntimeEntry(const Entry& entry)
				: Entry(entry), isAutoScan(false)
			{

			}

			bool isAutoScan;
			std::string loadDiskPath;
		};

	public:
		const std::map<std::string, RuntimeEntry>& GetStreamingList();

		RuntimeEntry* AddStreamingFile(const Entry& entry);

		RuntimeEntry* AddStreamingFile(const std::string& entryName, const std::string& fullPath, const std::string& cacheString);

		RuntimeEntry* AddStreamingFile(const std::string& fullPath);

		virtual void AttachToObject(fx::Resource* object) override;

	private:
		bool ShouldUpdateSet();

		bool UpdateSet();

		void ValidateSize(std::string_view name, uint32_t physSize, uint32_t virtSize);

	private:
		fx::Resource* m_resource;

		std::map<std::string, RuntimeEntry> m_resourcePairs;

		std::map<std::string, RuntimeEntry*> m_hashPairs;
	};
}

DECLARE_INSTANCE_TYPE(fx::ResourceStreamComponent);
