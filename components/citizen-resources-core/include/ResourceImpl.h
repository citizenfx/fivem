/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <Resource.h>

namespace fx
{
enum class ResourceState
{
	Uninitialized,
	Stopped,
	Starting,
	Started,
	Stopping
};

class ResourceManagerImpl;

class ResourceImpl : public Resource
{
private:
	std::string m_name;

	std::string m_rootPath;

	ResourceManagerImpl* m_manager;

	fwRefContainer<RefInstanceRegistry> m_instanceRegistry;

	ResourceState m_state;

public:
	ResourceImpl(const std::string& name, ResourceManagerImpl* manager);

	bool LoadFrom(const std::string& rootPath);

	virtual const std::string& GetName() override;

	virtual const std::string& GetIdentifier() override;

	virtual const std::string& GetPath() override;

	virtual bool Start() override;

	virtual bool Stop() override;

	virtual fwRefContainer<RefInstanceRegistry> GetInstanceRegistry() override;
};
}