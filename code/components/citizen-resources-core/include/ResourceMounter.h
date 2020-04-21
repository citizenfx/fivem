/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <Resource.h>

#include <tl/expected.hpp>

namespace fx
{
class ResourceMounter : public fwRefCountable
{
public:
	//
	// If overridden, returns whether this resource mounter handles the passed scheme.
	//
	virtual bool HandlesScheme(const std::string& scheme) = 0;

	//
	// If overridden, loads the resource matching a a particular URI.
	//
	virtual pplx::task<fwRefContainer<Resource>> LoadResource(const std::string& uri) = 0;

	//
	// If overridden, loads the resource matching a a particular URI, propagating an error return value.
	//
	inline virtual pplx::task<tl::expected<fwRefContainer<Resource>, ResourceManagerError>> LoadResourceWithError(const std::string& uri)
	{
		return LoadResource(uri)
			.then([](fwRefContainer<Resource> resource) -> tl::expected<fwRefContainer<Resource>, ResourceManagerError>
		{
			if (!resource.GetRef())
			{
				return tl::make_unexpected(ResourceManagerError{ "Null resource return value." });
			}

			return resource;
		});
	}

protected:
	//
	// Implementation helper for implementing LoadResource on top of LoadResourceWithError.
	//
	inline pplx::task<fwRefContainer<Resource>> LoadResourceFallback(const std::string& uri)
	{
		return LoadResourceWithError(uri)
			.then([](tl::expected<fwRefContainer<Resource>, ResourceManagerError> result)
		{
			return result.value_or(nullptr);
		});
	}
};
}
