#include "StdInc.h"
#include "MonoScriptRuntime.h"

#include "MonoComponentHost.h"
#include "MonoDomainScope.h"
#include "MonoFreeable.h"

#include "fxScriptBuffer.h"

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
* Notes while working on this environment:
*  - Scheduling: any function that can potentially add tasks to the C#'s scheduler needs to return the time
*    of when it needs to be activated again, which then needs to be scheduled in the core scheduler (bookmark).
*
*
* Some notes on mono domain switching (psuedo code):
* 
* mono_domain_set(MonoDomain* domain, bool force)  => if (!is_unloading(domain)) mono_domain_set_internal(domain);
* mono_domain_set_internal(MonoDomain* domain)     => if (domain != mono_domain_current()) code_to_set_domain(domain);
* 
* So we use mono_domain_set_internal() directly, as we control when it'll unload ourselves.
* We also don't need to do domain != current_domain checks as it's already included in mono_domain_set_internal().
*/

using namespace std::literals; // enable ""sv literals

uint64_t GetCurrentSchedulerTime()
{
	// TODO: replace this when the bookmark scheduler follows frame time instead of real time.
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

bool IsProfiling()
{
	static auto profiler = fx::ResourceManager::GetCurrent()->GetComponent<fx::ProfilerComponent>();
	return profiler->IsRecording();
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
		initialize({ mono_string_new(m_appDomain, resourceName), &thisPtr, &m_instanceId, &m_sharedData }, &exc);

		mono_domain_set_internal(mono_get_root_domain()); // back to root for v1

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
	// Technically we do not need to change domain as long as we're not on the one we unload.
	// It's purely used to set it back to by mono, but for now we do it to be 100% sure.
	mono_domain_set_internal(MonoComponentHost::GetRootDomain());

	MonoException* exc = nullptr;
	mono_domain_try_unload(m_appDomain, (MonoObject**)&exc);

	m_appDomain = nullptr;
	m_scriptHost = nullptr;

	mono_domain_set_internal(mono_get_root_domain()); // back to root for v1

	return ReturnOrError(exc);
}

result_t MonoScriptRuntime::Tick()
{
	// Tick-less: we don't pay for runtime entry and exit costs if there's nothing to do.
	{
		auto nextScheduledTime = m_sharedData.m_scheduledTime.load();
		if (GetCurrentSchedulerTime() < nextScheduledTime)
		{
			return FX_S_OK;
		}

		// We can ignore the time between the load above and the store below as the runtime will set this value again if there's still work to do
		m_sharedData.m_scheduledTime.store(~uint64_t(0));
	}

	m_handler->PushRuntime(static_cast<IScriptRuntime*>(this));
	if (m_parentObject)
		m_parentObject->OnActivate();

	mono_domain_set_internal(m_appDomain);

	MONO_BOUNDARY_START

	MonoException* exc;
	m_tick(GetCurrentSchedulerTime(), IsProfiling(), &exc);

	MONO_BOUNDARY_END

	m_handler->PopRuntime(static_cast<IScriptRuntime*>(this));
	if (m_parentObject)
		m_parentObject->OnDeactivate();

	mono_domain_set_internal(mono_get_root_domain()); // v1 requires us to be on the root domain

	return ReturnOrError(exc);
}

result_t MonoScriptRuntime::TriggerEvent(char* eventName, char* argsSerialized, uint32_t serializedSize, char* sourceId)
{
	fx::PushEnvironment env(this);
	MonoDomainScope scope(m_appDomain);

	MONO_BOUNDARY_START

	MonoException* exc = nullptr;
	m_triggerEvent(mono_string_new(m_appDomain, eventName),
		argsSerialized, serializedSize, mono_string_new(m_appDomain, sourceId),
		GetCurrentSchedulerTime(), IsProfiling(), &exc);

	MONO_BOUNDARY_END

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
	int monoRT2FlagCount = 0;
	metadata->GetNumResourceMetaData(const_cast<char*>("mono_rt2"), &monoRT2FlagCount); // should've been const qualified

	// check if mono_rt2 has been set
	if (monoRT2FlagCount == 0)
	{
		return false;
	}

	// check if file ends with .net.dll
	size_t size = strlen(filename);
	if (size <= 8 || strncmp(filename + size - 8, ".net.dll", 8) != 0)
	{
		return false;
	}

	// last supported date for this pilot of mono_rt2, in UTC
	constexpr int maxYear = 2024, maxMonth = 12, maxDay = 31;

	// Allowed values for mono_rt2
	constexpr std::string_view allowedValues[] = {
		// put latest on top, right here ↓
	    "Prerelease expiring 2024-12-31. See https://aka.cfx.re/mono-rt2-preview for info."sv,
		"Prerelease expiring 2024-06-30. See https://aka.cfx.re/mono-rt2-preview for info."sv,
		"Prerelease expiring 2024-03-31. See https://aka.cfx.re/mono-rt2-preview for info."sv,
		"Prerelease expiring 2023-12-31. See https://aka.cfx.re/mono-rt2-preview for info."sv,
		"Prerelease expiring 2023-08-31. See https://aka.cfx.re/mono-rt2-preview for info."sv,
		"Prerelease expiring 2023-06-30. See https://aka.cfx.re/mono-rt2-preview for info."sv,
	};

	// disable loading mono_rt2 scripts after maxYear-maxMonth-maxDay
	tm maxDate;
	memset(&maxDate, 0, sizeof(maxDate));
	maxDate.tm_year = maxYear - 1900; // YYYY - 1900 (starts from 1900)
	maxDate.tm_mon = maxMonth - 1;    // 0 .. 11
	maxDate.tm_mday = maxDay;         // 1 .. 31

	std::time_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	std::time_t endTime = mktime(&maxDate) + std::time_t(24 * 60 * 60); // until the end of the day

	if (currentTime > endTime)
	{
		console::PrintError(_CFX_NAME_STRING(_CFX_COMPONENT_NAME), "mono_rt2 is no longer supported since (%04d-%02d-%02d), skipped loading %s.\n",
			maxYear, maxMonth, maxDay, filename);

		return false;
	}

	// date & sentence restrictions on mono_rt2 flag, only allow loading if the value is in our array
	for (int i = 0; i < monoRT2FlagCount; ++i)
	{
		const char* flagValue = nullptr;

		// TODO: fix ill formed and/or unclear usage of non-const char* parameters
		if (FX_SUCCEEDED(metadata->GetResourceMetaData(const_cast<char*>("mono_rt2"), i, const_cast<char**>(&flagValue))))
		{
			for (auto& value : allowedValues)
			{
				if (value == flagValue)
				{
					return true;
				}
			}
		}
	}

	console::PrintError(_CFX_NAME_STRING(_CFX_COMPONENT_NAME), "mono_rt2 was requested for file %s but the value is missing or not accepted.\n"
		"\tTo continue using mono_rt2 please update your fxmanifest to:\n"
		"\tmono_rt2 '%s'\n", filename, allowedValues[0]);

	return false;
}

result_t MonoScriptRuntime::LoadFile(char* scriptFile)
{
	fx::PushEnvironment env(this);
	MonoComponentHost::EnsureThreadAttached();
	MonoDomainScope scope(m_appDomain);

	auto currentTime = GetCurrentSchedulerTime();
	bool isProfiling = IsProfiling();

	MonoException* exc = nullptr;
	m_loadAssembly({ mono_string_new(m_appDomain, scriptFile), &currentTime, &isProfiling }, &exc);
	
	console::PrintWarning(_CFX_NAME_STRING(_CFX_COMPONENT_NAME),
		"Assembly %s has been loaded into the mono rt2 runtime. This runtime is still in beta and shouldn't be used in production, "
		"crashes and breaking changes are to be expected.\n", scriptFile);

	return ReturnOrError(exc);
}

result_t MonoScriptRuntime::CallRef(int32_t refIndex, char* argsSerialized, uint32_t argsSize, IScriptBuffer** buffer)
{
	fx::PushEnvironment env(this);
	MonoComponentHost::EnsureThreadAttached();
	MonoDomainScope scope(m_appDomain);

	MonoArray* retval = nullptr;
	MonoException* exc = nullptr;
	m_callRef(refIndex, argsSerialized, argsSize, &retval, GetCurrentSchedulerTime(), IsProfiling(), &exc);

	*buffer = nullptr;

	if (retval)
	{
		char* retvalStart = mono_array_addr(retval, char, 0);
		uintptr_t retvalLength = mono_array_length(retval);

		auto rvb = fx::MemoryScriptBuffer::Make(retvalStart, retvalLength);
		rvb.CopyTo(buffer);
	}

	return ReturnOrError(exc);
}

result_t MonoScriptRuntime::DuplicateRef(int32_t refIndex, int32_t* newRefIdx)
{
	fx::PushEnvironment env(this);
	MonoComponentHost::EnsureThreadAttached();
	MonoDomainScope scope(m_appDomain);

	MonoException* exc = nullptr;
	m_duplicateRef(refIndex, newRefIdx, &exc);

	return ReturnOrError(exc);
}

result_t MonoScriptRuntime::RemoveRef(int32_t refIndex)
{
	fx::PushEnvironment env(this);
	MonoComponentHost::EnsureThreadAttached();
	MonoDomainScope scope(m_appDomain);

	MonoException* exc = nullptr;
	m_removeRef(refIndex, &exc);

	return ReturnOrError(exc);
}

MonoArray* MonoScriptRuntime::CanonicalizeRef(int referenceId) const
{
	char* str = nullptr;
	result_t hr = m_scriptHost->CanonicalizeRef(referenceId, m_instanceId, &str);
	size_t size = strlen(str) + 1; // also get and copy '\0'

	MonoArray* arr = mono_array_new(m_appDomain, mono_get_byte_class(), size);
	memcpy(mono_array_addr_with_size(arr, 1, 0), str, size);

	fwFree(str);

	return arr;
}

MonoArray* MonoScriptRuntime::InvokeFunctionReference(MonoString* referenceId, MonoArray* argsSerialized) const
{
	std::string referenceString = UTF8CString(referenceId);

	char* argsStart = mono_array_addr(argsSerialized, char, 0);
	uintptr_t argsLength = mono_array_length(argsSerialized);

	fx::OMPtr<IScriptBuffer> retval;
	result_t hr = m_scriptHost->InvokeFunctionReference(const_cast<char*>(referenceString.c_str()), argsStart, argsLength, retval.GetAddressOf());
	size_t size = (retval.GetRef()) ? retval->GetLength() : 0;

	MonoArray* arr = mono_array_new(m_appDomain, mono_get_byte_class(), size);

	if (size)
	{
		memcpy(mono_array_addr_with_size(arr, 1, 0), retval->GetBytes(), size);
	}

	return arr;
}

result_t MonoScriptRuntime::RequestMemoryUsage()
{
	return FX_S_OK;
}

result_t MonoScriptRuntime::GetMemoryUsage(int64_t* memoryUsage)
{
	MonoComponentHost::EnsureThreadAttached();
	MonoDomainScope scope(m_appDomain);

	*memoryUsage = MonoComponentHostShared::GetMemoryUsage();

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
	MonoDomainScope scope(MonoComponentHost::GetRootDomain());

	trace("begin m_startProfiling\n");
	MonoException* exc = nullptr;
	m_startProfiling(&exc);
	trace("end m_startProfiling\n");

	return ReturnOrError(exc);
}

result_t MonoScriptRuntime::ShutdownFxProfiler()
{
	fx::PushEnvironment env(this);
	MonoComponentHost::EnsureThreadAttached();
	MonoDomainScope scope(MonoComponentHost::GetRootDomain());

	trace("begin m_stopProfiling\n");
	MonoException* exc = nullptr;
	m_stopProfiling(&exc);
	trace("end m_stopProfiling\n");

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
			*assembly = mono_array_new(m_appDomain, mono_get_byte_class(), length);
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
			*symbols = mono_array_new(m_appDomain, mono_get_byte_class(), length);
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
