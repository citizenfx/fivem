/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Resource.h"
#include "ResourceMetaDataComponent.h"
#include "ResourceScriptingComponent.h"

namespace fx
{
ResourceScriptingComponent::ResourceScriptingComponent(Resource* resource)
	: m_resource(resource)
{
	resource->OnStart.Connect([=] ()
	{
		// pre-emptively instantiate all scripting environments
		std::vector<OMPtr<IScriptFileHandlingRuntime>> environments;

		{
			guid_t clsid;
			intptr_t findHandle = fxFindFirstImpl(IScriptFileHandlingRuntime::GetIID(), &clsid);

			if (findHandle != 0)
			{
				do
				{
					OMPtr<IScriptFileHandlingRuntime> ptr;

					if (SUCCEEDED(MakeInterface(&ptr, clsid)))
					{
						environments.push_back(ptr);
					}
				} while (fxFindNextImpl(findHandle, &clsid));

				fxFindImplClose(findHandle);
			}
		}

		// get metadata and list scripting environments we *do* want to use
		{
			fwRefContainer<ResourceMetaDataComponent> metaData = resource->GetComponent<ResourceMetaDataComponent>();
			auto clientScripts = metaData->GetEntries("client_script");

			for (auto it = environments.begin(); it != environments.end(); )
			{
				OMPtr<IScriptFileHandlingRuntime> ptr = *it;
				bool environmentUsed = false;

				for (auto& clientScript : clientScripts)
				{
					if (ptr->HandlesFile(const_cast<char*>(clientScript.second.c_str())))
					{
						environmentUsed = true;
						break;
					}
				}

				if (!environmentUsed)
				{
					it = environments.erase(it);
				}
				else
				{
					it++;
				}
			}
		}

		// assign them to ourselves and create them
		for (auto& environment : environments)
		{
			OMPtr<IScriptRuntime> ptr;
			if (SUCCEEDED(environment.As(&ptr)))
			{
				m_scriptRuntimes[ptr->GetInstanceId()] = ptr;
			}
		}

		if (!m_scriptRuntimes.empty())
		{
			CreateEnvironments();
		}
	});
}

OMPtr<IScriptHost> GetScriptHostForResource(Resource* resource);

void ResourceScriptingComponent::CreateEnvironments()
{
	m_scriptHost = GetScriptHostForResource(m_resource);

	for (auto& environment : m_scriptRuntimes)
	{
		environment.second->Create(m_scriptHost.GetRef());
	}
}
}

static InitFunction initFunction([] ()
{
	fx::Resource::OnInitializeInstance.Connect([] (fx::Resource* resource)
	{
		resource->SetComponent<fx::ResourceScriptingComponent>(new fx::ResourceScriptingComponent(resource));
	});
});