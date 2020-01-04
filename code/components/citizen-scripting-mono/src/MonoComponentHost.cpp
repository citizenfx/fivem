/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <om/OMComponent.h>

#include <Resource.h>

#include <fxScripting.h>

#include <CoreConsole.h>

#include <Error.h>

#include <mono/jit/jit.h>
#include <mono/utils/mono-logger.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/threads.h>

extern "C" {
	void mono_thread_push_appdomain_ref(MonoDomain *domain);
	void mono_thread_pop_appdomain_ref(void);

	char*
		mono_method_get_full_name(MonoMethod* method);

	MONO_API MonoArray* mono_get_current_context(void* stack_origin);
	MONO_API void
		mono_stack_walk_bounded(MonoStackWalk func, MonoArray* startRef, MonoArray* endRef, void* user_data);

}

#include <mono/metadata/exception.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/profiler.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#include <msgpack.hpp>

#include <shared_mutex>

#ifndef IS_FXSERVER
extern "C"
{
	#include <mono/metadata/security-core-clr.h>
}
#endif

static MonoDomain* g_rootDomain;

#ifndef IS_FXSERVER
const static wchar_t* const g_platformAssemblies[] =
{
	L"mscorlib.dll",
	L"System.dll",
	L"System.Core.dll",
	L"CitizenFX.Core.dll",
	L"Mono.CSharp.dll"
};

static int CoreClrCallback(const char* imageName)
{
	if (!imageName)
	{
		return FALSE;
	}

	wchar_t* filePart = nullptr;
	wchar_t fullPath[512];

	if (GetFullPathNameW(ToWide(imageName).c_str(), _countof(fullPath), fullPath, &filePart) == 0)
	{
		return FALSE;
	}

	if (!filePart)
	{
		return FALSE;
	}

	*(filePart - 1) = '\0';

	std::wstring platformPath = MakeRelativeCitPath(L"citizen\\clr2\\lib");

	if (_wcsicmp(platformPath.c_str(), fullPath) != 0)
	{
		platformPath = MakeRelativeCitPath(L"citizen\\clr2\\lib\\mono\\4.5");

		if (_wcsicmp(platformPath.c_str(), fullPath) != 0)
		{
			trace("%s %s is not a platform image.\n", ToNarrow(fullPath), ToNarrow(filePart));
			return FALSE;
		}
	}

	for (int i = 0; i < _countof(g_platformAssemblies); i++)
	{
		if (!_wcsicmp(filePart, g_platformAssemblies[i]))
		{
			return TRUE;
		}
	}

	trace("%s %s is not a platform image (even though the dir matches).\n", ToNarrow(fullPath), ToNarrow(filePart));

	return FALSE;
}
#endif

static void OutputExceptionDetails(MonoObject* exc, bool fatal = true)
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

		if (fatal)
		{
			GlobalError("Unhandled exception in Mono script environment: %s\n%s", mono_string_to_utf8(msg), mono_string_to_utf8(msg2));
		}
		else
		{
			trace("Exception in Mono script environment: %s\n%s", mono_string_to_utf8(msg), mono_string_to_utf8(msg2));
		}
	}
}

static void GI_PrintLogCall(MonoString* channel, MonoString* str)
{
	console::Printf(mono_string_to_utf8(channel), "%s", mono_string_to_utf8(str));
}

static void
gc_resize(MonoProfiler *profiler, int64_t new_size)
{
}

static void
profiler_shutdown(MonoProfiler *prof)
{
}

struct _MonoProfiler
{
	char pad[128];
};

MonoProfiler _monoProfiler;

#ifdef _WIN32
// custom heap so we won't end up depending on any suspended threads
// (we need to be safe even if the GC suspended the world)
static HANDLE g_heap = HeapCreate(0, 0, 0);

template<typename T>
struct StaticHeapAllocator
{
	using value_type = T;

	T* allocate(size_t n)
	{
		return (T*)HeapAlloc(g_heap, 0, n * sizeof(T));
	}

	void deallocate(T* p, size_t n)
	{
		HeapFree(g_heap, 0, p);
	}
};

static std::map<MonoDomain*, uint64_t, std::less<>, StaticHeapAllocator<std::pair<MonoDomain* const, uint64_t>>> g_memoryUsages;
#else
static std::map<MonoDomain*, uint64_t> g_memoryUsages;
#endif

static std::shared_mutex g_memoryUsagesMutex;

static bool g_requestedMemoryUsage;

