#pragma once

#include "ScriptWarnings.h"

namespace fx
{
enum class ScriptDeprecations : uint8_t
{
	// The client old net id is used inside the playerConnect event, the 'source' from the script runtime should be used instead
	CLIENT_EVENT_OLD_NET_ID
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
