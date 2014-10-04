#include <StdInc.h>
#include "MissionCleanup.h"
#include "MonoScriptEnvironment.h"

extern "C"
{
	#include <mono/metadata/security-core-clr.h>
}

static MonoDomain* g_rootDomain;

MonoScriptEnvironment::MonoScriptEnvironment(Resource* resource)
	: m_resource(resource)
{
	m_scriptDomain = mono_domain_create_appdomain(const_cast<char*>(resource->GetName().c_str()), nullptr);

	srand(GetTickCount());

	m_instanceId = rand() | 0x80000;

	m_missionCleanup = std::make_shared<CMissionCleanup>();
}

MonoScriptEnvironment::~MonoScriptEnvironment()
{
	mono_domain_unload(m_scriptDomain);
	mono_domain_set(g_rootDomain, true);
}

bool MonoScriptEnvironment::Create()
{
	PushEnvironment pushEnvironment(this);

	std::wstring platformPath = MakeRelativeCitPath(L"citizen\\clr\\lib\\mono\\4.5\\CitizenFX.Core.dll");
	char clrPath[256];
	wcstombs(clrPath, platformPath.c_str(), sizeof(clrPath));

	m_scriptManagerAssembly = mono_domain_assembly_open(m_scriptDomain, clrPath);

	if (!m_scriptManagerAssembly)
	{
		trace("Could not load CitizenFX.Core.dll.\n");
		return false;
	}

	m_scriptManagerImage = mono_assembly_get_image(m_scriptManagerAssembly);

	bool methodSearchSuccess = true;
	MonoMethodDesc* description;

#define method_search(name, method) description = mono_method_desc_new(name, 1); \
			method = mono_method_desc_search_in_image(description, m_scriptManagerImage); \
			mono_method_desc_free(description); \
			methodSearchSuccess = methodSearchSuccess && method != NULL

	MonoMethod* scriptInitMethod;
	method_search("CitizenFX.Core.RuntimeManager:Initialize", scriptInitMethod);
	method_search("CitizenFX.Core.RuntimeManager:LoadScripts", m_scriptLoadMethod);
	method_search("CitizenFX.Core.RuntimeManager:Tick", m_scriptTickMethod);
	method_search("CitizenFX.Core.RuntimeManager:TriggerEvent", m_scriptEventMethod);
	method_search("CitizenFX.Core.RuntimeManager:CallRef", m_scriptCallRefMethod);
	method_search("CitizenFX.Core.RuntimeManager:RemoveRef", m_scriptRemoveRefMethod);

	MonoAddInternalCalls();

	if (!methodSearchSuccess)
	{
		trace("Couldn't find one or more CitizenFX.Core methods.\n");
		return false;
	}

	MonoObject* exc = nullptr;
	mono_runtime_invoke(scriptInitMethod, nullptr, nullptr, &exc);

	if (exc)
	{
		OutputExceptionDetails(exc);

		return false;
	}

	return true;
}

void MonoScriptEnvironment::Destroy()
{

}

void MonoScriptEnvironment::Tick()
{
	PushEnvironment pushEnvironment(this);

	MonoObject* exc = nullptr;
	mono_runtime_invoke(m_scriptTickMethod, nullptr, nullptr, &exc);

	if (exc)
	{
		OutputExceptionDetails(exc);
	}
}

bool MonoScriptEnvironment::DoInitFile(bool isPreParse)
{
	return true;
}

bool MonoScriptEnvironment::LoadScripts()
{
	PushEnvironment pushEnvironment(this);

	MonoObject* exc = nullptr;
	mono_runtime_invoke(m_scriptLoadMethod, nullptr, nullptr, &exc);

	if (exc)
	{
		OutputExceptionDetails(exc);
	}

	return true;
}

fwString MonoScriptEnvironment::CallExport(ScriptFunctionRef ref, fwString& argsSerialized)
{
	PushEnvironment pushEnvironment(this);

	MonoArray* argsArray = mono_array_new(mono_domain_get(), mono_get_byte_class(), argsSerialized.size());

	char* argsAddr = mono_array_addr(argsArray, char, 0);
	memcpy(argsAddr, argsSerialized.c_str(), argsSerialized.size());

	void* args[2];
	args[0] = &ref;
	args[1] = argsArray;

	MonoObject* exc = nullptr;
	MonoArray* retval = (MonoArray*)mono_runtime_invoke(m_scriptCallRefMethod, nullptr, args, &exc);

	if (exc)
	{
		OutputExceptionDetails(exc);
	}

	// create a return value
	char* retvalStart = mono_array_addr(retval, char, 0);
	uintptr_t retvalLength = mono_array_length(retval);

	// prepare input args
	return fwString(retvalStart, retvalLength);
}

int MonoScriptEnvironment::DuplicateRef(ScriptFunctionRef ref)
{
	return -1;
}

void MonoScriptEnvironment::RemoveRef(ScriptFunctionRef ref)
{
	PushEnvironment pushEnvironment(this);

	void* args[1];
	args[0] = &ref;

	MonoObject* exc = nullptr;
	mono_runtime_invoke(m_scriptRemoveRefMethod, nullptr, args, &exc);

	if (exc)
	{
		OutputExceptionDetails(exc);
	}
}

void MonoScriptEnvironment::TriggerEvent(fwString& eventName, fwString& argsSerialized, int source)
{
	PushEnvironment pushEnvironment(this);

	MonoString* eventNameStr = mono_string_new(mono_domain_get(), eventName.c_str());
	MonoArray* argsArray = mono_array_new(mono_domain_get(), mono_get_byte_class(), argsSerialized.size());

	char* argsAddr = mono_array_addr(argsArray, char, 0);
	memcpy(argsAddr, argsSerialized.c_str(), argsSerialized.size());
	
	void* args[3];
	args[0] = eventNameStr;
	args[1] = argsArray;
	args[2] = &source;

	MonoObject* exc = nullptr;
	mono_runtime_invoke(m_scriptEventMethod, nullptr, args, &exc);

	if (exc)
	{
		OutputExceptionDetails(exc);
	}
}

