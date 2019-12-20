#pragma once

#if !defined(COMPILING_LAUNCH) && !defined(COMPILING_CONSOLE) && !defined(COMPILING_SHARED_LIBC)
#include <fnv.h>

int GlobalErrorRealV(const char* file, int line, uint32_t stringHash, const char* string, fmt::printf_args formatList);
int FatalErrorRealV(const char* file, int line, uint32_t stringHash, const char* string, fmt::printf_args formatList);

template<typename... TArgs>
inline int GlobalErrorReal(const char* file, int line, uint32_t stringHash, const char* string, const TArgs&... args)
{
	return GlobalErrorRealV(file, line, stringHash, string, fmt::make_printf_args(args...));
}

template<typename... TArgs>
inline int FatalErrorReal(const char* file, int line, uint32_t stringHash, const char* string, const TArgs&... args)
{
	return FatalErrorRealV(file, line, stringHash, string, fmt::make_printf_args(args...));
}

template<uint32_t I>
inline uint32_t const_uint32()
{
	return I;
}

#if defined(COMPILING_ADHESIVE) || defined(COMPILING_SVADHESIVE)
#define _CFX_FILE "adhesive"
#else
#define _CFX_FILE __FILE__
#endif

#define GlobalError(f, ...) do { if (GlobalErrorReal(_CFX_FILE, __LINE__, const_uint32<fnv1a_t<4>::Hash(f)>(), f, ##__VA_ARGS__) < 0) { *(volatile int*)0 = 0; } } while(false)
#define FatalError(f, ...) do { if (FatalErrorReal(_CFX_FILE, __LINE__, const_uint32<fnv1a_t<4>::Hash(f)>(), f, ##__VA_ARGS__) < 0) { *(volatile int*)0 = 0; } } while(false)
#define FatalErrorNoExcept(f, ...) do { if (FatalErrorReal(_CFX_FILE, 99999, const_uint32<fnv1a_t<4>::Hash(f)>(), f, ##__VA_ARGS__) < 0) { } } while(false)
#else
void GlobalErrorV(const char* string, fmt::printf_args formatList);
void FatalErrorV(const char* string, fmt::printf_args formatList);

template<typename... TArgs>
inline void GlobalError(const char* string, const TArgs&... args)
{
	return GlobalErrorV(string, fmt::make_printf_args(args...));
}

template<typename... TArgs>
inline void FatalError(const char* string, const TArgs&... args)
{
	return FatalErrorV(string, fmt::make_printf_args(args...));
}
#endif

void AddCrashometryV(const std::string& key, const std::string& format, fmt::printf_args value);

template<typename... TArgs>
inline void AddCrashometry(const std::string& key, const std::string& format, const TArgs&... args)
{
	return AddCrashometryV(key, format, fmt::make_printf_args(args...));
}
