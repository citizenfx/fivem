#include "StdInc.h"
#include "MonoScriptRuntime.h"

#include "MonoComponentHost.h"
#include "MonoFreeable.h"

#include <msgpack.hpp>
#include <Profiler.h>

#include <om/OMPtr.h>

#include <mono/jit/jit.h>
#include <mono/utils/mono-logger.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/threads.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/mono-gc.h>
#include <mono/metadata/exception.h>

/*
* Some notes on mono domain switching (psuedo code):
* 
* mono_domain_set(MonoDomain* domain, bool force)  => if (!is_unloading(domain)) mono_domain_set_internal(domain);
* mono_domain_set_internal(MonoDomain* domain)     => if (domain != mono_domain_current()) code_to_set_domain(domain);
* 
* So we use mono_domain_set_internal() directly, as we control when it'll unload ourselves.
* We also don't need to do domain != current_domain checks as it's already included in mono_domain_set_internal().
*/

// backwards compatibility, without this the v1 runtime will crash
static void back_to_root_domain()
{
	mono_domain_set_internal(mono_get_root_domain());
}

namespace fx::mono
{
struct MonoPushEnvironment
{
	fx::PushEnvironment m_pushEnvironment;

	MonoPushEnvironment(MonoScriptRuntime* runTime)
		: m_pushEnvironment(runTime)
	{
	}
};

struct MonoBoundary
{
	int32_t domain;
	std::thread::id threadId;
};

#ifdef MONO_BOUNDARIES_ENABLED
#define MONO_BOUNDARY_START                                             \
	MonoBoundary boundary{ m_appDomainId, std::this_thread::get_id() }; \
	m_scriptHost->SubmitBoundaryStart((char*)&boundary, sizeof(MonoBoundary));

#define MONO_BOUNDARY_END m_scriptHost->SubmitBoundaryEnd((char*)&boundary, sizeof(MonoBoundary));
#else
#define MONO_BOUNDARY_START
#define MONO_BOUNDARY_END
#endif // MONO_BOUNDARIES_ENABLED

inline static result_t ReturnOrError(MonoException* exc)
{
	if (exc == nullptr)
		return FX_S_OK;

	MonoComponentHostShared::PrintException((MonoObject*)exc, false);
	return FX_E_INVALIDARG;
}

result_t MonoScriptRuntime::Create(IScriptHost* host)
{
	try
	{
		m_scriptHost = host;

		assert(FX_SUCCEEDED(fx::MakeInterface(&m_handler, CLSID_ScriptRuntimeHandler)));

		{
			fx::OMPtr<IScriptHost> ptr(host);
			fx::OMPtr<IScriptHostWithResourceData> resourcePtr;
			ptr.As(&resourcePtr);

			m_resourceHost = resourcePtr.GetRef();

			fx::OMPtr<IScriptHostWithManifest> manifestPtr;
			ptr.As(&manifestPtr);

			m_manifestHost = manifestPtr.GetRef();
		}

		char* resourceName = nullptr;
		m_resourceHost->GetResourceName(&resourceName);
		m_resourceName = resourceName;

		fx::PushEnvironment env(this);
		mono_thread_attach(MonoComponentHost::GetRootDomain());
		mono_domain_set_internal(MonoComponentHost::GetRootDomain());

		fx::ResourceManager* resourceManager = fx::ResourceManager::GetCurrent();
		fwRefContainer<fx::Resource> resource = resourceManager->GetResource(resourceName);
		std::string resourcePath = resource->GetPath();

		m_appDomain = mono_domain_create_appdomain(const_cast<char*>(m_name.c_str()), const_cast<char*>("dummy.config"));
		m_appDomainId = mono_domain_get_id(m_appDomain);
		mono_domain_set_config(m_appDomain, resourcePath.c_str(), const_cast<char*>("dummy.config"));
		mono_domain_set_internal(m_appDomain);

		MonoImage* image = MonoComponentHost::GetRootImage();
		InitializeMethods(image);

		auto* thisPtr = this;
		MonoException* exc;
		auto initialize = Method::Find(image, "CitizenFX.Core.ScriptInterface:Initialize");
		initialize({ mono_string_new(MonoComponentHost::GetRootDomain(), resourceName), &thisPtr, &m_instanceId }, &exc);

		return ReturnOrError(exc);
	}
	catch (std::exception& e)
	{
		trace(e.what());
	}

	return FX_E_INVALIDARG;
}

void MonoScriptRuntime::InitializeMethods(MonoImage* image)
{
	// NOTE: add all these methods' names to the list of MonoComponentHost.cpp, bottom of void InitMono(), to do a startup check on all of them

	m_tick = Method::Find(image, "CitizenFX.Core.ScriptInterface:Tick");
	m_triggerEvent = Method::Find(image, "CitizenFX.Core.ScriptInterface:TriggerEvent");
	m_loadAssembly = Method::Find(image, "CitizenFX.Core.ScriptInterface:LoadAssembly");

	m_callRef = Method::Find(image, "CitizenFX.Core.ScriptInterface:CallRef");
	m_duplicateRef = Method::Find(image, "CitizenFX.Core.ScriptInterface:DuplicateRef");
	m_removeRef = Method::Find(image, "CitizenFX.Core.ScriptInterface:RemoveRef");
}

result_t MonoScriptRuntime::Destroy()
{
	mono_domain_set_internal(MonoComponentHost::GetRootDomain()); // not doing this crashes the unloading of this app domain
	mono_domain_unload(m_appDomain);

	m_appDomain = nullptr;
	m_scriptHost = nullptr;

	back_to_root_domain();

	return FX_S_OK;
}

result_t MonoScriptRuntime::Tick()
{
	static auto profiler = fx::ResourceManager::GetCurrent()->GetComponent<fx::ProfilerComponent>();

	// m_handler->PushRuntime(static_cast<IScriptRuntime*>(this));
	if (m_parentObject)
		m_parentObject->OnActivate();

	mono_domain_set_internal(m_appDomain);

	MONO_BOUNDARY_START

	MonoException* exc;
	m_tick(profiler->IsRecording(), &exc);

	MONO_BOUNDARY_END

	// m_handler->PopRuntime(static_cast<IScriptRuntime*>(this));
	if (m_parentObject)
		m_parentObject->OnDeactivate();

	back_to_root_domain();

	return ReturnOrError(exc);
}

result_t MonoScriptRuntime::TriggerEvent(char* eventName, char* argsSerialized, uint32_t serializedSize, char* sourceId)
{
	fx::PushEnvironment env(this);
	mono_domain_set_internal(m_appDomain);

	MONO_BOUNDARY_START

	MonoException* exc = nullptr;
	m_triggerEvent(mono_string_new(MonoComponentHost::GetRootDomain(), eventName), argsSerialized, serializedSize, mono_string_new(MonoComponentHost::GetRootDomain(), sourceId), &exc);

	MONO_BOUNDARY_END

	back_to_root_domain();

	return ReturnOrError(exc);
}

// #TODO: make const correct
void* MonoScriptRuntime::GetParentObject()
{
	return m_parentObject;
}

void MonoScriptRuntime::SetParentObject(void* ptr)
{
	m_parentObject = reinterpret_cast<fx::Resource*>(ptr);
}

// #TODO: make const correct
int MonoScriptRuntime::GetInstanceId()
{
	return m_instanceId;
}

int MonoScriptRuntime::HandlesFile(char* filename, IScriptHostWithResourceData* metadata)
{
	int enableV2 = 0;
	metadata->GetNumResourceMetaData(const_cast<char*>("mono_rt2"), &enableV2); // should've been const qualified

	if (enableV2 > 0)
	{
		size_t size = strlen(filename);
		return size > 8 && memcmp(filename + size - 8, ".net.dll", 8) == 0;
	}

	return false;
}

result_t MonoScriptRuntime::LoadFile(char* scriptFile)
{
	fx::PushEnvironment env(this);
	MonoComponentHost::EnsureThreadAttached();
	mono_domain_set_internal(m_appDomain);

	MonoException* exc = nullptr;
	m_loadAssembly({ mono_string_new(MonoComponentHost::GetRootDomain(), scriptFile) }, &exc);

	mono_domain_set_internal(MonoComponentHost::GetRootDomain());

	return ReturnOrError(exc);
}

result_t MonoScriptRuntime::CallRef(int32_t refIndex, char* argsSerialized, uint32_t argsSize, char** retvalSerialized, uint32_t* retvalSize)
{
	*retvalSerialized = nullptr;
	*retvalSize = 0;

	fx::PushEnvironment env(this);
	MonoComponentHost::EnsureThreadAttached();
	mono_domain_set_internal(m_appDomain);

	MonoException* exc = nullptr;
	m_callRef(refIndex, argsSerialized, argsSize, retvalSerialized, retvalSize, &exc);

	back_to_root_domain();

	return ReturnOrError(exc);
}

result_t MonoScriptRuntime::DuplicateRef(int32_t refIndex, int32_t* newRefIdx)
{
	fx::PushEnvironment env(this);
	MonoComponentHost::EnsureThreadAttached();
	mono_domain_set_internal(m_appDomain);

	MonoException* exc = nullptr;
	m_duplicateRef(refIndex, newRefIdx, &exc);

	back_to_root_domain();

	return ReturnOrError(exc);
}

result_t MonoScriptRuntime::RemoveRef(int32_t refIndex)
{
	fx::PushEnvironment env(this);
	MonoComponentHost::EnsureThreadAttached();
	mono_domain_set_internal(m_appDomain);

	MonoException* exc = nullptr;
	m_removeRef(refIndex, &exc);

	back_to_root_domain();

	return ReturnOrError(exc);
}

MonoArray* MonoScriptRuntime::CanonicalizeRef(int referenceId) const
{
	char* str = nullptr;
	result_t hr = m_scriptHost->CanonicalizeRef(referenceId, m_instanceId, &str);
	size_t size = strlen(str) + 1; // also get and copy '\0'

	MonoArray* arr = mono_array_new(MonoComponentHost::GetRootDomain(), mono_get_byte_class(), size);
	memcpy(mono_array_addr_with_size(arr, 1, 0), str, size);

	fwFree(str);

	return arr;
}

result_t MonoScriptRuntime::RequestMemoryUsage()
{
	return FX_S_OK;
}

result_t MonoScriptRuntime::GetMemoryUsage(int64_t* memoryUsage)
{
	MonoComponentHost::EnsureThreadAttached();
	mono_domain_set_internal(m_appDomain);

	*memoryUsage = MonoComponentHostShared::GetMemoryUsage();

	back_to_root_domain();

	return FX_S_OK;
}

result_t MonoScriptRuntime::SetScriptIdentifier(char* fileName, int32_t scriptId)
{
	m_scriptIds[fileName] = scriptId;

	return FX_S_OK;
}

result_t MonoScriptRuntime::SetDebugEventListener(IDebugEventListener* listener)
{
	m_debugListener = listener;

	return FX_S_OK;
}

result_t MonoScriptRuntime::SetupFxProfiler(void* obj, int32_t resourceId)
{
	fx::PushEnvironment env(this);
	MonoComponentHost::EnsureThreadAttached();
	mono_domain_set_internal(m_appDomain);

	trace("begin m_startProfiling\n");
	MonoException* exc = nullptr;
	m_startProfiling(&exc);
	trace("end m_startProfiling\n");

	back_to_root_domain();

	return ReturnOrError(exc);
}

result_t MonoScriptRuntime::ShutdownFxProfiler()
{
	fx::PushEnvironment env(this);
	MonoComponentHost::EnsureThreadAttached();
	mono_domain_set_internal(m_appDomain);

	trace("begin m_stopProfiling\n");
	MonoException* exc = nullptr;
	m_stopProfiling(&exc);
	trace("end m_stopProfiling\n");

	back_to_root_domain();

	return ReturnOrError(exc);
}

bool MonoScriptRuntime::ReadAssembly(MonoString* name, MonoArray** assembly, MonoArray** symbols) const
{
	// no need for high performance.
	std::string assemblyName = UTF8CString(name); // UTF8CString will free the intermediate utf8 c string for us

	if (memcmp(assemblyName.data() + assemblyName.size() - 4, ".dll", 4) != 0)
		assemblyName += ".dll";

	fx::OMPtr<fxIStream> stream;
	result_t hr = m_scriptHost->OpenHostFile(const_cast<char*>(assemblyName.c_str()), stream.GetAddressOf()); // should've been const qualified

	if (FX_SUCCEEDED(hr))
	{
		{
			size_t length = 0;
			uint32_t read;

			stream->GetLength(&length);
			*assembly = mono_array_new(MonoComponentHost::GetRootDomain(), mono_get_byte_class(), length);
			hr = stream->Read(mono_array_addr_with_size(*assembly, sizeof(char), 0), length, &read);

			stream.ReleaseAndGetAddressOf();
		}

		// load symbols
		std::string symbolsName = assemblyName + ".mdb";
		hr = m_scriptHost->OpenHostFile(const_cast<char*>(symbolsName.c_str()), stream.ReleaseAndGetAddressOf());
		if (FX_FAILED(hr))
		{
			symbolsName.erase(symbolsName.size() - (sizeof(".dll.mdb") - 1));
			symbolsName += ".pdb";
			hr = m_scriptHost->OpenHostFile(const_cast<char*>(symbolsName.c_str()), stream.ReleaseAndGetAddressOf());
		}

		if (FX_SUCCEEDED(hr))
		{
			size_t length = 0;
			uint32_t read;

			stream->GetLength(&length);
			*symbols = mono_array_new(MonoComponentHost::GetRootDomain(), mono_get_byte_class(), length);
			stream->Read(mono_array_addr_with_size(*symbols, sizeof(char), 0), length, &read);
		}

		return true;
	}

	mono_raise_exception(mono_get_exception_file_not_found(name));

	return false;
}

// {C068E0AB-DD9C-48F2-A7F3-69E866D27F17} = v1
//FX_DEFINE_GUID(CLSID_MonoScriptRuntime, 0xc068e0ab, 0xdd9c, 0x48f2, 0xa7, 0xf3, 0x69, 0xe8, 0x66, 0xd2, 0x7f, 0x17);

// {74df7d09-db7d-4c05-9788-3f80c464e14e} = v2
FX_DEFINE_GUID(CLSID_MonoScriptRuntime, 0x74df7d09, 0xdb7d, 0x4c05, 0x97, 0x88, 0x3f, 0x80, 0xc4, 0x64, 0xe1, 0x4e);

FX_NEW_FACTORY(MonoScriptRuntime);

FX_IMPLEMENTS(CLSID_MonoScriptRuntime, IScriptRuntime);
FX_IMPLEMENTS(CLSID_MonoScriptRuntime, IScriptFileHandlingRuntime);

}
