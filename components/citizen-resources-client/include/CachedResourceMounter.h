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

			inline ResourceFileEntry(const std::string& basename, const std::string& referenceHash, const std::string& remoteUrl)
				: basename(basename), referenceHash(referenceHash), remoteUrl(remoteUrl)
			{

			}
		};

	private:
		std::multimap<std::string, ResourceFileEntry> m_resourceEntries;

	public:
		void RemoveResourceEntries(const std::string& resourceName);

		void AddResourceEntry(const std::string& resourceName, const std::string& basename, const std::string& referenceHash, const std::string& remoteUrl);
	};


	RESCLIENT_EXPORT fwRefContainer<CachedResourceMounter> GetCachedResourceMounter(fx::ResourceManager* manager, const std::string& cachePath);
}