#ifndef IS_FXSERVER
static void gc_event(MonoProfiler *profiler, MonoGCEvent event, int generation)
#else
static void gc_event(MonoProfiler *profiler, MonoProfilerGCEvent event, int generation)
#endif
{
	switch (event) {
	case MONO_GC_EVENT_PRE_START_WORLD:
#ifndef IS_FXSERVER
		if (g_requestedMemoryUsage)
		{
			std::unique_lock<std::shared_mutex> lock(g_memoryUsagesMutex);

			g_memoryUsages.clear();

			mono_gc_walk_heap(0, [](MonoObject *obj, MonoClass *klass, uintptr_t size, uintptr_t num, MonoObject **refs, uintptr_t *offsets, void *data) -> int
			{
				g_memoryUsages[mono_object_get_domain(obj)] += size;

				return 0;
			}, nullptr);

			g_requestedMemoryUsage = false;
		}
#endif
		break;
	}
}

static uint64_t GI_GetMemoryUsage()
{
	auto monoDomain = mono_domain_get();

	g_requestedMemoryUsage = true;

	std::shared_lock<std::shared_mutex> lock(g_memoryUsagesMutex);
	return g_memoryUsages[monoDomain];
}

static bool GI_SnapshotStackBoundary(MonoArray** blob)
{
#if _WIN32
	*blob = mono_get_current_context((void*)((uintptr_t)_AddressOfReturnAddress() + sizeof(void*)));
#else
	*blob = mono_get_current_context((void*)((uintptr_t)__builtin_frame_address(0U) + sizeof(void*)));
#endif

	return true;
}

struct ScriptStackFrame
{
	std::string name;
	std::string file;
	std::string sourcefile;
	int line;

	MonoMethod* method;

	ScriptStackFrame()
		: method(nullptr), line(0)
	{
	}

	MSGPACK_DEFINE_MAP(name, file, sourcefile, line);
};

MonoMethod* g_getMethodDisplayStringMethod;

static bool GI_WalkStackBoundary(MonoString* resourceName, MonoArray* start, MonoArray* end, MonoArray** outBlob)
{
	struct WD
	{
		MonoString* resourceName;
		std::vector<ScriptStackFrame> frames;
	} wd;

	wd.resourceName = resourceName;

	mono_stack_walk_bounded([](MonoMethod* method, int32_t native_offset, int32_t il_offset, mono_bool managed, void* data)
	{
		WD* d = (WD*)data;

		auto ma = mono_image_get_assembly(mono_class_get_image(mono_method_get_class(method)));
		auto man = mono_assembly_get_name(ma);
		auto s = mono_assembly_name_get_name(man);

		if (strstr(s, "CitizenFX.Core") || strstr(s, "mscorlib") || strncmp(s, "System", 6) == 0)
		{
			return FALSE;
		}

		ScriptStackFrame ssf;
		ssf.name = mono_method_get_name(method);
		ssf.method = method;

		auto sl = mono_debug_lookup_source_location(method, native_offset, mono_domain_get());

		ssf.file = fmt::sprintf("@%s/%s.dll", mono_string_to_utf8(d->resourceName), s);

		if (sl)
		{
			ssf.sourcefile = sl->source_file;
			ssf.line = sl->row;
		}

		d->frames.push_back(ssf);

		return FALSE;
	}, start, end, &wd);

	auto description = mono_method_desc_new("System.Reflection.RuntimeMethodInfo:GetMethodFromHandleInternalType", 1);
	auto method = mono_method_desc_search_in_image(description, mono_get_corlib());
	mono_method_desc_free(description);

	// pre-corefx import mono name
	if (!method)
	{
		description = mono_method_desc_new("System.Reflection.MethodBase:GetMethodFromHandleInternalType", 1);
		method = mono_method_desc_search_in_image(description, mono_get_corlib());
		mono_method_desc_free(description);
	}

	for (auto& frame : wd.frames)
	{
		if (frame.method)
		{
			void* zero = nullptr;
			void* args[] = { &frame.method, &zero };

			MonoObject* exc = nullptr;
			auto methodInfo = mono_runtime_invoke(method, nullptr, args, &exc);

			if (!exc && methodInfo)
			{
				args[0] = methodInfo;
				auto resolvedMethod = mono_runtime_invoke(g_getMethodDisplayStringMethod, nullptr, args, &exc);

				if (!exc && resolvedMethod)
				{
					auto str = mono_object_to_string(resolvedMethod, &exc);

					if (!exc && str)
					{
						frame.name = mono_string_to_utf8(str);
					}
				}
			}

			if (exc)
			{
				OutputExceptionDetails(exc, false);
			}
		}
	}

	msgpack::sbuffer sb;
	msgpack::pack(sb, wd.frames);

	MonoArray* blob = mono_array_new(mono_domain_get(), mono_get_byte_class(), sb.size());
	mono_value_copy_array(blob, 0, sb.data(), sb.size());

	*outBlob = blob;

	return true;
}

