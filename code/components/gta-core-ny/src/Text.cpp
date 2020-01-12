/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Text.h"
#include <Hooking.h>

static hook::thiscall_stub<const wchar_t*(CText*, const char* textKey)> _CText__Get([]()
{
	return hook::get_pattern("83 EC 44 A1 ? ? ? ? 33 C4 89 44 24 40 8B 44 24 48 56 8B F1");
});

const wchar_t* CText::Get(const char* textKey)
{ 
	return _CText__Get(this, textKey);
}

static std::unordered_map<uint32_t, std::wstring> g_customTexts;

const wchar_t* CText::GetCustom(const char* key)
{
	auto it = g_customTexts.find(HashString(key));

	if (it == g_customTexts.end() || it->second.empty())
	{
		return nullptr;
	}

	return it->second.c_str();
}

void CText::SetCustom(uint32_t key, const wchar_t* value)
{
	g_customTexts[key] = value;
}

void CText::SetCustom(const char* key, const wchar_t* value)
{
	g_customTexts[HashString(key)] = value;
}

CText* TheText;

static HookFunction hookFunc([]()
{
	TheText = *hook::get_pattern<CText*>("EB ? A1 ? ? ? ? 48 83 F8 03", -9);
});
