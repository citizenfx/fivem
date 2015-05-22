/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <ResourceImpl.h>

#include <ResourceMetaDataComponent.h>

namespace fx
{
ResourceImpl::ResourceImpl(const std::string& name, ResourceManagerImpl* manager)
	: m_name(name), m_manager(manager)
{
	OnInitializeInstance(this);
}

bool ResourceImpl::LoadFrom(const std::string& rootPath)
{
	auto metaData = GetComponent<ResourceMetaDataComponent>();

	return true;
}

const std::string& ResourceImpl::GetName()
{
	return m_name;
}

const std::string& ResourceImpl::GetIdentifier()
{
	return m_name;
}

bool ResourceImpl::Start()
{
	return true;
}

bool ResourceImpl::Stop()
{
	return true;
}

fwRefContainer<RefInstanceRegistry> ResourceImpl::GetInstanceRegistry()
{
	return m_instanceRegistry;
}

fwEvent<Resource*> Resource::OnInitializeInstance;
}