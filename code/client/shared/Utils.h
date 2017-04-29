/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <algorithm>

//
// Returns the Citizen root directory.
//

fwPlatformString GetAbsoluteCitPath();

//
// Returns the game root directory.
//

fwPlatformString GetAbsoluteGamePath();

//
// Returns a said path relative to the Citizen base installation directory (containing the launcher executable).
//

inline fwPlatformString MakeRelativeCitPath(fwPlatformString targetPath)
{
	return GetAbsoluteCitPath() + targetPath;
}

//
// Returns a said path relative to the game installation directory (containing the target executable).
//

inline fwPlatformString MakeRelativeGamePath(fwPlatformString targetPath)
{
	return GetAbsoluteGamePath() + targetPath;
}

//
// Returns whether or not we are in a test executable.
//

bool IsRunningTests();

//
// Base initialization function (for running on library init and so on)
//

class STATIC InitFunctionBase
{
protected:
	InitFunctionBase* m_next;

	int m_order;

public:
	InitFunctionBase(int order = 0);

	virtual void Run() = 0;

	void Register();

	static void RunAll();
};

//
// Initialization function that will be called around initialization of the primary component.
//

class STATIC InitFunction : public InitFunctionBase
{
private:
	void(*m_function)();

public:
	InitFunction(void(*function)(), int order = 0)
		: InitFunctionBase(order)
	{
		m_function = function;

		Register();
	}

	virtual void Run()
	{
		m_function();
	}
};

//
// 'compile-time' hashing
//

template <unsigned int N, unsigned int I>
struct FnvHash
{
	FORCEINLINE static unsigned int Hash(const char(&str)[N])
	{
		return (FnvHash<N, I - 1>::Hash(str) ^ str[I - 1]) * 16777619u;
	}
};

template <unsigned int N>
struct FnvHash<N, 1>
{
	FORCEINLINE static unsigned int Hash(const char(&str)[N])
	{
		return (2166136261u ^ str[0]) * 16777619u;
	}
};

class StringHash
{
private:
	unsigned int m_hash;

public:
	template <unsigned int N>
	FORCEINLINE StringHash(const char(&str)[N])
		: m_hash(FnvHash<N, N>::Hash(str))
	{
	}

	FORCEINLINE operator unsigned int() const
	{
		return m_hash;
	}
};

#ifndef _M_AMD64
#define CALL_NO_ARGUMENTS(addr) ((void(*)())addr)()
#define EAXJMP(a) { _asm mov eax, a _asm jmp eax }
#define WRAPPER __declspec(naked)
#else
#define EAXJMP(a)
#define WRAPPER
#endif

//
// export class specifiers
//

#ifdef COMPILING_GAME
#define GAME_EXPORT DLL_EXPORT
#else
#define GAME_EXPORT DLL_IMPORT
#endif

#ifdef COMPILING_HOOKS
#define HOOKS_EXPORT DLL_EXPORT
#else
#define HOOKS_EXPORT DLL_IMPORT
#endif

#ifdef COMPILING_CORE
#define CORE_EXPORT DLL_EXPORT
#else
#define CORE_EXPORT DLL_IMPORT
#endif

#ifdef COMPILING_GAMESPEC
#define GAMESPEC_EXPORT DLL_EXPORT
#define GAMESPEC_EXPORT_VMT DLL_EXPORT
#else
#define GAMESPEC_EXPORT DLL_IMPORT
#define GAMESPEC_EXPORT_VMT
#endif

//
// formatting/logging functions
//

#include <fmt/format.h>
#include <fmt/printf.h>

template<typename TEnum, typename = std::enable_if_t<std::is_enum<TEnum>::value>>
std::ostream& operator<<(std::ostream& os, const TEnum& value)
{
	os << static_cast<typename std::underlying_type_t<TEnum>>(value);
	return os;
}

const char* va(const char* string, const fmt::ArgList& formatList);
FMT_VARIADIC(const char*, va, const char*);

void trace(const char* string, const fmt::ArgList& formatList);
FMT_VARIADIC(void, trace, const char*);

const wchar_t* va(const wchar_t* string, const fmt::ArgList& formatList);
FMT_VARIADIC_W(const wchar_t*, va, const wchar_t*);

uint32_t HashRageString(const char* string);
uint32_t HashString(const char* string);

inline void LowerString(fwString& string)
{
	std::transform(string.begin(), string.end(), string.begin(), ::tolower);
}

fwString url_encode(const fwString &value);
bool UrlDecode(const std::string& in, std::string& out);
void CreateDirectoryAnyDepth(const char *path);

void SetThreadName(int threadId, char* threadName);

std::wstring ToWide(const std::string& narrow);
std::string ToNarrow(const std::wstring& wide);

#ifdef COMPILING_CORE
extern "C" bool DLL_EXPORT CoreIsDebuggerPresent();
extern "C" void DLL_EXPORT CoreSetDebuggerPresent();
#else
inline bool CoreIsDebuggerPresent()
{
    static bool(*func)();

    if (!func)
    {
        func = (bool(*)())GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "CoreIsDebuggerPresent");
    }

    return (!func) ? false : func();
}

inline void CoreSetDebuggerPresent()
{
    static void(*func)();

    if (!func)
    {
        func = (void(*)())GetProcAddress(GetModuleHandle(L"CoreRT.dll"), "CoreSetDebuggerPresent");
    }

    (func) ? func() : 0;
}
#endif


// min/max
template<typename T, typename = void>
struct MinMax
{
	inline static T min(T a, T b)
	{
		return (a < b) ? a : b;
	}

	inline static T max(T a, T b)
	{
		return (a > b) ? a : b;
	}
};

template<typename TValue>
struct MinMax<TValue, std::enable_if_t<std::is_integral<TValue>::value>>
{
	using TSigned = std::make_signed_t<TValue>;

	inline static TValue min(TValue aa, TValue bb)
	{
		TSigned a = (TSigned)aa;
		TSigned b = (TSigned)bb;

		return b + ((a - b) & (a - b) >> (sizeof(TSigned) * std::numeric_limits<TSigned>::max() - 1));
	}

	inline static TValue max(TValue aa, TValue bb)
	{
		TSigned a = (TSigned)aa;
		TSigned b = (TSigned)bb;

		return a - ((a - b) & (a - b) >> (sizeof(TSigned) * std::numeric_limits<TSigned>::max() - 1));
	}
};

template<>
struct MinMax<float>
{
	inline static float min(float a, float b)
	{
		_mm_store_ss(&a, _mm_min_ss(_mm_set_ss(a), _mm_set_ss(b)));
		return a;
	}

	inline static float max(float a, float b)
	{
		_mm_store_ss(&a, _mm_max_ss(_mm_set_ss(a), _mm_set_ss(b)));
		return a;
	}
};

template<>
struct MinMax<double>
{
	inline static double min(double a, double b)
	{
		_mm_store_sd(&a, _mm_min_sd(_mm_set_sd(a), _mm_set_sd(b)));
		return a;
	}

	inline static double max(double a, double b)
	{
		_mm_store_sd(&a, _mm_max_sd(_mm_set_sd(a), _mm_set_sd(b)));
		return a;
	}
};

template<typename T>
inline T min(T a, T b)
{
	return MinMax<T>::min(a, b);
}

template<typename T>
inline T max(T a, T b)
{
	return MinMax<T>::max(a, b);
}