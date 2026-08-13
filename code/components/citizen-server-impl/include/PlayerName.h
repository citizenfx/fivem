#pragma once

#include <string>

#include <utf8.h>

namespace fx
{
static constexpr size_t kMaxPlayerNameLength = 200;

// Limits a name to kMaxPlayerNameLength bytes and replaces invalid UTF-8 in it.
inline bool NormalizePlayerName(std::string& name)
{
	if (name.length() >= kMaxPlayerNameLength)
	{
		name = name.substr(0, kMaxPlayerNameLength);
	}

	std::string validName;

	try
	{
		utf8::replace_invalid(name.begin(), name.end(), std::back_inserter(validName));
	}
	catch (std::exception&)
	{
		return false;
	}

	name = std::move(validName);

	return true;
}
}
