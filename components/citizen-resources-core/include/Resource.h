/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <ppltasks.h>


#ifdef COMPILING_CITIZEN_RESOURCES_CORE
#define RESOURCES_CORE_EXPORT DLL_EXPORT
#else
#define RESOURCES_CORE_EXPORT DLL_IMPORT
#endif

namespace fx
{
class Resource : public fwRefCountable
{
public:
	//
	// Gets the resource name.
	//
	virtual const std::string& GetName() = 0;

	//
	// Gets the fully-qualified identifier of the resource.
	//
	virtual const std::string& GetIdentifier() = 0;

	//
	// Gets the root path of the resource in the application file system.
	//
	virtual const std::string& GetPath() = 0;

	//
	// Starts the resource.
	//
	virtual bool Start() = 0;

	//
	// Stops the resource.
	//
	virtual bool Stop() = 0;

	//
	// Gets the resource-specific instance registry.
	//
	virtual fwRefContainer<RefInstanceRegistry> GetInstanceRegistry() = 0;

	//
	// Utility function to get an instance of a particular interface from the instance registry.
	//
	template<typename TInstance>
	fwRefContainer<TInstance> GetComponent()
	{
		return Instance<TInstance>::Get(GetInstanceRegistry());
	}

	//
	// Utility function to set an instance of a particular interface in the instance registry.
	//
	template<typename TInstance>
	void SetComponent(fwRefContainer<TInstance> inst)
	{
		Instance<TInstance>::Set(inst, GetInstanceRegistry());
	}

public:
	//
	// An event to handle tasks to be performed when starting a resource.
	//
	fwEvent<> OnStart;

public:
	//
	// An event to add components to a newly-initializing resource's instance registry.
	//
	static RESOURCES_CORE_EXPORT fwEvent<Resource*> OnInitializeInstance;
};
}