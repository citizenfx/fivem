/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Text.h"

WRAPPER const wchar_t* CText::Get(const char* textKey) { EAXJMP(0x7B54C0); }

static std::unordered_map<std::string, std::wstring> g_customTexts;

const wchar_t* CText::GetCustom(const char* key)
{
	wchar_t keyOut[256];
	mbstowcs(keyOut, key, _countof(keyOut));

	return va(L"%s", keyOut);

	auto it = g_customTexts.find(key);

	if (it == g_customTexts.end())
	{
		return nullptr;
	}

	return it->second.c_str();
}

void CText::SetCustom(const char* key, const wchar_t* value)
{
	g_customTexts[key] = value;
}

CText& TheText = *(CText*)0x10F4CE8;