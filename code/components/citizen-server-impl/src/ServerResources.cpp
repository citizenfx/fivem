#include "StdInc.h"

#include <ResourceManager.h>
#include <ServerInstanceBase.h>

#include <RelativeDevice.h>

#include <VFSManager.h>

#include <network/uri.hpp>

class LocalResourceMounter : public fx::ResourceMounter
{
public:
	LocalResourceMounter(fx::ResourceManager* manager)
		: m_manager(manager)
	{
		
	}

	virtual bool HandlesScheme(const std::string& scheme) override
	{
		return (scheme == "file");
	}

	virtual concurrency::task<fwRefContainer<fx::Resource>> LoadResource(const std::string& uri) override
	{
		std::error_code ec;
		auto uriParsed = network::make_uri(uri, ec);

		fwRefContainer<fx::Resource> resource;

		if (!ec)
		{
			auto pathRef = uriParsed.path();
			auto fragRef = uriParsed.fragment();

			if (pathRef && fragRef)
			{
				std::vector<char> path;
				std::string pr = pathRef->substr(1).to_string();
				network::uri::decode(pr.begin(), pr.end(), std::back_inserter(path));

				resource = m_manager->CreateResource(fragRef->to_string());
				resource->LoadFrom(std::string(path.begin(), path.begin() + path.size()));
				resource->Start();

				m_manager->Tick();
				m_manager->Tick();
			}
		}

		return concurrency::task_from_result<fwRefContainer<fx::Resource>>(resource);
	}

private:
	fx::ResourceManager* m_manager;
};

static InitFunction initFunction([]()
{
	fx::ServerInstanceBase::OnServerCreate.Connect([](fx::ServerInstanceBase* instance)
	{
		instance->SetComponent(fx::CreateResourceManager());

		vfs::Mount(new vfs::RelativeDevice("C:/fivem/data/citizen/"), "citizen:/");

		fwRefContainer<fx::ResourceManager> resman = instance->GetComponent<fx::ResourceManager>();
		resman->AddMounter(new LocalResourceMounter(resman.GetRef()));
		resman->AddResource("file:///C:/cfx-server-data/resources/%5Bsystem%5D/spawnmanager#spawnmanager");
	});
});