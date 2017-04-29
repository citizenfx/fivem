#pragma once

#if !defined(COMPILING_LAUNCH) && !defined(COMPILING_CONSOLE) && !defined(COMPILING_SHARED_LIBC)
#include <fnv.h>

int GlobalErrorReal(const char* file, int line, uint32_t stringHash, const char* string, const fmt::ArgList& formatList);
int FatalErrorReal(const char* file, int line, uint32_t stringHash, const char* string, const fmt::ArgList& formatList);

FMT_VARIADIC(int, GlobalErrorReal, const char*, int, uint32_t, const char*);
FMT_VARIADIC(int, FatalErrorReal, const char*, int, uint32_t, const char*);

template<uint32_t I>
inline uint32_t const_uint32()
{
	return I;
}

#ifdef COMPILING_ADHESIVE
#define _CFX_FILE "adhesive"
#else
#define _CFX_FILE __FILE__
#endif

#define GlobalError(f, ...) do { if (GlobalErrorReal(_CFX_FILE, __LINE__, const_uint32<fnv1a_t<4>::Hash(f)>(), f, __VA_ARGS__) < 0) { *(int*)0 = 0; } } while(false)
#define FatalError(f, ...) do { if (FatalErrorReal(_CFX_FILE, __LINE__, const_uint32<fnv1a_t<4>::Hash(f)>(), f, __VA_ARGS__) < 0) { *(int*)0 = 0; } } while(false)
#define FatalErrorNoExcept(f, ...) do { if (FatalErrorReal(_CFX_FILE, 99999, const_uint32<fnv1a_t<4>::Hash(f)>(), f, __VA_ARGS__) < 0) { } } while(false)
#else
void GlobalError(const char* string, const fmt::ArgList& formatList);
void FatalError(const char* string, const fmt::ArgList& formatList);

FMT_VARIADIC(void, GlobalError, const char*);
FMT_VARIADIC(void, FatalError, const char*);
#endif