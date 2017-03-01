/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Resource.h"
#include "ResourceEventComponent.h"
#include "ResourceMetaDataComponent.h"
#include "ResourceScriptingComponent.h"

#include <Error.h>

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
				std::unique_lock<std::recursive_mutex> lock(m_scriptRuntimesLock);

				ptr->SetParentObject(resource);

				m_scriptRuntimes[ptr->GetInstanceId()] = ptr;
			}
		}

		if (!m_scriptRuntimes.empty())
		{
			CreateEnvironments();
		}
	});

	resource->OnTick.Connect([=] ()
	{
		std::unique_lock<std::recursive_mutex> lock(m_scriptRuntimesLock);

		for (auto& environment : m_scriptRuntimes)
		{
			OMPtr<IScriptTickRuntime> tickRuntime;

			if (FX_SUCCEEDED(environment.second.As(&tickRuntime)))
			{
				tickRuntime->Tick();
			}
		}
	});

	resource->OnStop.Connect([=] ()
	{
		std::unique_lock<std::recursive_mutex> lock(m_scriptRuntimesLock);

		for (auto& environment : m_scriptRuntimes)
		{
			environment.second->Destroy();
		}

		m_scriptRuntimes.clear();
	});
}

OMPtr<IScriptHost> GetScriptHostForResource(Resource* resource);

void ResourceScriptingComponent::CreateEnvironments()
{
	std::unique_lock<std::recursive_mutex> lock(m_scriptRuntimesLock);

	m_scriptHost = GetScriptHostForResource(m_resource);
	
	for (auto& environment : m_scriptRuntimes)
	{
		auto hr = environment.second->Create(m_scriptHost.GetRef());

		if (FX_FAILED(hr))
		{
			FatalError("Failed to create environment, hresult %x", hr);
		}
	}

	// iterate over the runtimes and load scripts as requested
	for (auto& environment : m_scriptRuntimes)
	{
		OMPtr<IScriptFileHandlingRuntime> ptr;

		fwRefContainer<ResourceMetaDataComponent> metaData = m_resource->GetComponent<ResourceMetaDataComponent>();
		auto clientScripts = metaData->GetEntries("client_script");
		
		if (FX_SUCCEEDED(environment.second.As(&ptr)))
		{
			bool environmentUsed = false;

			for (auto& clientScript : clientScripts)
			{
				if (ptr->HandlesFile(const_cast<char*>(clientScript.second.c_str())))
				{
					result_t hr = ptr->LoadFile(const_cast<char*>(clientScript.second.c_str()));

					if (FX_FAILED(hr))
					{
						trace("Failed to load client script %s.\n", clientScript.second.c_str());
					}
				}
			}
		}
	}

	// initialize event handler calls
	fwRefContainer<ResourceEventComponent> eventComponent = m_resource->GetComponent<ResourceEventComponent>();

	assert(eventComponent.GetRef());

	if (eventComponent.GetRef())
	{
		// pre-cache event-handling runtimes
		std::vector<OMPtr<IScriptEventRuntime>> eventRuntimes;

		for (auto& environment : m_scriptRuntimes)
		{
			OMPtr<IScriptEventRuntime> ptr;

			if (FX_SUCCEEDED(environment.second.As(&ptr)))
			{
				eventRuntimes.push_back(ptr);
			}
		}

		// add the event
		eventComponent->OnTriggerEvent.Connect([=] (const std::string& eventName, const std::string& eventPayload, const std::string& eventSource, bool* eventCanceled)
		{
			// invoke the event runtime
			for (auto&& runtime : eventRuntimes)
			{
				result_t hr;

				if (FX_FAILED(hr = runtime->TriggerEvent(const_cast<char*>(eventName.c_str()), const_cast<char*>(eventPayload.c_str()), eventPayload.size(), const_cast<char*>(eventSource.c_str()))))
				{
					trace("Failed to execute event %s - %08x.\n", eventName.c_str(), hr);
				}
			}
		});
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