uint32_t MonoScriptEnvironment::GetInstanceId()
{
	return m_instanceId;
}

const char* MonoScriptEnvironment::GetEnvironmentName()
{
	return "Mono";
}

CMissionCleanup* MonoScriptEnvironment::GetMissionCleanup()
{
	return m_missionCleanup.get();
}

void MonoScriptEnvironment::OutputExceptionDetails(MonoObject* exc)
{
	MonoClass* eclass = mono_object_get_class(exc);

	if (eclass)
	{
		MonoProperty* prop = mono_class_get_property_from_name(eclass, "Message");
		MonoMethod* getter = mono_property_get_get_method(prop);
		MonoString* msg = (MonoString*)mono_runtime_invoke(getter, exc, NULL, NULL);

		GlobalError("Unhandled exception in Mono script environment: %s", mono_string_to_utf8(msg));
	}
}

void MonoScriptEnvironment::Activate()
{
	mono_domain_set(m_scriptDomain, true);
}

const static char* g_platformAssemblies[] =
{
	"mscorlib.dll",
	"System.dll",
	"System.Core.dll",
	"CitizenFX.Core.dll",
	"Mono.CSharp.dll"
};

static int CoreClrCallback(const char* imageName)
{
	if (!imageName)
	{
		return FALSE;
	}

	char* filePart = nullptr;
	char fullPath[512];
	
	if (GetFullPathNameA(imageName, sizeof(fullPath), fullPath, &filePart) == 0)
	{
		return FALSE;
	}

	if (!filePart)
	{
		return FALSE;
	}

	*(filePart - 1) = '\0';

	std::wstring platformPath = MakeRelativeCitPath(L"citizen\\clr\\lib");
	char clrPath[256];
	wcstombs(clrPath, platformPath.c_str(), sizeof(clrPath));

	if (_stricmp(clrPath, fullPath) != 0)
	{
		platformPath = MakeRelativeCitPath(L"citizen\\clr\\lib\\mono\\4.5");
		wcstombs(clrPath, platformPath.c_str(), sizeof(clrPath));

		if (_stricmp(clrPath, fullPath) != 0)
		{
			trace("%s %s is not a platform image.\n", fullPath, filePart);
			return FALSE;
		}
	}

	for (int i = 0; i < _countof(g_platformAssemblies); i++)
	{
		if (!_stricmp(filePart, g_platformAssemblies[i]))
		{
			trace("%s %s is a platform image.\n", fullPath, filePart);
			return TRUE;
		}
	}

	trace("%s %s is not a platform image (even though the dir matches).\n", fullPath, filePart);

	return FALSE;
}

static InitFunction initFunction([] ()
{
	// initialize Mono
	mono_set_dirs("citizen/clr/lib/", "citizen/clr/cfg/");

	std::wstring citizenClrPath = MakeRelativeCitPath(L"citizen/clr/lib/");
	char clrPath[256];
	wcstombs(clrPath, citizenClrPath.c_str(), sizeof(clrPath));

	std::wstring citizenClrLibPath = MakeRelativeCitPath(L"citizen/clr/lib/mono/4.5/");

	SetEnvironmentVariable(L"MONO_PATH", citizenClrLibPath.c_str());

	mono_assembly_setrootdir(clrPath);

	mono_security_enable_core_clr();
	mono_security_core_clr_set_options((MonoSecurityCoreCLROptions)(MONO_SECURITY_CORE_CLR_OPTIONS_RELAX_DELEGATE | MONO_SECURITY_CORE_CLR_OPTIONS_RELAX_REFLECTION));
	mono_security_set_core_clr_platform_callback(CoreClrCallback);

	g_rootDomain = mono_jit_init_version("Citizen", "v4.0.30319");

	char* args[1];
	args[0] = "--soft-breakpoints";

	mono_jit_parse_options(1, args);

	CMissionCleanup::OnQueryMissionCleanup.Connect([] (CMissionCleanup*& handler)
	{
		auto environment = dynamic_cast<MonoScriptEnvironment*>(BaseScriptEnvironment::GetCurrentEnvironment());

		if (environment)
		{
			handler = environment->GetMissionCleanup();
		}
	});

	PushEnvironment::OnDeactivateLastEnvironment.Connect([] ()
	{
		mono_domain_set(g_rootDomain, true);
	});

	Resource::OnSetMetaData.Connect([] (fwRefContainer<Resource> resource, fwString key, fwString value)
	{
		if (key == "clr_solution")
		{
			fwRefContainer<BaseScriptEnvironment> monoSE = new MonoScriptEnvironment(resource.GetRef());

			if (monoSE->Create())
			{
				resource->AddScriptEnvironment(monoSE);
			}
		}
	});
});

// dllmain? yep.
__declspec(thread) MonoThread* monoThread;

DWORD WINAPI DllMain(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved)
{
	if (g_rootDomain)
	{
		if (dwReason == DLL_THREAD_ATTACH)
		{
			monoThread = mono_thread_attach(g_rootDomain);
		}
		else if (dwReason == DLL_THREAD_DETACH)
		{
			if (monoThread)
			{
				// causes some weird assertions in mono
				//mono_thread_detach(monoThread);
			}
		}
	}
	else
	{
		if (dwReason == DLL_THREAD_ATTACH)
		{
			monoThread = nullptr;
		}
	}

	return TRUE;
}