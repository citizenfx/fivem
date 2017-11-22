/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

#include <Resource.h>

namespace fx
{
class ResourceManagerImpl;

class ResourceImpl : public Resource
{
private:
	std::string m_name;

	std::string m_rootPath;

	ResourceManagerImpl* m_manager;

	ResourceState m_state;

public:
	ResourceImpl(const std::string& name, ResourceManagerImpl* manager);

	virtual bool LoadFrom(const std::string& rootPath) override;

	virtual const std::string& GetName() override;

	virtual const std::string& GetIdentifier() override;

	virtual const std::string& GetPath() override;

	virtual ResourceState GetState() override;

	virtual bool Start() override;

	virtual bool Stop() override;

	virtual void Tick() override;

	virtual ResourceManager* GetManager() override;

public:
	void Destroy();
};
}