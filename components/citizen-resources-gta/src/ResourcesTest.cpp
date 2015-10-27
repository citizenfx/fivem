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

#include <ResourceCacheDevice.h>

class ExampleMounter : public fx::ResourceMounter
{
private:
	fwRefContainer<fx::ResourceManager> m_manager;

public:
	inline ExampleMounter(fwRefContainer<fx::ResourceManager> manager)
		: m_manager(manager)
	{

	}

	virtual bool HandlesScheme(const std::string& scheme) override;

	virtual concurrency::task<fwRefContainer<fx::Resource>> LoadResource(const std::string& uri) override;
};

bool ExampleMounter::HandlesScheme(const std::string& scheme)
{
	return (scheme == "gta");
}

concurrency::task<fwRefContainer<fx::Resource>> ExampleMounter::LoadResource(const std::string& uri)
{
	return concurrency::task_from_result(m_manager->CreateResource("cake", "citizen:/test/"));
}

#include <VFSManager.h>

fwRefContainer<fx::ResourceManager> g_resourceManager;

static InitFunction initFunction([] ()
{
	rage::fiDevice::OnInitialMount.Connect([] ()
	{
		//while (true)
		{
			fwRefContainer<fx::ResourceManager> manager = fx::CreateResourceManager();
			Instance<fx::ResourceManager>::Set(manager.GetRef());

			g_resourceManager = manager;

			manager->AddMounter(new ExampleMounter(manager));
			manager->AddMounter(fx::GetCachedResourceMounter("rescache:/"));

			manager->AddResource("gta:///").then([=] (fwRefContainer<fx::Resource> resource)
			{
				resource->Start();

				fwRefContainer<ResourceCacheEntryList> entryList = new ResourceCacheEntryList();
				resource->SetComponent(entryList);

				ResourceCacheEntryList::Entry entry;
				entry.remoteUrl = "http://refint.org/files/mspaint.exe";
				entry.referenceHash = "975b337f7a2576f382cd40dfcbcca994265be599";
				entry.basename = "mspaint.exe";

				entryList->AddEntry(entry);

				fwRefContainer<vfs::Stream> stream = vfs::OpenRead("cache:/cake/mspaint.exe");
				auto data = stream->Read(4096);

				//__debugbreak();
			});

			//__debugbreak();

			//Sleep(99999999);
		}
	}, 9000);
});