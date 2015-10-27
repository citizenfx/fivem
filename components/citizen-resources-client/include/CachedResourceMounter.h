/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <ResourceMounter.h>

#ifdef COMPILING_CITIZEN_RESOURCES_CLIENT
#define RESCLIENT_EXPORT DLL_EXPORT
#else
#define RESCLIENT_EXPORT DLL_IMPORT
#endif

namespace fx
{
	RESCLIENT_EXPORT fwRefContainer<ResourceMounter> GetCachedResourceMounter(const std::string& cachePath);
}