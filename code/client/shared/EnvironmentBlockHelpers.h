#pragma once

#include <map>
#include <vector>
#include <sstream>

struct EnvVarComparator : std::binary_function<std::wstring, std::wstring, bool>
{
	bool operator()(const std::wstring& left, const std::wstring& right) const
	{
		return _wcsicmp(left.c_str(), right.c_str()) < 0;
	}
};

using EnvironmentMap = std::map<std::wstring, std::wstring, EnvVarComparator>;

static void ParseEnvironmentBlock(const wchar_t* block, EnvironmentMap& out)
{
	const wchar_t* p = block;

	std::wstringstream curString;

	std::wstring curKey;
	std::wstring curValue;

	bool inKey = true;
	bool firstKeyChar = true;

	while (true)
	{
		wchar_t curChar = *p;

		if (inKey)
		{
			if (curChar == L'\0')
			{
				break;
			}
			else if (curChar == L'=' && !firstKeyChar)
			{
				curKey = curString.str();
				curString.str(L"");
				curString.clear();

				inKey = false;
			}
			else
			{
				curString << curChar;
			}

			firstKeyChar = false;
		}
		else
		{
			if (curChar == L'\0')
			{
				curValue = curString.str();
				curString.str(L"");
				curString.clear();

				out[curKey] = curValue;

				inKey = true;
				firstKeyChar = true;
			}
			else
			{
				curString << curChar;
			}
		}

		p++;
	}
}

static void BuildEnvironmentBlock(const EnvironmentMap& map, std::vector<wchar_t>& outBlock)
{
	outBlock.clear();

	for (auto& pair : map)
	{
		// write the key
		outBlock.insert(outBlock.end(), pair.first.cbegin(), pair.first.cend());

		// equals symbol
		outBlock.push_back(L'=');

		// value
		outBlock.insert(outBlock.end(), pair.second.cbegin(), pair.second.cend());

		// null terminator
		outBlock.push_back(L'\0');
	}

	outBlock.push_back(L'\0');
}
