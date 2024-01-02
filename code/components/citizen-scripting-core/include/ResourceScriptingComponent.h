/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <fxScripting.h>

#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_unordered_set.h>

#include "ResourceMetaDataComponent.h"

#ifdef COMPILING_CITIZEN_SCRIPTING_CORE
#define SCRIPTING_CORE_EXPORT DLL_EXPORT
#else
#define SCRIPTING_CORE_EXPORT DLL_IMPORT
#endif

namespace fx
{
class Resource;

class ResourceScriptingComponent final : public fwRefCountable
{
private:
	Resource* m_resource;

	fx::OMPtr<IScriptHost> m_scriptHost;

	tbb::concurrent_unordered_map<int32_t, fx::OMPtr<IScriptRuntime>> m_scriptRuntimes;

	std::unordered_map<int32_t, fx::OMPtr<IScriptTickRuntime>> m_tickRuntimes;

	tbb::concurrent_unordered_set<std::string> m_eventsHandled;

private:
	void CreateEnvironments();
	void CreateEmptyEnvironments();

public:
	ResourceScriptingComponent(Resource* resource);

	fwEvent<> OnCreatedRuntimes;

	fwEvent<const std::string&, const std::string&> OnOpenScript;

	inline fx::OMPtr<IScriptHost> GetScriptHost()
	{
		return m_scriptHost;
	}

	inline void AddRuntime(OMPtr<IScriptRuntime> runtime)
	{
		m_scriptRuntimes.insert({ runtime->GetInstanceId(), runtime });
	}

	inline OMPtr<IScriptRuntime> GetRuntimeById(int32_t instanceId)
	{
		auto it = m_scriptRuntimes.find(instanceId);

		if (it == m_scriptRuntimes.end())
		{
			return nullptr;
		}

		return it->second;
	}

	template<typename TFn>
	inline void ForAllRuntimes(const TFn&& fn)
	{
		for (auto& runtime : m_scriptRuntimes)
		{
			fn(runtime.second);
		}
	}

	inline Resource* GetResource()
	{
		return m_resource;
	}

	inline void AddHandledEvent(const std::string& eventName)
	{
		if (m_eventsHandled.find("*") == m_eventsHandled.end()) //Only adds if theres no wildcard found
		{
			m_eventsHandled.insert(eventName);
			return;
		}

		//if wildcard is found remove all others, so events doesn't get called multiple times...
		m_eventsHandled.clear(); //Not sure if clear is even valid function :D
		m_eventsHandled.insert("*");
	}

	SCRIPTING_CORE_EXPORT void Tick();
};

class ScriptMetaDataComponent : public OMClass<ScriptMetaDataComponent, IScriptHostWithResourceData, IScriptHostWithManifest>
{
public:
	// NS_DECL_ISCRIPTHOSTWITHRESOURCEDATA;

	// NS_DECL_ISCRIPTHOSTWITHMANIFEST;

private:
	Resource* m_resource;

public:
	ScriptMetaDataComponent(Resource* resource)
		: m_resource(resource)
	{
	}

	result_t OM_DECL GetResourceName(char** outResourceName)
	{
		*outResourceName = const_cast<char*>(m_resource->GetName().c_str());
		return FX_S_OK;
	}

	result_t OM_DECL GetNumResourceMetaData(char* fieldName, int32_t* numFields)
	{
		auto metaData = m_resource->GetComponent<ResourceMetaDataComponent>();

		auto entries = metaData->GetEntries(fieldName);

		*numFields = static_cast<int32_t>(std::distance(entries.begin(), entries.end()));

		return FX_S_OK;
	}

	result_t OM_DECL GetResourceMetaData(char* fieldName, int32_t fieldIndex, char** fieldValue)
	{
		auto metaData = m_resource->GetComponent<ResourceMetaDataComponent>();

		auto entries = metaData->GetEntries(fieldName);

		// and loop over the entries to see if we find anything
		int i = 0;

		for (auto& entry : entries)
		{
			if (fieldIndex == i)
			{
				*fieldValue = const_cast<char*>(entry.second.c_str());
				return FX_S_OK;
			}

			i++;
		}

		// return not-found
		return 0x80070490;
	}

	result_t OM_DECL IsManifestVersionBetween(const guid_t& lowerBound, const guid_t& upperBound, bool* _retval)
	{
		// get the manifest version
		auto metaData = m_resource->GetComponent<ResourceMetaDataComponent>();

		auto retval = metaData->IsManifestVersionBetween(lowerBound, upperBound);

		if (retval)
		{
			*_retval = *retval;

			return FX_S_OK;
		}

		return FX_E_INVALIDARG;
	}

	result_t OM_DECL IsManifestVersionV2Between(char* lowerBound, char* upperBound, bool* _retval)
	{
		// get the manifest version
		auto metaData = m_resource->GetComponent<ResourceMetaDataComponent>();

		auto retval = metaData->IsManifestVersionBetween(lowerBound, upperBound);

		if (retval)
		{
			*_retval = *retval;

			return FX_S_OK;
		}

		return FX_E_INVALIDARG;
	}
};
}

DECLARE_INSTANCE_TYPE(fx::ResourceScriptingComponent);
DECLARE_INSTANCE_TYPE(fx::ScriptMetaDataComponent);