#ifndef IS_FXSERVER
MonoMethod* g_tickMethod;

extern "C" DLL_EXPORT void GI_TickInDomain(MonoAppDomain* domain)
{
	auto targetDomain = mono_domain_from_appdomain(domain);

	if (!targetDomain)
	{
		return;
	}

	auto currentDomain = mono_domain_get();

	if (currentDomain != targetDomain)
	{
		mono_thread_push_appdomain_ref(targetDomain);

		if (!mono_domain_set(targetDomain, false))
		{
			mono_thread_pop_appdomain_ref();
			return;
		}
	}
	
	MonoObject* exc = nullptr;
	mono_runtime_invoke(g_tickMethod, nullptr, nullptr, &exc);

	if (currentDomain != targetDomain)
	{
		mono_domain_set(currentDomain, true);
		mono_thread_pop_appdomain_ref();
	}
}
#endif

MonoMethod* g_getImplementsMethod;
MonoMethod* g_createObjectMethod;

static inline std::string MakeRelativeNarrowPath(const std::string& path)
{
#ifdef _WIN32
	return ToNarrow(MakeRelativeCitPath(ToWide(path)));
#else
	return MakeRelativeCitPath(path);
#endif
}

static void InitMono()
{
	std::string citizenClrPath = MakeRelativeNarrowPath("citizen/clr2/lib/");
	std::string citizenCfgPath = MakeRelativeNarrowPath("citizen/clr2/cfg/");

	mono_set_dirs(citizenClrPath.c_str(), citizenCfgPath.c_str());

#ifdef _WIN32
	std::wstring citizenClrLibPath = MakeRelativeCitPath(L"citizen/clr2/lib/mono/4.5/");

	SetEnvironmentVariable(L"MONO_PATH", citizenClrLibPath.c_str());

	mono_set_crash_chaining(true);
#else
	std::string citizenClrLibPath = MakeRelativeNarrowPath("citizen/clr2/lib/mono/4.5/");

	putenv(const_cast<char*>(va("MONO_PATH=%s", citizenClrLibPath)));
#endif

	mono_assembly_setrootdir(citizenClrPath.c_str());

	// https://github.com/mono/mono/pull/9811
	// https://www.mono-project.com/docs/advanced/runtime/docs/coop-suspend/#cant-handle-the-embedding-api
	// Mono coop suspend does not work for embedders, and on systems not designed for multithreading (aka any POSIX system)
	// it'll infinitely wait for the main thread's infinite loop to handle a GC suspend call. Since the 'yield for the GC'
	// API isn't exposed, and it's clearly not meant for embedding, we just switch to the old non-coop suspender.
	putenv("MONO_THREADS_SUSPEND=preemptive");

	putenv("MONO_DEBUG=casts");

#ifndef IS_FXSERVER
	mono_security_enable_core_clr();
	mono_security_core_clr_set_options((MonoSecurityCoreCLROptions)(MONO_SECURITY_CORE_CLR_OPTIONS_RELAX_DELEGATE | MONO_SECURITY_CORE_CLR_OPTIONS_RELAX_REFLECTION));
	mono_security_set_core_clr_platform_callback(CoreClrCallback);

	mono_profiler_install(&_monoProfiler, profiler_shutdown);
	mono_profiler_install_gc(gc_event, gc_resize);
	mono_profiler_set_events(MONO_PROFILE_GC);
#endif

	char* args[2];

#ifdef _WIN32
	args[0] = "--soft-breakpoints";
	args[1] = "--optimize=peephole,cfold,inline,consprop,copyprop,deadce,branch,linears,intrins,loop,exception,cmov,gshared,simd,alias-analysis,float32,unsafe";
#else
	args[0] = "--use-fallback-tls";
	args[1] = "";
#endif

	mono_jit_parse_options(std::size(args), args);

	mono_debug_init(MONO_DEBUG_FORMAT_MONO);

	g_rootDomain = mono_jit_init_version("Citizen", "v4.0.30319");

	mono_domain_set_config(g_rootDomain, MakeRelativeNarrowPath("").c_str(), "cfx.config");

	mono_install_unhandled_exception_hook([] (MonoObject* exc, void*)
	{
		OutputExceptionDetails(exc);
	}, nullptr);

	mono_set_crash_chaining(true);

	mono_add_internal_call("CitizenFX.Core.GameInterface::PrintLog", reinterpret_cast<void*>(GI_PrintLogCall));
	mono_add_internal_call("CitizenFX.Core.GameInterface::fwFree", reinterpret_cast<void*>(fwFree));

#ifndef IS_FXSERVER
	mono_add_internal_call("CitizenFX.Core.GameInterface::TickInDomain", reinterpret_cast<void*>(GI_TickInDomain));
#endif

	mono_add_internal_call("CitizenFX.Core.GameInterface::GetMemoryUsage", reinterpret_cast<void*>(GI_GetMemoryUsage));
	mono_add_internal_call("CitizenFX.Core.GameInterface::WalkStackBoundary", reinterpret_cast<void*>(GI_WalkStackBoundary));
	mono_add_internal_call("CitizenFX.Core.GameInterface::SnapshotStackBoundary", reinterpret_cast<void*>(GI_SnapshotStackBoundary));

	std::string platformPath = MakeRelativeNarrowPath("citizen/clr2/lib/mono/4.5/CitizenFX.Core.dll");

	auto scriptManagerAssembly = mono_domain_assembly_open(g_rootDomain, platformPath.c_str());

	if (!scriptManagerAssembly)
	{
		FatalError("Could not load CitizenFX.Core.dll.\n");
	}

	auto scriptManagerImage = mono_assembly_get_image(scriptManagerAssembly);

	bool methodSearchSuccess = true;
	MonoMethodDesc* description;

#define method_search(name, method) description = mono_method_desc_new(name, 1); \
			method = mono_method_desc_search_in_image(description, scriptManagerImage); \
			mono_method_desc_free(description); \
			methodSearchSuccess = methodSearchSuccess && method != NULL

	MonoMethod* rtInitMethod;
	method_search("CitizenFX.Core.RuntimeManager:Initialize", rtInitMethod);
	method_search("CitizenFX.Core.RuntimeManager:GetImplementedClasses", g_getImplementsMethod);
	method_search("CitizenFX.Core.RuntimeManager:CreateObjectInstance", g_createObjectMethod);
	method_search("System.Diagnostics.EnhancedStackTrace:GetMethodDisplayString", g_getMethodDisplayStringMethod);

#ifndef IS_FXSERVER
	method_search("CitizenFX.Core.InternalManager:TickGlobal", g_tickMethod);
#endif

	if (!methodSearchSuccess)
	{
		FatalError("Couldn't find one or more CitizenFX.Core methods.\n");
	}

	MonoObject* exc = nullptr;
	mono_runtime_invoke(rtInitMethod, nullptr, nullptr, &exc);

	if (exc)
	{
		OutputExceptionDetails(exc);
		return;
	}
}

