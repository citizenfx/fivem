#pragma once

#include <console/Console.Commands.h>

namespace fx
{
class GameBuild : public std::string
{
public:
	GameBuild()
		: std::string()
	{

	}

	GameBuild(const char* str)
		: std::string(str)
	{
	}

	GameBuild(const std::string& str)
		: std::string(str)
	{

	}
};
}

template<>
struct ConsoleArgumentType<fx::GameBuild>
{
	static std::string Unparse(const fx::GameBuild& input)
	{
		return input;
	}

	static bool Parse(const std::string& input, fx::GameBuild* out)
	{
		std::string inputStr = input;

		try
		{
			if (auto intString = std::to_string(std::stoi(inputStr)); intString == inputStr)
			{
				*out = intString;
				return true;
			}
		}
		catch (std::invalid_argument&)
		{
			// nothing
		}

		// xm -> christmas20
		if (inputStr.find("xm") == 0 && inputStr.length() == 4)
		{
			inputStr = "christmas20" + inputStr.substr(2);
		}

		// hN -> heistN
		if (inputStr.find("h") == 0 && inputStr.length() == 2)
		{
			inputStr = "mpheist" + inputStr.substr(1);
		}

		// no mp? make it mp
		if (inputStr.find("mp") != 0)
		{
			inputStr = "mp" + inputStr;
		}

		if (_stricmp(inputStr.c_str(), "mpchristmas2018") == 0)
		{
			*out = "1604";
			return true;
		}
		else if (_stricmp(inputStr.c_str(), "mpsum") == 0)
		{
			*out = "2060";
			return true;
		}
		else if (_stricmp(inputStr.c_str(), "mpheist4") == 0)
		{
			*out = "2189";
			return true;
		}
		else if (_stricmp(inputStr.c_str(), "mptuner") == 0)
		{
			*out = "2372";
			return true;
		}
		else if (_stricmp(inputStr.c_str(), "mpsecurity") == 0)
		{
			*out = "2545";
			return true;
		}
		else if (_stricmp(inputStr.c_str(), "mpg9ec") == 0)
		{
			*out = "2612";
			return true;
		}
		else if (_stricmp(inputStr.c_str(), "mpsum2") == 0)
		{
			*out = "2699";
			return true;
		}
		else if (_stricmp(inputStr.c_str(), "mpchristmas3") == 0)
		{
			*out = "2802";
			return true;
		}

		// not an int or a known alias
		return false;
	}
};

template<>
struct ConsoleArgumentName<fx::GameBuild>
{
	inline static const char* Get()
	{
		return "fx::GameBuild";
	}
};
