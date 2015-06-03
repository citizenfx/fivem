/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "IteratorView.h"

namespace fx
{
class Resource;

class ResourceMetaDataComponent : public fwRefCountable
{
private:
	Resource* m_resource;

	std::multimap<std::string, std::string> m_metaDataEntries;

public:
	inline auto GetEntries(const std::string& key)
	{
		return GetIteratorView(m_metaDataEntries.equal_range(key));
	}
};
}

DECLARE_INSTANCE_TYPE(fx::ResourceMetaDataComponent);