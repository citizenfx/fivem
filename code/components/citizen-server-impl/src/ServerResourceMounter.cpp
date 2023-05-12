#include "StdInc.h"

#include "ResourceManager.h"
#include "ServerResourceList.h"

#include <skyr/url.hpp>
#include <skyr/percent_encode.hpp>

class ServerResourceMounter : public fx::ResourceMounter
{
public:
	ServerResourceMounter(fx::ResourceManager* manager)
		: m_manager(manager)
	{
	}

	virtual bool HandlesScheme(const std::string& scheme) override
	{
		return (scheme == "file");
	}

	virtual pplx::task<fwRefContainer<fx::Resource>> LoadResource(const std::string& uri) override
	{
		auto resourceList = m_manager->GetComponent<fx::resources::ServerResourceList>();
		auto uriParsed = skyr::make_url(uri);

		fwRefContainer<fx::Resource> resource;

		if (uriParsed)
		{
			auto pathRef = uriParsed->pathname();
			auto fragRef = uriParsed->hash().substr(1);

			if (!pathRef.empty() && !fragRef.empty())
			{
#ifdef _WIN32
				std::string pr = pathRef.substr(1);
#else
				std::string pr = pathRef;
#endif

				std::string error;

				resource = m_manager->CreateResource(fragRef, this);
				if (!resource->LoadFrom(*skyr::percent_decode(pr), &error))
				{
					// error matching LuaMetaDataLoader.cpp in citizen:resources:metadata:lua
					if (error == "Could not open resource metadata file - no such file.")
					{
						resourceList->AddError(fx::resources::ScanMessageType::Warning, fragRef, "no_manifest", {});
					}
					else
					{
						resourceList->AddError(fx::resources::ScanMessageType::Error, fragRef, "load_failed", { error });
					}

					m_manager->RemoveResource(resource);
					resource = nullptr;
				}
			}
		}

		return pplx::task_from_result<fwRefContainer<fx::Resource>>(resource);
	}

private:
	fx::ResourceManager* m_manager;
};

DLL_EXPORT fwRefContainer<fx::ResourceMounter> MakeServerResourceMounter(const fwRefContainer<fx::ResourceManager>& resman)
{
	return new ServerResourceMounter(resman.GetRef());
}
