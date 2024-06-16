#pragma once

#include "ComponentExport.h"

namespace fx
{
namespace scripting
{
	COMPONENT_EXPORT(CITIZEN_SCRIPTING_CORE)
	void Warningfv(std::string_view channel, std::string_view format, fmt::printf_args argumentList);

	template<typename... TArgs>
	inline void Warningf(std::string_view channel, std::string_view format, const TArgs&... args)
	{
		return Warningfv(channel, format, fmt::make_printf_args(args...));
	}
}
}