struct MonoAttachment
{
	MonoThread* thread;

	MonoAttachment()
		: thread(nullptr)
	{
		if (!mono_domain_get())
		{
			thread = mono_thread_attach(g_rootDomain);
		}
	}

	~MonoAttachment()
	{
		if (thread)
		{
			mono_thread_detach(thread);
			thread = nullptr;
		}
	}
};

DLL_EXPORT void MonoEnsureThreadAttached()
{
	static thread_local MonoAttachment attachment;
}

result_t MonoCreateObjectInstance(const guid_t& guid, const guid_t& iid, void** objectRef)
{
	MonoEnsureThreadAttached();

	MonoObject* exc = nullptr;

	guid_t lguid = guid;
	guid_t liid = iid;

	void* args[2];
	args[0] = &lguid;
	args[1] = &liid;

	auto retval = mono_runtime_invoke(g_createObjectMethod, nullptr, args, &exc);

	if (exc)
	{
        return FX_E_NOINTERFACE;
	}

	*objectRef = *(void**)(mono_object_unbox(retval));

    if (!*objectRef)
    {
        return FX_E_NOINTERFACE;
    }

	return FX_S_OK;
}

std::vector<guid_t> MonoGetImplementedClasses(const guid_t& iid)
{
	MonoEnsureThreadAttached();

	void* args[1];
	args[0] = (char*)&iid;

	MonoObject* exc = nullptr;
	MonoArray* retval = (MonoArray*)mono_runtime_invoke(g_getImplementsMethod, nullptr, args, &exc);

	if (exc)
	{
        return std::vector<guid_t>();
	}

	guid_t* retvalStart = mono_array_addr(retval, guid_t, 0);
	uintptr_t retvalLength = mono_array_length(retval);

	return std::vector<guid_t>(retvalStart, retvalStart + retvalLength);
}

static InitFunction initFunction([] ()
{
	// should've been ResourceManager but ResourceManager OnTick happens _after_ individual resource ticks
	// which is too early for on-start Mono resources to have run
	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* instance)
	{
		instance->OnTick.Connect([]()
		{
			MonoEnsureThreadAttached();
		}, INT32_MIN);
	});

	InitMono();
});
