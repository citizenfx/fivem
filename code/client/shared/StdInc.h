/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#ifndef _STDINC_H_

#define _STDINC_H_

// client-side shared include file
#if defined(_MSC_VER)
#pragma warning(disable: 4251) // needs to have dll-interface to be used by clients
#pragma warning(disable: 4273) // inconsistent dll linkage
#pragma warning(disable: 4275) // non dll-interface class used as base
#pragma warning(disable: 4244) // possible loss of data
#pragma warning(disable: 4800) // forcing value to bool
#pragma warning(disable: 4290) // C++ exception specification ignored

// MSVC odd defines
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#endif

#if defined(_WIN32)
// platform primary include
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

// silly win10 prerel sdk defines this for some reason
#ifdef Yield
#undef Yield
#endif

#include <versionhelpers.h>

#ifdef GTA_NY
#include <d3d9.h>
#endif

#define DLL_IMPORT __declspec(dllimport)
#define DLL_EXPORT __declspec(dllexport)

#define STATIC

#define __thread __declspec(thread)
#elif defined(__GNUC__)
#define DLL_IMPORT 
#define DLL_EXPORT __attribute__((visibility("default")))

#define STATIC __attribute__((visibility("internal")))

#define FORCEINLINE __attribute__((always_inline))

#include <unistd.h>

// compatibility
#define _stricmp strcasecmp
#define _strnicmp strncasecmp

#define _countof(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
#endif

#undef NDEBUG

#ifdef MEMDBGOK
#define _CRTDBG_MAP_ALLOC
#endif

// C/C++ headers
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef NDEBUG
#undef NDEBUG
#include <assert.h>
#define NDEBUG
#else
#include <assert.h>
#endif

#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <list>
#include <atomic>
#include <locale>
#include <codecvt>
#include <thread>

// our common includes
#define COMPONENT_EXPORT

// string types per-platform
#if defined(_WIN32)
class fwPlatformString : public std::wstring
{
private:
	inline std::wstring ConvertString(const char* narrowString)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
		return converter.from_bytes(narrowString);
	}

public:
	fwPlatformString()
		: std::wstring()
	{
	}

	fwPlatformString(const std::wstring& arg)
		: std::wstring(arg)
	{
	}

	fwPlatformString(const wchar_t* arg)
		: std::wstring(arg)
	{
	}

	inline fwPlatformString(const std::string& narrowString)
		: std::wstring(ConvertString(narrowString.c_str()))
	{

	}

	inline fwPlatformString(const char* narrowString)
		: std::wstring(ConvertString(narrowString))
	{

	}
};
typedef wchar_t pchar_t;

#define _pfopen _wfopen
#define _P(x) L##x
#else
class fwPlatformString : public std::string
{
private:
	inline std::string ConvertString(const wchar_t* wideString)
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
		return converter.to_bytes(wideString);
	}

public:
	fwPlatformString()
		: std::string()
	{
	}

	fwPlatformString(const std::string& arg)
		: std::string(arg)
	{
	}

	fwPlatformString(const char* arg)
		: std::string(arg)
	{
	}

	inline fwPlatformString(const wchar_t* wideString)
		: std::string(ConvertString(wideString))
	{
		
	}
};

typedef char pchar_t;

#define _pfopen fopen
#define _P(x) x
#endif

#include "EventCore.h"

#include <Utils.h>
#include <sigslot.h>

// module-specific includes
#ifdef COMPILING_HOOKS
#include "Hooking.h"
#endif

#include "Registry.h"
#include "HookFunction.h"

#ifdef HAS_LOCAL_H
#include "Local.h"
#endif

#ifdef MEMDBGOK
#include <crtdbg.h>
#endif

#endif