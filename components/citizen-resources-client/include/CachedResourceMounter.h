/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <ResourceCache.h>
#include <ResourceMounter.h>

#ifdef COMPILING_CITIZEN_RESOURCES_CLIENT
#define RESCLIENT_EXPORT DLL_EXPORT
#else
#define RESCLIENT_EXPORT DLL_IMPORT
#endif

namespace fx
{
	class ResourceManager;

	class RESCLIENT_EXPORT CachedResourceMounter : public fx::ResourceMounter
	{
	private:
		std::shared_ptr<ResourceCache> m_resourceCache;

		ResourceManager* m_manager;

	public:
		CachedResourceMounter(ResourceManager* manager, const std::string& cachePath);

		virtual bool HandlesScheme(const std::string& scheme) override;

		virtual concurrency::task<fwRefContainer<fx::Resource>> LoadResource(const std::string& uri) override;

	private:
		struct ResourceFileEntry
		{
			std::string basename;
			std::string referenceHash;
			std::string remoteUrl;
			size_t size;

			inline ResourceFileEntry(const std::string& basename, const std::string& referenceHash, const std::string& remoteUrl, size_t size = 0)
				: basename(basename), referenceHash(referenceHash), remoteUrl(remoteUrl), size(size)
			{

			}
		};

	private:
		std::multimap<std::string, ResourceFileEntry> m_resourceEntries;

	public:
		void RemoveResourceEntries(const std::string& resourceName);

		void AddResourceEntry(const std::string& resourceName, const std::string& basename, const std::string& referenceHash, const std::string& remoteUrl, size_t size = 0);
	};


	RESCLIENT_EXPORT fwRefContainer<CachedResourceMounter> GetCachedResourceMounter(fx::ResourceManager* manager, const std::string& cachePath);

	struct StreamingEntryData
	{
		std::string fileName;
		std::string resourceName;
		uint32_t rscVersion;
		uint32_t rscPagesPhysical;
		uint32_t rscPagesVirtual;
	};

	extern RESCLIENT_EXPORT fwEvent<const StreamingEntryData&> OnAddStreamingResource;
}