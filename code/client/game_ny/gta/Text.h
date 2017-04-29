#pragma once

class GAMESPEC_EXPORT CText
{
public:
	const wchar_t* Get(const char* key);

	const wchar_t* GetCustom(const char* key);

	void SetCustom(const char* key, const wchar_t* value);
};

extern GAMESPEC_EXPORT CText& TheText;