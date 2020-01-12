#pragma once

#include <Text.h>

namespace game
{
inline void AddCustomText(const std::string& key, const std::string& value)
{
	TheText->SetCustom(key.c_str(), ToWide(value).c_str());
}

inline void AddCustomText(uint32_t hash, const std::string& value)
{
	TheText->SetCustom(hash, ToWide(value).c_str());
}

inline void RemoveCustomText(uint32_t hash)
{
	TheText->SetCustom(hash, L"");
}
}
