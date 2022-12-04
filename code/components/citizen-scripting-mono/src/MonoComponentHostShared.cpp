#include "StdInc.h"

#include "MonoComponentHostShared.h"

#include <Error.h>
#include <CoreConsole.h>

#include <mono/jit/jit.h>
#include <mono/metadata/exception.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/mono-gc.h>

#ifndef IS_FXSERVER
extern "C" {
#include <mono/metadata/security-core-clr.h>
}
#endif

#ifdef _WIN32
#define FX_SEARCHPATH_SEPARATOR ';'
#else
#define FX_SEARCHPATH_SEPARATOR ':'
#endif

namespace fx::mono
{
// assembly trust and resolving
#ifndef IS_FXSERVER
const wchar_t* const MonoComponentHostShared::s_platformAssemblies[] = {
	L"mscorlib.dll",
	L"Mono.CSharp.dll",
	L"System.dll",
	L"System.Core.dll",
	L"CitizenFX.Core.dll",
	L"CitizenFX.Core.Client.dll",
	L"CitizenFX." PRODUCT_NAME ".dll",
	L"CitizenFX." PRODUCT_NAME ".NativesImpl.dll",
};
#endif

// gc events
eastl::fixed_hash_map<int32_t, uint64_t, 4096, 4096 + 1, false> MonoComponentHostShared::s_memoryUsages;
std::array<uint64_t, 128> MonoComponentHostShared::s_memoryUsagesById;
std::shared_mutex MonoComponentHostShared::s_memoryUsagesMutex;

bool MonoComponentHostShared::s_requestedMemoryUsage;
bool MonoComponentHostShared::s_enableMemoryUsage;

// profiling
_MonoProfiler MonoComponentHostShared::s_monoProfiler;

bool SetEnvironmentVariableNarrow(const char* key, const char* value)
{
#ifdef _WIN32
	return SetEnvironmentVariableW(ToWide(key).c_str(), ToWide(value).c_str());
#else
	// POSIX doesn't copy our strings with putenv, setenv does
	return setenv(key, value, true) == 0;
#endif
}

#ifndef _WIN32
extern "C" void mono_handle_native_crash_nop(const char* signal, void* sigctx, void* siginfo)
{
}
#endif

void MonoComponentHostShared::Initialize()
{
	static ConVar<bool> memoryUsageVar("mono_enableMemoryUsageTracking", ConVar_None, true, &s_enableMemoryUsage);

	if (mono_get_root_domain() == nullptr)
	{
		std::string basePath = MakeRelativeNarrowPath("citizen/clr2/");

		// MONO_PATH
		std::string citizenClrLibPath = basePath + "lib/mono/4.5/";
		// citizenClrLibPath = citizenClrLibPath + FX_SEARCHPATH_SEPARATOR + citizenClrLibPath + "v2/"; // throws warnings when v2 folder isn't present, re-enable later
		SetEnvironmentVariableNarrow("MONO_PATH", citizenClrLibPath.c_str());

		// https://github.com/mono/mono/pull/9811
		// https://www.mono-project.com/docs/advanced/runtime/docs/coop-suspend/#cant-handle-the-embedding-api
		// Mono coop suspend does not work for embedders, and on systems not designed for multithreading (aka any POSIX system)
		// it'll infinitely wait for the main thread's infinite loop to handle a GC suspend call. Since the 'yield for the GC'
		// API isn't exposed, and it's clearly not meant for embedding, we just switch to the old non-coop suspender.
		SetEnvironmentVariableNarrow("MONO_THREADS_SUSPEND", "preemptive");

		// low-pause GC mode (with 5ms target pause duration), quadruple default nursery size to ensure first-phase allocs
		// see https://www.mono-project.com/docs/advanced/garbage-collector/sgen/working-with-sgen/
		SetEnvironmentVariableNarrow("MONO_GC_PARAMS", "mode=pause:5,nursery-size=16m");

		// unsure why this is here
		// putenv("MONO_DEBUG=casts");

#ifndef IS_FXSERVER
		mono_security_enable_core_clr();
		mono_security_core_clr_set_options((MonoSecurityCoreCLROptions)(MONO_SECURITY_CORE_CLR_OPTIONS_RELAX_DELEGATE | MONO_SECURITY_CORE_CLR_OPTIONS_RELAX_REFLECTION));
		mono_security_set_core_clr_platform_callback(CoreCLRIsTrustedCode);

		//mono_profiler_install(&s_monoProfiler, ProfilerShutDown);
		mono_profiler_install_gc(gc_event, gc_resize);
		mono_profiler_set_events(MONO_PROFILE_GC);
#elif defined(_WIN32)
		auto monoProfilerHandle = mono_profiler_create(&s_monoProfiler);

		mono_profiler_set_gc_event_callback(monoProfilerHandle, gc_event);
		mono_profiler_set_gc_resize_callback(monoProfilerHandle, gc_resize);
#endif

		const char* args[]
		{
#ifdef _WIN32
			"--soft-breakpoints",
			"--optimize=peephole,cfold,inline,consprop,copyprop,deadce,branch,linears,intrins,loop,exception,cmov,gshared,simd,alias-analysis,float32,unsafe"
#else
			"--use-fallback-tls"
#endif
		};

		mono_jit_parse_options((int)std::size(args), const_cast<char**>(args)); // should've been const qualified
		mono_set_crash_chaining(true);

#ifndef _WIN32
		mono_set_signal_chaining(true);
#endif

		if (!mono_debug_enabled())
			mono_debug_init(MONO_DEBUG_FORMAT_MONO);

		mono_set_dirs((basePath + "lib/").c_str(), (basePath + "cfg/").c_str());
		MonoDomain* rootDomain = mono_jit_init_version("Citizen", "v4.0.30319");

		mono_domain_set_config(rootDomain, (basePath + "cfg/mono/").c_str(), "cfx.config");

		mono_install_unhandled_exception_hook(UnhandledException, nullptr);
	}
}

void MonoComponentHostShared::PrintException(MonoObject* exc, bool fatal)
{
	MonoClass* eclass = mono_object_get_class(exc);

	if (eclass)
	{
		MonoObject* toStringExc = nullptr;
		MonoString* msg = mono_object_to_string(exc, &toStringExc);

		MonoProperty* prop = mono_class_get_property_from_name(eclass, "StackTrace");
		MonoMethod* getter = mono_property_get_get_method(prop);
		MonoString* msg2 = (MonoString*)mono_runtime_invoke(getter, exc, NULL, NULL);

		if (toStringExc)
		{
			MonoProperty* prop = mono_class_get_property_from_name(eclass, "Message");
			MonoMethod* getter = mono_property_get_get_method(prop);
			msg = (MonoString*)mono_runtime_invoke(getter, exc, NULL, NULL);
		}

		char* msg1CStr = mono_string_to_utf8(msg);
		char* msg2CStr = mono_string_to_utf8(msg2);

		if (fatal)
		{
			GlobalError("Unhandled exception in Mono script environment: %s\n%s", msg1CStr, msg2CStr);
		}
		else
		{
			trace("Exception in Mono script environment: %s\n%s", msg1CStr, msg2CStr);
		}

		mono_free(msg1CStr);
		mono_free(msg2CStr);
	}
}

#if COMPILE_MONO_RUNTIME_METHODS
#ifndef IS_FXSERVER
int MonoComponentHostShared::CoreCLRIsTrustedCode(const char* imageName)
{
	if (imageName)
	{
		wchar_t* filePart = nullptr;
		wchar_t fullPath[512];

		// *W variants (like `GetFullPathNameW`) are guaranteed to be Unicode, *A are dependent on locale and other settings.
		if (GetFullPathNameW(ToWide(imageName).c_str(), _countof(fullPath), fullPath, &filePart) != 0 && filePart != nullptr)
		{
			std::size_t folderPathSize = filePart - fullPath;
			std::wstring base = GetAbsoluteCitPath() + L"citizen\\clr2\\lib\\mono\\4.5\\";

			// check if the path is or is a child of the /mono/4.5/ directory
			// if this needs to be faster then we can introduce a reverse strcmp() in which we omit the above ToNarrow() by comparing char* and wchar_t* directly and do inline to_lower()
			if (folderPathSize >= base.size()
				&& _wcsnicmp(fullPath, base.data(), base.size()) == 0)
			{
				// compare file name
				for (int i = 0; i < _countof(s_platformAssemblies); ++i)
				{
					if (_wcsicmp(filePart, s_platformAssemblies[i]) == 0)
					{
						return TRUE;
					}
				}
			}
		}
	}

	return FALSE;
}
#endif

uint64_t MonoComponentHostShared::GetMemoryUsage()
{
	auto monoDomain = mono_domain_get();

	s_requestedMemoryUsage = true;

	std::shared_lock<std::shared_mutex> lock(s_memoryUsagesMutex);
	auto did = mono_domain_get_id(monoDomain);

	return std::size(s_memoryUsagesById) > did ? s_memoryUsagesById[did] : s_memoryUsages[did];
}

#ifdef IS_FXSERVER
void MonoComponentHostShared::gc_event(MonoProfiler* profiler, MonoProfilerGCEvent event, uint32_t generation, mono_bool is_serial)
#else
void MonoComponentHostShared::gc_event(MonoProfiler* profiler, MonoGCEvent event, int generation)
#endif
{
	if (event == MONO_GC_EVENT_PRE_STOP_WORLD || event == MONO_GC_EVENT_POST_START_WORLD)
	{
		static auto profiler = fx::ResourceManager::GetCurrent(true)->GetComponent<fx::ProfilerComponent>();

		if (profiler->IsRecording())
		{
			bool isMajor = (generation == 1); // sgen seems to only have 0/1 (mono_gc_max_generation == 1)
			static std::string majorGcString = ".NET Major GC";
			static std::string minorGcString = ".NET Minor GC";

			if (event == MONO_GC_EVENT_PRE_STOP_WORLD)
			{
				profiler->EnterScope(isMajor ? majorGcString : minorGcString);
			}
			else
			{
				profiler->ExitScope();
			}
		}
	}

#if defined(_WIN32)
	switch (event)
	{
		// a comment above mono_gc_walk_heap says the following:
		// 'heap walking is only valid in the pre-stop-world event callback'
		// however, this is actually wrong: pre-stop-world isn't locked, and walking the heap there is not thread-safe
		// more importantly, mono itself uses this in MONO_GC_EVENT_PRE_START_WORLD:
		// https://github.com/mono/mono/blob/bdd772531d379b4e78593587d15113c37edd4a64/mono/profiler/log.c#L1456
		//
		// therefore, we assume the comment is wrong (a typo?) and the implementation is correct, and this should indeed be pre-start-world
		case MONO_GC_EVENT_PRE_START_WORLD:
			if (s_requestedMemoryUsage && s_enableMemoryUsage)
			{
				std::unique_lock<std::shared_mutex> lock(s_memoryUsagesMutex);

				s_memoryUsages.clear();
				memset(s_memoryUsagesById.data(), 0, s_memoryUsagesById.size() * sizeof(uint64_t));

				mono_gc_walk_heap(
				0, [](MonoObject* obj, MonoClass* klass, uintptr_t size, uintptr_t num, MonoObject** refs, uintptr_t* offsets, void* data) -> int
				{
					auto did = mono_domain_get_id(mono_object_get_domain(obj));

					if (did < 0 || did >= std::size(s_memoryUsagesById))
					{
						s_memoryUsages[did] += size;
					}
					else
					{
						s_memoryUsagesById[did] += size;
					}

					return 0;
				},
				nullptr);

				s_requestedMemoryUsage = false;
			}
			break;
	}
#endif
}
	
#ifdef IS_FXSERVER
void MonoComponentHostShared::gc_resize(MonoProfiler* profiler, uintptr_t new_size)
#else
void MonoComponentHostShared::gc_resize(MonoProfiler* profiler, int64_t new_size)
#endif
{
}
}
#endif // COMPILE_MONO_RUNTIME_METHODS
