/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <om/OMComponent.h>
#include <ResourceManager.h>
#include <fxScripting.h>
#include <Error.h>

#ifndef IS_FXSERVER
#include "DeferredInitializer.h"
static std::shared_ptr<DeferredInitializer> g_monoInitializer;

static inline void WaitForMono()
{
	if (auto initializer = g_monoInitializer)
	{
		initializer->Wait();
	}
}
#else
static inline void WaitForMono()
{
}
#endif

#include <EASTL/fixed_hash_map.h>

#include <mono/jit/jit.h>
#include <mono/utils/mono-logger.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/threads.h>

#include "MonoComponentHostShared.h"

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

static void GI_PrintLogCall(MonoString* channel, MonoString* str)
{
	fx::mono::MonoComponentHostShared::Print(channel, str);
}

static uint64_t GI_GetMemoryUsage()
{
	return fx::mono::MonoComponentHostShared::GetMemoryUsage();
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
				fx::mono::MonoComponentHostShared::PrintException(exc, false);
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

#if !defined(IS_FXSERVER) || defined(_WIN32)
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
	return ToNarrow(GetAbsoluteCitPath()) + path;
#else
	return GetAbsoluteCitPath() + path;
#endif
}

static void InitMono()
{
	// initializes the mono runtime
	fx::mono::MonoComponentHostShared::Initialize();

	g_rootDomain = mono_get_root_domain();

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

#if !defined(IS_FXSERVER) || defined(_WIN32)
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
		fx::mono::MonoComponentHostShared::PrintException(exc);
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

extern "C" DLL_EXPORT void MonoEnsureThreadAttached()
{
	if (!g_rootDomain)
	{
		return;
	}

	static thread_local MonoAttachment attachment;
}

result_t MonoCreateObjectInstance(const guid_t& guid, const guid_t& iid, void** objectRef)
{
	WaitForMono();

	if (!g_rootDomain)
	{
		return FX_E_NOINTERFACE;
	}

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
	WaitForMono();

	if (!g_rootDomain)
	{
		return {};
	}

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
	fx::ResourceManager::OnInitializeInstance.Connect([](fx::ResourceManager* instance)
	{
		instance->OnTick.Connect([]()
		{
			MonoEnsureThreadAttached();
		}, INT32_MIN);
	});

#ifndef IS_FXSERVER
	g_monoInitializer = DeferredInitializer::Create([]()
	{
		InitMono();
	});
#else
	InitMono();
#endif
});
