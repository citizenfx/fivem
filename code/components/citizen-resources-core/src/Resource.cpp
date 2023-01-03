/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <ResourceImpl.h>
#include <ResourceManagerImpl.h>

#include <DebugAlias.h>

#include <ResourceMetaDataComponent.h>

#include <Error.h>

namespace fx
{
ResourceImpl::ResourceImpl(const std::string& name, ResourceManagerImpl* manager)
	: m_name(name), m_manager(manager), m_state(ResourceState::Uninitialized)
{
	OnInitializeInstance(this);
}

bool ResourceImpl::LoadFrom(const std::string& rootPath, std::string* errorState)
{
	fwRefContainer<ResourceMetaDataComponent> metaData = GetComponent<ResourceMetaDataComponent>();

	if (m_state != ResourceState::Uninitialized)
	{
		return true;
	}

	auto retval = metaData->LoadMetaData(rootPath);

	if (retval)
	{
		if (errorState)
		{
			*errorState = *retval;
		}
		else
		{
			trace("Resource loading for %s failed:\n%s\n", m_name.c_str(), retval->c_str());
		}
	}

	m_rootPath = rootPath;

	m_state = ResourceState::Stopped;

	if (!retval)
	{
		OnLoad();
	}

	return !retval.is_initialized();
}

const std::string& ResourceImpl::GetName()
{
	return m_name;
}

const std::string& ResourceImpl::GetIdentifier()
{
	return m_name;
}

const std::string& ResourceImpl::GetPath()
{
	return m_rootPath;
}

ResourceState ResourceImpl::GetState()
{
	return m_state;
}

bool ResourceImpl::Start()
{
	m_manager->MakeCurrent();

	if (m_state != ResourceState::Started)
	{
		// skip the starting stage if we're already started
		if (m_state != ResourceState::Starting)
		{
			m_state = ResourceState::Starting;

			if (!OnBeforeStart())
			{
				if (m_state != ResourceState::Started)
				{
					m_state = ResourceState::Stopped;
				}

				return false;
			}
		}

		if (!OnStart())
		{
			if (m_state != ResourceState::Started)
			{
				m_state = ResourceState::Stopped;
			}

			return false;
		}
	}

	m_state = ResourceState::Started;

	return true;
}

bool ResourceImpl::Stop()
{
	m_manager->MakeCurrent();

	if (m_state != ResourceState::Stopped)
	{
		if (!OnStop())
		{
			return false;
		}

		m_state = ResourceState::Stopped;
	}

	return true;
}

void ResourceImpl::Run(std::function<void()>&& fn)
{
	if (m_state != ResourceState::Started)
	{
		return;
	}

	// save data in case we need to trace this back from dumps
	char resourceNameBit[128];
	memcpy(resourceNameBit, GetName().c_str(), std::min(std::size(resourceNameBit), GetName().length() + 1));
	debug::Alias(resourceNameBit);

	OnEnter();
	fn();

	// #TODO: unwind-dtor
	OnLeave();
}

void ResourceImpl::Destroy()
{
	m_state = ResourceState::Uninitialized;

	OnRemove();
}

ResourceManager* ResourceImpl::GetManager()
{
	return m_manager;
}

fwEvent<Resource*> Resource::OnInitializeInstance;
}
