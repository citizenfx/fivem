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
class ResourceManagerImpl;

class ResourceImpl final : public Resource
{
private:
	std::string m_name;

	std::string m_rootPath;

	ResourceManagerImpl* m_manager;

	ResourceState m_state;

public:
	ResourceImpl(const std::string& name, ResourceManagerImpl* manager);

	virtual bool LoadFrom(const std::string& rootPath, std::string* errorState) override;

	virtual const std::string& GetName() override;

	virtual const std::string& GetIdentifier() override;

	virtual const std::string& GetPath() override;

	virtual ResourceState GetState() override;

	virtual bool Start() override;

	virtual bool ClientReloadFile() override;

	virtual bool Stop() override;

	virtual void Run(std::function<void()>&& fn) override;

	virtual ResourceManager* GetManager() override;

public:
	void Destroy();
};
}
