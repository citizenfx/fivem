/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#pragma once

#ifdef COMPILING_GTA_CORE_FIVE
#define GTA_CORE_EXPORT DLL_EXPORT
#else
#define GTA_CORE_EXPORT DLL_IMPORT
#endif

namespace game
{
	void GTA_CORE_EXPORT AddCustomText(const std::string& key, const std::string& value);

	void GTA_CORE_EXPORT AddCustomText(uint32_t hash, const std::string& value);

	void GTA_CORE_EXPORT RemoveCustomText(uint32_t hash);
}