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
	virtual concurrency::task<fwRefContainer<Resource>> LoadResource(const std::string& uri) = 0;
};
}