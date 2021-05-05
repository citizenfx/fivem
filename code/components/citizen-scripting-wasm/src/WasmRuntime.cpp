#include "StdInc.h"
#include "fxScripting.h"
#include <ManifestVersion.h>

#include <cfx-wasm-runtime.h>

#include <ResourceCallbackComponent.h>

#ifdef _WIN32
#pragma comment(lib, "userenv")
#pragma comment(lib, "bcrypt")
#endif

#include <CoreConsole.h>

#include <om/OMComponent.h>
#include <Resource.h>

// TODO: Better tracing
void ScriptTraceV(const char* string, fmt::printf_args formatList)
{
	auto t = fmt::vsprintf(string, formatList);
	console::Printf(fmt::sprintf("script:%s", "WASM"), "%s\n", t);

	// console::Printf(fmt::sprintf("script:%s", WasmRuntime::GetCurrent()->GetResourceName()), "%s", t);

	// WasmRuntime::GetCurrent()->GetScriptHost()->ScriptTrace(const_cast<char*>(t.c_str()));
}

template<typename... TArgs>
void ScriptTrace(const char* string, const TArgs&... args)
{
	ScriptTraceV(string, fmt::make_printf_args(args...));
}

namespace fx
{

// TODO: Return result

static void Invoke(fxNativeContext* ctx);
static void HostLog(const char* string);

class WasmRuntime : public OMClass<WasmRuntime, IScriptRuntime, IScriptFileHandlingRuntime, IScriptTickRuntime, IScriptEventRuntime, IScriptMemInfoRuntime>
{
private:
	int m_instanceId;
	void* m_parentObject;
	void* m_runtime;

	IScriptHost* m_scriptHost;
	IScriptHostWithResourceData* m_resourceHost;

public:
	inline WasmRuntime()
	{
		m_instanceId = rand() ^ 0xDEAD;
		m_parentObject = nullptr;
		m_scriptHost = nullptr;
		m_resourceHost = nullptr;
		m_runtime = wasm_create_runtime();

		wasm_set_logger_function((void (*)(const int8_t*))HostLog);
		wasm_set_invoke_native((void (*)(void*))Invoke);
	}

	~WasmRuntime()
	{
		wasm_destroy_runtime(m_runtime);
	}

	static const OMPtr<WasmRuntime>& GetCurrent();

	inline OMPtr<IScriptHost> GetScriptHost()
	{
		return m_scriptHost;
	}

	inline const char* GetResourceName()
	{
		char* resourceName = "";
		m_resourceHost->GetResourceName(&resourceName);

		return resourceName;
	}

	NS_DECL_ISCRIPTRUNTIME;
	NS_DECL_ISCRIPTFILEHANDLINGRUNTIME;
	NS_DECL_ISCRIPTTICKRUNTIME;
	NS_DECL_ISCRIPTEVENTRUNTIME;
	NS_DECL_ISCRIPTMEMINFORUNTIME;
};

static OMPtr<WasmRuntime> g_currentWasmRuntime;
static IScriptHost* g_lastScriptHost;

const OMPtr<WasmRuntime>& WasmRuntime::GetCurrent()
{
	return g_currentWasmRuntime;
}

class WasmPushEnvironment
{
private:
	fx::PushEnvironment m_pushEnvironment;

	OMPtr<WasmRuntime> m_lastWasmRuntime;

public:
	inline WasmPushEnvironment(WasmRuntime* runtime)
		: m_pushEnvironment(runtime)
	{
		g_lastScriptHost = runtime->GetScriptHost().GetRef();

		m_lastWasmRuntime = g_currentWasmRuntime;
		g_currentWasmRuntime = runtime;
	}

	inline ~WasmPushEnvironment()
	{
		g_currentWasmRuntime = m_lastWasmRuntime;
	}
};

result_t WasmRuntime::Create(IScriptHost* scriptHost)
{
	m_scriptHost = scriptHost;

	{
		fx::OMPtr<IScriptHost> ptr(scriptHost);
		fx::OMPtr<IScriptHostWithResourceData> resourcePtr;
		ptr.As(&resourcePtr);

		m_resourceHost = resourcePtr.GetRef();
	}

	return FX_S_OK;
}

result_t WasmRuntime::Destroy()
{
	wasm_runtime_destroy_module(m_runtime);

	return FX_S_OK;
}

void* WasmRuntime::GetParentObject()
{
	return m_parentObject;
}

void WasmRuntime::SetParentObject(void* parentObject)
{
	m_parentObject = parentObject;
}

result_t WasmRuntime::LoadFile(char* scriptName)
{
	WasmPushEnvironment pushed(this);

	OMPtr<fxIStream> stream;

	result_t result = m_scriptHost->OpenHostFile(scriptName, stream.GetAddressOf());

	if (FX_FAILED(result))
	{
		ScriptTrace("WASM couldn't load %s file", scriptName);
		return result;
	}

	uint64_t length;
	result_t hr;

	if (FX_FAILED(hr = stream->GetLength(&length)))
	{
		return hr;
	}

	std::vector<uint8_t> fileData(length);

	if (FX_FAILED(hr = stream->Read(fileData.data(), length, nullptr)))
	{
		return hr;
	}

	wasm_runtime_create_module(m_runtime, fileData.data(), length);

	return FX_S_OK;
}

int WasmRuntime::GetInstanceId()
{
	return m_instanceId;
}

int32_t WasmRuntime::HandlesFile(char* fileName, IScriptHostWithResourceData* metadata)
{
	return strstr(fileName, ".wasm") != 0;
}

result_t WasmRuntime::Tick()
{
	{
		WasmPushEnvironment pushed(this);
		wasm_runtime_tick(m_runtime);
	}

	return FX_S_OK;
}

result_t WasmRuntime::TriggerEvent(char* eventName, char* eventPayload, uint32_t payloadSize, char* eventSource)
{
	{
		WasmPushEnvironment pushed(this);
		wasm_runtime_trigger_event(m_runtime, (const int8_t*)eventName, (const uint8_t*)eventPayload, payloadSize, (const int8_t*)eventSource);
	}

	return FX_S_OK;
}

result_t WasmRuntime::RequestMemoryUsage()
{
	return FX_S_OK;
}

result_t WasmRuntime::GetMemoryUsage(int64_t* memoryUsage)
{
	WasmPushEnvironment pushed(this);
	*memoryUsage = wasm_runtime_memory_usage(m_runtime);

	return FX_S_OK;
}

static void Invoke(fxNativeContext* ctx)
{
	auto host = WasmRuntime::GetCurrent()->GetScriptHost();
	host->InvokeNative(*ctx);
}

static void HostLog(const char* string)
{
	ScriptTrace(string);
}

static InitFunction initFunction([]()
{
	// TODO: ?
	ResourceManager::OnInitializeInstance.Connect([](ResourceManager* manager)
	{
		manager->OnTick.Connect([]()
		{

		});
	});
});

// {EA3BEC5E-2C92-4519-AC17-80DE4A41B5FF}
FX_DEFINE_GUID(CLSID_WasmRuntime,
0xea3bec5e, 0x2c92, 0x4519, 0xac, 0x17, 0x80, 0xde, 0x4a, 0x41, 0xb5, 0xff);

FX_NEW_FACTORY(WasmRuntime);

FX_IMPLEMENTS(CLSID_WasmRuntime, IScriptRuntime);
FX_IMPLEMENTS(CLSID_WasmRuntime, IScriptFileHandlingRuntime);

}
