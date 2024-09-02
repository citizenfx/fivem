#pragma once

#include <optional>
#include <string>
#include <unordered_map>

namespace fx
{
	namespace client
	{
		class PoolSizeManager
		{
		public:
			static const char* LIMITS_FILE_PATH;

			static const std::unordered_map<std::string, uint32_t>& GetIncreaseRequest();

			static std::optional<std::string> Validate(const std::unordered_map<std::string, uint32_t>& increaseRequest);
		};
	}
}
