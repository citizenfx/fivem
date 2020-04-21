/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

//#define CPPREST_FORCE_PPLX 1
#include <pplx/pplxtasks.h>

#include <ComponentHolder.h>

#ifdef COMPILING_CITIZEN_RESOURCES_CORE
#define RESOURCES_CORE_EXPORT DLL_EXPORT
#else
#define RESOURCES_CORE_EXPORT DLL_IMPORT
#endif

namespace fx
{
class Resource;

class ResourceManager;

class ResourceManagerError
{
public:
	inline explicit ResourceManagerError(const std::string& error)
		: m_error(error)
	{

	}

	inline const std::string& Get() const
	{
		return m_error;
	}

private:
	std::string m_error;
};

enum class ResourceState
{
	Uninitialized,
	Stopped,
	Starting,
	Started,
	Stopping
};

class Resource : public fwRefCountable, public ComponentHolderImpl<Resource>
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
	// Gets the current state of the resource.
	//
	virtual ResourceState GetState() = 0;

	//
	// Loads the resource from the specified root path - compatibility wrapper.
	//
	inline bool LoadFrom(const std::string& rootPath)
	{
		return LoadFrom(rootPath, nullptr);
	}

	//
	// Loads the resource from the specified root path.
	//
	virtual bool LoadFrom(const std::string& rootPath, std::string* errorResult) = 0;

	//
	// Starts the resource.
	//
	virtual bool Start() = 0;

	//
	// Stops the resource.
	//
	virtual bool Stop() = 0;

	//
	// Executes a tick on the resource.
	//
	virtual void Tick() = 0;

	//
	// Gets a reference to the owning resource manager.
	//
	virtual ResourceManager* GetManager() = 0;

public:
	//
	// An event to handle tasks to be performed before starting a resource.
	//
	fwEvent<> OnBeforeStart;

	//
	// An event to handle tasks to be performed when starting a resource.
	//
	fwEvent<> OnStart;

	//
	// An event to handle tasks to be performed when stopping a resource.
	//
	fwEvent<> OnStop;

	//
	// An event to handle tasks to be performed when the resource ticks.
	//
	fwEvent<> OnTick;

	//
	// An event to handle tasks to be performed when a resource is created, after the game loads.
	//
	fwEvent<> OnCreate;

	//
	// An event to handle tasks to be performed when the resource becomes active.
	//
	fwEvent<> OnActivate;

	//
	// An event to handle tasks to be performed when the resource becomes inactive.
	//
	fwEvent<> OnDeactivate;

	//
	// An event to handle tasks to be performed before the resource gets deleted.
	//
	fwEvent<> OnRemove;

public:
	//
	// An event to add components to a newly-initializing resource's instance registry.
	//
	static RESOURCES_CORE_EXPORT fwEvent<Resource*> OnInitializeInstance;
};
}
