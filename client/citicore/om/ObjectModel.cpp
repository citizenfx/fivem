/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include <ComponentLoader.h>

#include <om/core.h>
#include <om/OMComponent.h>

struct FindHandle
{
	std::vector<guid_t> clsids;
	std::vector<guid_t>::iterator cursor;
};

namespace
{
	void ForAllOMComponents(const std::function<void(OMComponent*)>& cb)
	{
		ComponentLoader* loader = ComponentLoader::GetInstance();

		loader->ForAllComponents([&] (fwRefContainer<ComponentData> data)
		{
			auto& instances = data->GetInstances();

			if (!instances.empty())
			{
				fwRefContainer<Component> instance = instances[0];
				OMComponent* omInstance = dynamic_cast<OMComponent*>(instance.GetRef());

				if (omInstance != nullptr)
				{
					cb(omInstance);
				}
			}
		});
	}
}

extern "C" CORE_EXPORT intptr_t fxFindFirstImpl(const guid_t& iid, guid_t* clsid)
{
	// allocate a list of returned class IDs
	std::vector<guid_t> clsids;

	// loop through each component in the global component loader
	ForAllOMComponents([&] (OMComponent* component)
	{
		auto implementedClasses = component->GetImplementedClasses(iid);

		clsids.insert(clsids.begin(), implementedClasses.begin(), implementedClasses.end());
	});

	// if the list isn't empty...
	if (!clsids.empty())
	{
		// allocate a find handle, fill it and return it
		FindHandle* handle = new FindHandle;

		handle->clsids = std::move(clsids);
		handle->cursor = handle->clsids.begin();

		*clsid = *handle->cursor;

		return reinterpret_cast<intptr_t>(handle);
	}

	// return a null handle otherwise
	return 0;
}

extern "C" CORE_EXPORT int32_t fxFindNextImpl(intptr_t findHandle, guid_t* clsid)
{
	// verify the handle for not being null
	if (findHandle == 0)
	{
		// return a failure result
		return false;
	}

	// cast as a find handle
	FindHandle* handle = reinterpret_cast<FindHandle*>(findHandle);

	// if the iterator reached the end, bail out
	if (handle->cursor == handle->clsids.end())
	{
		return false;
	}

	handle->cursor++;

	// if the *incremented* iterator reached the end, bail out again
	if (handle->cursor == handle->clsids.end())
	{
		return false;
	}

	// write out the ID to the argument and return success
	*clsid = *handle->cursor;

	return true;
}

extern "C" CORE_EXPORT void fxFindImplClose(intptr_t findHandle)
{
	// delete the cast handle
	delete reinterpret_cast<FindHandle*>(findHandle);
}

extern "C" result_t fxCreateObjectInstance(const guid_t& guid, const guid_t& iid, void** objectRef)
{
	// look through the object list and try to create an instance until it succeeds or fails *badly*
	result_t result = FX_E_NOINTERFACE;

	ForAllOMComponents([&] (OMComponent* component)
	{
		if (result != FX_S_OK)
		{
			result = component->CreateObjectInstance(guid, iid, objectRef);
		}
	});

	return result;
}