/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <ppltasks.h>

#include <Resource.h>
#include <ResourceMounter.h>

#include <ComponentHolder.h>

#ifdef COMPILING_CITIZEN_RESOURCES_CORE
#define RESOURCES_CORE_EXPORT DLL_EXPORT
#else
#define RESOURCES_CORE_EXPORT DLL_IMPORT
#endif

namespace fx
{
class ResourceManager : public fwRefCountable, public ComponentHolderImpl<ResourceManager>
{
public:
	//
	// Adds a resource to the resource manager from the passed resource URI.
	//
	virtual concurrency::task<fwRefContainer<Resource>> AddResource(const std::string& uri) = 0;

	//
	// Gets the mounter that is responsible for handling a particular resource URI.
	//
	virtual fwRefContainer<ResourceMounter> GetMounterForUri(const std::string& uri) = 0;

	//
	// Obtains a reference to the resource with the passed identity string.
	//
	virtual fwRefContainer<Resource> GetResource(const std::string& identifier) = 0;

	//
	// Iterates over all registered resources.
	//
	virtual void ForAllResources(const std::function<void(fwRefContainer<Resource>)>& function) = 0;

	//
	// Stops and unloads all registered resources.
	//
	virtual void ResetResources() = 0;

	//
	// Registers a resource mounter.
	//
	virtual void AddMounter(fwRefContainer<ResourceMounter> mounter) = 0;

	//
	// Remove and destroys a resource.
	//
	virtual void RemoveResource(fwRefContainer<Resource> resource) = 0;

	//
	// For use in resource mounters, creates a resource with the passed identity.
	//
	virtual fwRefContainer<Resource> CreateResource(const std::string& resourceName) = 0;

	//
	// Executes a single tick for the resource manager.
	//
	virtual void Tick() = 0;

public:
	fwEvent<> OnTick;

public:
	//
	// An event to add components to a newly-initializing resource manager's instance registry.
	//
	static RESOURCES_CORE_EXPORT fwEvent<ResourceManager*> OnInitializeInstance;
};

RESOURCES_CORE_EXPORT ResourceManager* CreateResourceManager();
}

DECLARE_INSTANCE_TYPE(fx::ResourceManager);