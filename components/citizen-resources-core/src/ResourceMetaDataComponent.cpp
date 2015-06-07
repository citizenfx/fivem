/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ResourceMetaDataComponent.h"

namespace fx
{
ResourceMetaDataComponent::ResourceMetaDataComponent(Resource* resourceRef)
	: m_resource(resourceRef), m_metaDataLoader(nullptr)
{

}

boost::optional<std::string> ResourceMetaDataComponent::LoadMetaData(const std::string& resourcePath)
{
	assert(m_metaDataLoader.GetRef());

	return m_metaDataLoader->LoadMetaData(this, resourcePath);
}
}