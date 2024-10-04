#pragma once

#include "StdInc.h"

#include <string>
#include <tuple>
#include <unordered_map>
#include <shared_mutex>

#include <mono/metadata/object.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/profiler.h>

#include <Profiler.h>
#include <CoreConsole.h>

#include <EASTL/fixed_hash_map.h>

#ifdef IS_FXSERVER
#define PRODUCT_NAME "Server"
#elif defined(GTA_FIVE)
#define PRODUCT_NAME "FiveM"
#elif defined(IS_RDR3)
#define PRODUCT_NAME "RedM"
#elif defined(GTA_NY)
#define PRODUCT_NAME "LibertyM"
#else
static_assert(false, "No native wrapper dll set for this game or program, are you missing a preprocessor definition?");
#endif

#ifndef _WIN32
#define FX_API 
#define COMPILE_MONO_RUNTIME_METHODS true // Linux loads different instances of shared libraries, Windows doesn't
#elif defined(COMPILING_CITIZEN_SCRIPTING_MONO)
#define FX_API DLL_EXPORT
#define COMPILE_MONO_RUNTIME_METHODS true
#else
#define FX_API DLL_IMPORT
#define COMPILE_MONO_RUNTIME_METHODS false
#endif


struct _MonoProfiler
{
	char pad[128];
};

typedef struct _MonoProfiler MonoProfiler;

namespace fx::mono
{
// Shared among v1 and v2 runtimes, some code is only directly available to the v1 runtime
// but we still move it here to make a clear distinction of what is required for and influences both runtimes
// and to potentially be able to unify these in the future
class MonoComponentHostShared
{
#if COMPILE_MONO_RUNTIME_METHODS
private:
#ifndef IS_FXSERVER
	// assembly trust and resolve
	static const wchar_t* const s_platformAssemblies[];
#endif

	// gc events
	static eastl::fixed_hash_map<int32_t, uint64_t, 4096, 4096 + 1, false> s_memoryUsages;
	static std::array<uint64_t, 128> s_memoryUsagesById;
	static std::shared_mutex s_memoryUsagesMutex;

	static bool s_requestedMemoryUsage;
	static bool s_enableMemoryUsage;

	// profiling
	static MonoProfiler s_monoProfiler;
#endif // COMPILE_MONO_RUNTIME_METHODS

public:
	// Initialize the mono environment, needed for both v1 and v2, only call once per runtime initialization
	FX_API static void Initialize();

	// Print script messages
	static void Print(MonoString* channel, MonoString* message);

	// Print exception details
	FX_API static void PrintException(MonoObject* exc, bool fatal = false);

#if COMPILE_MONO_RUNTIME_METHODS
	// Hook to handle unhandled exceptions
	static void UnhandledException(MonoObject* exc, void* userData);

private:
	// Hook to that determines if the loading of an image/assembly is trusted/platform or untrusted/UGC code
	static int CoreCLRIsTrustedCode(const char* imageName);
#endif // COMPILE_MONO_RUNTIME_METHODS

#pragma region GC & related hooks

public:
	// Get current memory usage
	FX_API static uint64_t GetMemoryUsage();

#if COMPILE_MONO_RUNTIME_METHODS
private:
	// Called on a GC event, used to profile	
#ifdef IS_FXSERVER
	static void gc_resize(MonoProfiler* profiler, uintptr_t new_size);
#else
	static void gc_resize(MonoProfiler* profiler, int64_t new_size);
#endif

	// Called on a GC resizing event, used to profile
#ifdef IS_FXSERVER
	static void gc_event(MonoProfiler* profiler, MonoProfilerGCEvent event, uint32_t generation, mono_bool is_serial);
#else
	static void gc_event(MonoProfiler* profiler, MonoGCEvent event, int generation);
#endif

#pragma endregion

private:
	static void ProfilerShutDown(MonoProfiler* profiler);
#endif // COMPILE_MONO_RUNTIME_METHODS

public:
	static std::string MakeRelativeNarrowPath(const std::string& path);
};


inline void MonoComponentHostShared::Print(MonoString* channel, MonoString* message)
{
	char* cChannel = mono_string_to_utf8(channel);
	char* cMessage = mono_string_to_utf8(message);

	console::Printf(cChannel, "%s", cMessage);

	mono_free(cChannel);
	mono_free(cMessage);
}

#if COMPILE_MONO_RUNTIME_METHODS
inline void MonoComponentHostShared::UnhandledException(MonoObject* exc, void* userData)
{
	PrintException(exc, true);
}

inline void MonoComponentHostShared::ProfilerShutDown(MonoProfiler* profiler)
{
	// not implemented?
}
#endif // COMPILE_MONO_RUNTIME_METHODS

inline std::string MonoComponentHostShared::MakeRelativeNarrowPath(const std::string& path)
{
#ifdef _WIN32
	return ToNarrow(GetAbsoluteCitPath()) + path;
#else
	return GetAbsoluteCitPath() + path;
#endif
}
}
