/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <ResourceCache.h>
#include <ResourceMounter.h>

#include <VFSDevice.h>

#include <network/uri.hpp>

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
	protected:
		std::shared_ptr<ResourceCache> m_resourceCache;

		ResourceManager* m_manager;

	protected:
		CachedResourceMounter(ResourceManager* manager);

	public:
		CachedResourceMounter(ResourceManager* manager, const std::string& cachePath);

		virtual bool HandlesScheme(const std::string& scheme) override;

		virtual concurrency::task<fwRefContainer<fx::Resource>> LoadResource(const std::string& uri) override;

	protected:
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

	protected:
		virtual fwRefContainer<fx::Resource> InitializeLoad(const std::string& uri, network::uri* parsedUri);

		virtual fwRefContainer<vfs::Device> OpenResourcePackfile(const fwRefContainer<fx::Resource>& resource);

	protected:
		std::multimap<std::string, ResourceFileEntry> m_resourceEntries;

	public:
		virtual void RemoveResourceEntries(const std::string& resourceName);

		virtual void AddResourceEntry(const std::string& resourceName, const std::string& basename, const std::string& referenceHash, const std::string& remoteUrl, size_t size = 0);

		virtual std::string FormatPath(const std::string& resourceName, const std::string& basename);
	};


	RESCLIENT_EXPORT fwRefContainer<CachedResourceMounter> GetCachedResourceMounter(fx::ResourceManager* manager, const std::string& cachePath);

	struct StreamingEntryData
	{
		std::string resourceName;
		std::string filePath;
		uint32_t rscVersion;
		uint32_t rscPagesPhysical;
		uint32_t rscPagesVirtual;
	};

	extern RESCLIENT_EXPORT fwEvent<const StreamingEntryData&> OnAddStreamingResource;
}