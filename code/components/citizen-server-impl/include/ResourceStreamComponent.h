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
		};

	public:
		const std::map<std::string, Entry>& GetStreamingList();

		virtual void AttachToObject(fx::Resource* object) override;

	private:
		bool ShouldUpdateSet();

		bool UpdateSet();

	private:
		fx::Resource* m_resource;

		std::map<std::string, Entry> m_resourcePairs;
	};
}

DECLARE_INSTANCE_TYPE(fx::ResourceStreamComponent);
