
#include <StdInc.h>

#include "PoolSizesState.h"

#include "Utils.h"

#include <json.hpp>
#include <fstream>

namespace fx
{
	namespace client
	{
		const char* PoolSizeManager::LIMITS_FILE_PATH = "data\\cache\\pool-size-limits.json";

		const std::unordered_map<std::string, uint32_t>& PoolSizeManager::GetIncreaseRequest()
		{
			static std::optional<std::unordered_map<std::string, uint32_t>> sizeIncrease;

			if (sizeIncrease.has_value())
			{
				return sizeIncrease.value();
			}

			sizeIncrease = std::unordered_map<std::string, uint32_t>();

			std::wstring fpath = MakeRelativeCitPath(L"CitizenFX.ini");
			if (GetFileAttributes(fpath.c_str()) != INVALID_FILE_ATTRIBUTES)
			{
				wchar_t requestRaw[1024];
				GetPrivateProfileString(L"Game", L"PoolSizesIncrease", L"", requestRaw, _countof(requestRaw), fpath.c_str());
				std::string request = ToNarrow(requestRaw);
				if (!request.empty())
				{
					sizeIncrease = nlohmann::json::parse(request).get<std::unordered_map<std::string, uint32_t>>();
				}
			}

			return sizeIncrease.value();
		}

		std::optional<std::string> PoolSizeManager::Validate(const std::unordered_map<std::string, uint32_t>& increaseRequest)
		{
			static std::optional<std::unordered_map<std::string, uint32_t>> limits;

			if (increaseRequest.empty())
			{
				// No pool size changes are requested. Validation is not needed.
				return std::nullopt;
			}

			if (!limits.has_value())
			{
				std::ifstream limitsFile(MakeRelativeCitPath(LIMITS_FILE_PATH));

				if (limitsFile.is_open())
				{
					limits = nlohmann::json::parse(limitsFile).get<std::unordered_map<std::string, uint32_t>>();
				}
				else
				{
					return "could not load pool size limits from cfx server, pool size changes can not be validated and therefore are not allowed";
				}
			}

			for (const auto& [name, sizeIncrease] : increaseRequest)
			{
				auto it = limits->find(name);
				if (it == limits->end())
				{
					return fmt::sprintf("it is not allowed to change size of pool %s", name);
				}

				if (sizeIncrease > it->second)
				{
					return fmt::sprintf("requested size increase %d for pool %s exceeds allowed limit of %d", sizeIncrease, name, it->second);
				}
			}

			return std::nullopt;
		}
	}
}
