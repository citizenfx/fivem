#pragma once

#include <StdInc.h>
#include "ScriptWarnings.h"

namespace fx
{
enum class ScriptDeprecations : uint8_t
{
	// Previously experimental undocumented natives, now fully removed.
	DRAW_IM_NATIVES,
	DRAW_TEXTURED_IM_VERTICES,
	DRAW_TEXTURED_IM_VERTICES_CLIPPED
};

template<ScriptDeprecations TDeprecation>
bool ShowDeprecation()
{
	static bool shown = false;
	if (shown)
	{
		return false;
	}

	shown = true;
	return true;
}

template<ScriptDeprecations TDeprecation, typename... TArgs>
void WarningDeprecationf(std::string_view channel, std::string_view format, const TArgs&... args)
{
	if (!ShowDeprecation<TDeprecation>())
	{
		return;
	}

	return scripting::Warningfv(channel, format, fmt::make_printf_args(args...));
}
}
