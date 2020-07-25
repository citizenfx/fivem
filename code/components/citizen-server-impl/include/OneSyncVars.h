#pragma once

#include <console/Console.Commands.h>

namespace fx
{
enum class OneSyncState
{
	Off,
	Legacy,
	On
};
}

template<>
struct ConsoleArgumentType<fx::OneSyncState>
{
	static std::string Unparse(const fx::OneSyncState& input)
	{
		switch (input)
		{
			case fx::OneSyncState::Off:
				return "off";
			case fx::OneSyncState::On:
				return "on";
			case fx::OneSyncState::Legacy:
				return "legacy";
			default:
				return "unk";
		}
	}

	static bool Parse(const std::string& input, fx::OneSyncState* out)
	{
		if (_stricmp(input.c_str(), "on") == 0 || _stricmp(input.c_str(), "true") == 0)
		{
			*out = fx::OneSyncState::On;
			return true;
		}
		else if (_stricmp(input.c_str(), "legacy") == 0)
		{
			*out = fx::OneSyncState::Legacy;
			return true;
		}
		else if (_stricmp(input.c_str(), "off") == 0 || _stricmp(input.c_str(), "false") == 0)
		{
			*out = fx::OneSyncState::Off;
			return true;
		}

		return false;
	}
};

template<>
struct ConsoleArgumentName<fx::OneSyncState>
{
	inline static const char* Get()
	{
		return "fx::OneSyncState";
	}
};
