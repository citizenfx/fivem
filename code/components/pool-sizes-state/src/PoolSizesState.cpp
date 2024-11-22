/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>

#include "PoolSizesState.h"

#include "CoreConsole.h"
#include "HttpClient.h"
#include "Utils.h"

#ifndef IS_FXSERVER
#include "Error.h"
#endif

#include <json.hpp>

namespace fx
{
	std::optional<std::unordered_map<std::string, uint32_t>> PoolSizeManager::limits = std::nullopt;

	std::unordered_map<std::string, uint32_t> PoolSizeManager::sizeIncrease = std::unordered_map<std::string, uint32_t>();

#ifndef IS_FXSERVER
	void PoolSizeManager::FetchIncreaseRequest()
	{
		std::wstring fpath = MakeRelativeCitPath(L"VMP.ini");
		if (GetFileAttributes(fpath.c_str()) != INVALID_FILE_ATTRIBUTES)
		{
			wchar_t requestRaw[2048];
			GetPrivateProfileString(L"Game", L"PoolSizesIncrease", L"", requestRaw, _countof(requestRaw), fpath.c_str());
			std::string request = ToNarrow(requestRaw);
			if (!request.empty())
			{
				sizeIncrease = nlohmann::json::parse(request).get<std::unordered_map<std::string, uint32_t>>();
			}
		}
	}

	const std::unordered_map<std::string, uint32_t>& PoolSizeManager::GetIncreaseRequest()
	{
		return sizeIncrease;
	}
#endif

	bool PoolSizeManager::LimitsLoaded()
	{
		return limits.has_value();
	}

	void PoolSizeManager::FetchLimits(const std::string& limitsFileUrl, bool wait = false)
	{
		auto httpRequestPrt = Instance<HttpClient>::Get()->DoGetRequest(limitsFileUrl, [](bool success, const char* data, size_t length)
		{
			if (success)
			{
				try
				{
					limits = nlohmann::json::parse(data, data + length).get<std::unordered_map<std::string, uint32_t>>();
				}
				catch (std::exception& e)
				{
					trace("Error occured while parsing pool size limits json file: %s.\n", e.what());
				}
			}
		});

		if (wait)
		{
			httpRequestPrt->Wait();
		}
	}

	std::optional<std::string> PoolSizeManager::ValidateImpl(const std::string& poolName, uint32_t sizeIncrease)
	{
		if (!LimitsLoaded())
		{
			return "could not load pool size limits from cfx server, pool size changes can not be validated and therefore are not allowed";
		}

		auto it = limits->find(poolName);
		if (it == limits->end())
		{
			return fmt::sprintf("it is not allowed to change size of pool %s", poolName);
		}

		if (sizeIncrease > it->second)
		{
			return fmt::sprintf("requested size increase %d for pool %s exceeds allowed limit of %d", sizeIncrease, poolName, it->second);
		}

		return std::nullopt;
	}

	std::optional<std::string> PoolSizeManager::Validate(const std::string& poolName, uint32_t sizeIncrease)
	{
		static ConVar<int> moo("moo", ConVar_None, 0);

		bool skipValidation = moo.GetValue() == 31337;

		auto validationError = ValidateImpl(poolName, sizeIncrease);
		if (validationError.has_value())
		{
			if (!skipValidation)
			{
				return validationError;
			}

			trace(
				"Pool size increase validation failed: %s. However the \"moo 31337\" is set, so the validation error will be ignored. "
				"Unexpected problems may occur. Only use it for debugging purposes. If pool size limits change is needed - reach out to CFX team.\n",
				validationError.value()
			);

#ifndef IS_FXSERVER
			AddCrashometry("invalid_pool_size_increase_used", "%s: %d", poolName, sizeIncrease);
#endif
		}

		return std::nullopt;
	}

	std::optional<std::string> PoolSizeManager::Validate(const std::unordered_map<std::string, uint32_t>& increaseRequest)
	{
		for (const auto& [name, sizeIncrease] : increaseRequest)
		{
			std::optional<std::string> validationError = Validate(name, sizeIncrease);
			if (validationError.has_value())
			{
				return validationError;
			}
		}

		return std::nullopt;
	}
}

#ifndef IS_FXSERVER
static InitFunction initFunction([]()
{
#ifdef GTA_FIVE
	std::string limitsFileUrl = "https://vmp.724548.ir.cdn.ir/mirrors/client/pool-size-limits/fivem.json";
#else
	std::string limitsFileUrl = "https://vmp.724548.ir.cdn.ir/mirrors/client/pool-size-limits/redm.json";
#endif
	fx::PoolSizeManager::FetchLimits(limitsFileUrl);
	fx::PoolSizeManager::FetchIncreaseRequest();
});
#endif
