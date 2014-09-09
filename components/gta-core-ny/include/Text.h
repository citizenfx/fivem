#pragma once

#ifndef GTACORE_EXPORT
#ifdef COMPILING_GTA_CORE_NY
#define GTACORE_EXPORT __declspec(dllexport)
#else
#define GTACORE_EXPORT __declspec(dllimport)
#endif
#endif

class GTACORE_EXPORT CText
{
public:
	const wchar_t* Get(const char* key);

	const wchar_t* GetCustom(const char* key);

	void SetCustom(const char* key, const wchar_t* value);
};

extern GTACORE_EXPORT CText& TheText;