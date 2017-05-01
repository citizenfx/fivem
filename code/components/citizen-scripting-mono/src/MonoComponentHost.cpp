/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <om/OMComponent.h>

#include <fxScripting.h>

#include <Error.h>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/exception.h>
#include <mono/metadata/mono-debug.h>

extern "C"
{
	#include <mono/metadata/security-core-clr.h>
}

static MonoDomain* g_rootDomain;

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

static void OutputExceptionDetails(MonoObject* exc)
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

		GlobalError("Unhandled exception in Mono script environment: %s %s", mono_string_to_utf8(msg), mono_string_to_utf8(msg2));
	}
}

static void GI_PrintLogCall(MonoString* str)
{
	trace("%s", mono_string_to_utf8(str));
}

MonoMethod* g_getImplementsMethod;
MonoMethod* g_createObjectMethod;

static void InitMono()
{
	mono_set_dirs("citizen/clr2/lib/", "citizen/clr2/cfg/");

	std::wstring citizenClrPath = MakeRelativeCitPath(L"citizen/clr2/lib/");
	std::wstring citizenClrLibPath = MakeRelativeCitPath(L"citizen/clr2/lib/mono/4.5/");

	SetEnvironmentVariable(L"MONO_PATH", citizenClrLibPath.c_str());
	SetEnvironmentVariable(L"MONO_DEBUG", L"casts");

	mono_assembly_setrootdir(ToNarrow(citizenClrPath).c_str());
	mono_set_crash_chaining(true);

	mono_security_enable_core_clr();
	mono_security_core_clr_set_options((MonoSecurityCoreCLROptions)(MONO_SECURITY_CORE_CLR_OPTIONS_RELAX_DELEGATE | MONO_SECURITY_CORE_CLR_OPTIONS_RELAX_REFLECTION));
	mono_security_set_core_clr_platform_callback(CoreClrCallback);

	char* args[1];
	args[0] = "--soft-breakpoints";

	mono_jit_parse_options(1, args);

	mono_debug_init(MONO_DEBUG_FORMAT_MONO);

	g_rootDomain = mono_jit_init_version("Citizen", "v4.0.30319");

	mono_install_unhandled_exception_hook([] (MonoObject* exc, void*)
	{
		OutputExceptionDetails(exc);
	}, nullptr);

	mono_set_crash_chaining(true);

	mono_add_internal_call("CitizenFX.Core.GameInterface::PrintLog", GI_PrintLogCall);
	mono_add_internal_call("CitizenFX.Core.GameInterface::fwFree", fwFree);

	std::wstring platformPath = MakeRelativeCitPath(L"citizen\\clr2\\lib\\mono\\4.5\\CitizenFX.Core.dll");

	auto scriptManagerAssembly = mono_domain_assembly_open(g_rootDomain, ToNarrow(platformPath).c_str());

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

static void MonoEnsureThreadAttached()
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
	InitMono();
});