#pragma once

#include <console/Console.Commands.h>

namespace fx
{
enum class GameName
{
	GTA5,
	RDR3
};
}

template<>
struct ConsoleArgumentType<fx::GameName>
{
	static std::string Unparse(const fx::GameName& input)
	{
		switch (input)
		{
		case fx::GameName::GTA5:
			return "gta5";
		case fx::GameName::RDR3:
			return "rdr3";
		default:
			return "unk";
		}
	}

	static bool Parse(const std::string& input, fx::GameName* out)
	{
		if (_stricmp(input.c_str(), "rdr3") == 0)
		{
			*out = fx::GameName::RDR3;
			return true;
		}
		else if (_stricmp(input.c_str(), "gta5") == 0)
		{
			*out = fx::GameName::GTA5;
			return true;
		}

		return false;
	}
};

template<>
struct ConsoleArgumentName<fx::GameName>
{
	inline static const char* Get()
	{
		return "fx::GameName";
	}
};
