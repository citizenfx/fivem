/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "ComponentExport.h"

#include <optional>
#include <string>
#include <unordered_map>

namespace fx
{
	class PoolSizeManager
	{
		static std::optional<std::unordered_map<std::string, uint32_t>> limits;

		static std::unordered_map<std::string, uint32_t> sizeIncrease;

	public:
#ifndef IS_FXSERVER
		static void FetchIncreaseRequest();

		COMPONENT_EXPORT(POOL_SIZES_STATE) static const std::unordered_map<std::string, uint32_t>& GetIncreaseRequest();
#endif

		COMPONENT_EXPORT(POOL_SIZES_STATE) static bool LimitsLoaded();

		COMPONENT_EXPORT(POOL_SIZES_STATE) static void FetchLimits(const std::string& limitsFileUrl, bool wait);

		COMPONENT_EXPORT(POOL_SIZES_STATE) static std::optional<std::string> Validate(const std::string& poolName, uint32_t sizeIncrease);

		COMPONENT_EXPORT(POOL_SIZES_STATE) static std::optional<std::string> Validate(const std::unordered_map<std::string, uint32_t>& increaseRequest);
	};
}
