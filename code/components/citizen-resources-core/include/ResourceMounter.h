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
};
}
