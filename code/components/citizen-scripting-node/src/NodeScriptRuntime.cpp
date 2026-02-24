/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NodeScriptRuntime.h"
#include "NodeParentEnvironment.h"

#include <msgpack.hpp>
#include <Profiler.h>

#include <om/OMPtr.h>
#include <console/Console.h>
#include <ServerInstanceBase.h>

// NOTE: these can be reused by old node and v8 runtime and will be used by future v8 runtime too
#include "FilesystemPermissions.h"
#include "shared/BoundaryFunctions.h"
#include "shared/GenericFunctions.h"
#include "shared/GlobalFunctions.h"
#include "shared/MetaFieldFunctions.h"
#include "shared/NativeFunctions.h"
#include "shared/ProfilerFunctions.h"
#include "shared/RefFunctions.h"

#include "JavaScriptEnvironmentCode.h"
#include "NodeScopeHandler.h"
#include "VFSDevice.h"
#include "VFSManager.h"

#include "permission/permission_base.h"
#include "path.h"
#include <ServerInstanceBaseRef.h>

#include <boost/algorithm/string.hpp>

using namespace fx::v8shared;

namespace fx::nodejs
{
static NodeParentEnvironment g_nodeEnv;
static ScopeHandler g_scopeHandler;

std::unordered_map<v8::Isolate*, NodeScriptRuntime*> runtimeMap;

namespace permission
{

#define FILESYSTEM_PERMISSIONS(V)            \
	V(FileSystem, "fs", PermissionsRoot)     \
	V(FileSystemRead, "fs.read", FileSystem) \
	V(FileSystemWrite, "fs.write", FileSystem)

#define CHILD_PROCESS_PERMISSIONS(V) V(ChildProcess, "child", PermissionsRoot)

#define WASI_PERMISSIONS(V) V(WASI, "wasi", PermissionsRoot)

#define WORKER_THREADS_PERMISSIONS(V) \
	V(WorkerThreads, "worker", PermissionsRoot)

#define INSPECTOR_PERMISSIONS(V) V(Inspector, "inspector", PermissionsRoot)

#define PERMISSIONS(V)            \
	FILESYSTEM_PERMISSIONS(V)     \
	CHILD_PROCESS_PERMISSIONS(V)  \
	WASI_PERMISSIONS(V)           \
	WORKER_THREADS_PERMISSIONS(V) \
	INSPECTOR_PERMISSIONS(V)

#define V(name, _, __) k##name,
	enum class PermissionScope
	{
		kPermissionsRoot = -1,
		PERMISSIONS(V) kPermissionsCount
	};
#undef V

	// Enum to string conversion function for PermissionScope
	inline const char* ToString(PermissionScope scope)
	{
		switch (scope)
		{
			case PermissionScope::kPermissionsRoot:
				return "PermissionsRoot";
#define V(name, str, __)           \
	case PermissionScope::k##name: \
		return str;
				PERMISSIONS(V)
#undef V
			case PermissionScope::kPermissionsCount:
				return "PermissionsCount";
			default:
				return "Unknown";
		}
	}
} // namespace permission

bool NodeScriptRuntime::NodePermissionCallback(node::Environment* env,
node::permission::PermissionScope permission_,
const std::string_view& resource)
{
	fx::OMPtr<IScriptRuntime> runtime{};
	const permission::PermissionScope perm = static_cast<permission::PermissionScope>(permission_);
	const std::string permName = permission::ToString(perm);

	if (m_isMonitorRuntime)
	{
		return true;
	}

	if (m_resourceName == "yarn" || m_resourceName == "webpack")
	{
		return true;
	}

	std::string res = std::string(resource);
	if (perm == permission::PermissionScope::kFileSystem || perm == permission::PermissionScope::kFileSystemRead || perm == permission::PermissionScope::kFileSystemWrite)
	{
#ifdef _WIN32
		// Remove leading "\\?\" from UNC path
		if (res.size() > 3 && res.substr(0, 4) == R"(\\?\)")
		{
			res.erase(0, 4);
		}

		// Remove leading "UNC\" from UNC path
		if (res.size() > 3 && res.substr(0, 4) == "UNC\\")
		{
			res.erase(0, 4);
		}
		// Remove leading "//" from UNC path
		if (res.size() > 1 && res.substr(0, 2) == "//")
		{
			res.erase(0, 2);
		}
#endif

		for (const auto& part : std::filesystem::path(res))
		{
			if (part == "..")
			{
				trace("Filesystem permission check from '%s' for permission %s on resource '%s' - path traversal detected\n", m_resourceName.c_str(), permName.c_str(), res.c_str());
				return false;
			}
		}

		fwRefContainer<vfs::Device> device = !res.empty() && res[0] == '@' ? vfs::GetDevice(res) : nullptr;
		std::string path = res;
		if (!device.GetRef())
		{
			std::string absolutePath = std::filesystem::absolute(std::filesystem::path(path)).string();
			device = vfs::FindDevice(absolutePath, path);
			if (!device.GetRef())
			{
				trace("Filesystem permission check from '%s' for permission %s on resource '%s' - no device found\n", m_resourceName.c_str(), permName.c_str(), res.c_str());
				return false;
			}
		}

		if (perm == permission::PermissionScope::kFileSystemWrite && !fx::ScriptingFilesystemAllowWrite(path, m_parentObject))
		{
			trace("Filesystem write permission check from '%s' for permission %s on resource '%s' - write not allowed\n", m_resourceName.c_str(), permName.c_str(), res.c_str());
			return false;
		}
	}
	else if (perm == permission::PermissionScope::kChildProcess)
	{
		if (!fx::ScriptingChildProcessAllowSpawn(m_parentObject))
		{
			trace("Child process permission check from '%s' for permission %s on resource '%s' - child spawn not allowed\n", m_resourceName.c_str(), permName.c_str(), res.c_str());
			return false;
		}

		return true;
	}
	else if (perm == permission::PermissionScope::kWASI)
	{
		// No WASI for security
		trace("WASI permission check from '%s' for permission %s on resource '%s'\n", m_resourceName.c_str(), permName.c_str(), res.c_str());
		return false;
	}
	else if (perm == permission::PermissionScope::kWorkerThreads)
	{
		if (!fx::ScriptingWorkerAllowSpawn(m_parentObject))
		{
			trace("Worker threads permission check from '%s' for permission %s on resource '%s' - worker spawn not allowed\n", m_resourceName.c_str(), permName.c_str(), res.c_str());
			return false;
		}

		return true;
	}
	// Leaving here for future if new permissions are added
	/*else
	{
		trace("Permission check for permission %s on resource '%s'\n", permName.c_str(), res.c_str());
	}*/
	return true;
}

static InitFunction initFunction([]()
{
	// set up the environment scope handler
	if (!g_nodeEnv.IsStartNode())
	{
		g_scopeHandler.Initialize();
	}

	// trigger removing funcrefs on the *resource manager* so that it'll still happen when a runtime is destroyed
	ResourceManager::OnInitializeInstance.Connect([](ResourceManager* manager)
	{
		manager->OnTick.Connect([]()
		{
			RefAndPersistent<NodeScriptRuntime>* deleteRef;

			while (g_cleanUpFuncRefs.try_pop(reinterpret_cast<void*&>(deleteRef)))
			{
				delete deleteRef;
			}
		});
	});

	// initialize the parent node environment, should be done one time per process
	result_t hr = g_nodeEnv.Initialize();
	if (FX_FAILED(hr) || g_nodeEnv.IsStartNode())
	{
#ifdef _WIN32
		TerminateProcess(GetCurrentProcess(), hr);
#else
		exit(hr);
#endif
		return;
	}
});

// NOTE: it still depends on preexisting files from old runtime
static const char* g_platformScripts[] = {
	"citizen:/scripting/v8/natives_server.js",
	"citizen:/scripting/v8/console.js",
	"citizen:/scripting/v8/timer.js",
	"citizen:/scripting/v8/msgpack.js",
	"citizen:/scripting/v8/eventemitter2.js",
	"citizen:/scripting/v8/main.js"
};

static std::pair<std::string, v8::FunctionCallback> g_citizenFunctions[] = {
	// generic
	{ "trace", V8_Trace<NodeScriptRuntime, SharedPushEnvironmentNoContext> },
	{ "setTickFunction", V8_SetTickFunction<NodeScriptRuntime> },
	{ "setEventFunction", V8_SetEventFunction<NodeScriptRuntime> },
	{ "setStackTraceFunction", V8_SetStackTraceRoutine<NodeScriptRuntime> },
	{ "setUnhandledPromiseRejectionFunction", V8_SetUnhandledPromiseRejectionRoutine<NodeScriptRuntime> },
	{ "getTickCount", V8_GetTickCount },
	{ "getResourcePath", V8_GetResourcePath<NodeScriptRuntime> },

	// ref stuff
	{ "setCallRefFunction", V8_SetCallRefFunction<NodeScriptRuntime> },
	{ "setDeleteRefFunction", V8_SetDeleteRefFunction<NodeScriptRuntime> },
	{ "setDuplicateRefFunction", V8_SetDuplicateRefFunction<NodeScriptRuntime> },
	{ "canonicalizeRef", V8_CanonicalizeRef<NodeScriptRuntime> },
	{ "makeFunctionReference", V8_MakeFunctionReference<NodeScriptRuntime> },

	// natives
	{ "invokeNative", V8_InvokeNativeString<NodeScriptRuntime> },
	{ "invokeNativeByHash", V8_InvokeNativeHash<NodeScriptRuntime> },

	// profiler
	{ "snap", V8_Snap },
	{ "startProfiling", V8_StartProfiling },
	{ "stopProfiling", V8_StopProfiling },

	// boundary
	{ "submitBoundaryStart", V8_SubmitBoundaryStart<NodeScriptRuntime> },
	{ "submitBoundaryEnd", V8_SubmitBoundaryEnd<NodeScriptRuntime> },

	// metafields
	{ "pointerValueIntInitialized", V8_GetPointerField<MetaField::PointerValueInteger, NodeScriptRuntime> },
	{ "pointerValueFloatInitialized", V8_GetPointerField<MetaField::PointerValueFloat, NodeScriptRuntime> },
	{ "pointerValueInt", V8_GetMetaField<MetaField::PointerValueInteger> },
	{ "pointerValueFloat", V8_GetMetaField<MetaField::PointerValueFloat> },
	{ "pointerValueVector", V8_GetMetaField<MetaField::PointerValueVector> },
	{ "returnResultAnyway", V8_GetMetaField<MetaField::ReturnResultAnyway> },
	{ "resultAsInteger", V8_GetMetaField<MetaField::ResultAsInteger> },
	{ "resultAsLong", V8_GetMetaField<MetaField::ResultAsLong> },
	{ "resultAsFloat", V8_GetMetaField<MetaField::ResultAsFloat> },
	{ "resultAsString", V8_GetMetaField<MetaField::ResultAsString> },
	{ "resultAsVector", V8_GetMetaField<MetaField::ResultAsVector> },
	{ "resultAsObject2", V8_GetMetaField<MetaField::ResultAsObject> },
};

static std::pair<std::string, v8::FunctionCallback> g_globalFunctions[] = {
	{ "read", V8_Read<NodeScriptRuntime, SharedPushEnvironment<NodeScriptRuntime>> },
	{ "readbuffer", V8_ReadBuffer<NodeScriptRuntime, SharedPushEnvironment<NodeScriptRuntime>> },
};

static void OnMessage(v8::Local<v8::Message> message, v8::Local<v8::Value> error)
{
	auto isolate = message->GetIsolate();

	v8::String::Utf8Value messageStr(isolate, message->Get());
	v8::String::Utf8Value errorStr(isolate, error);

	std::stringstream stack;
	auto stackTrace = message->GetStackTrace();

	for (int i = 0; i < stackTrace->GetFrameCount(); i++)
	{
		auto frame = stackTrace->GetFrame(isolate, i);

		v8::String::Utf8Value sourceStr(isolate, frame->GetScriptNameOrSourceURL());
		v8::String::Utf8Value functionStr(isolate, frame->GetFunctionName());

		stack << (*sourceStr ? *sourceStr : "(unknown)") << "(" << frame->GetLineNumber() << "," << frame->GetColumn() << "): " << (*functionStr ? *functionStr : "") << "\n";
	}

	auto context = isolate->GetEnteredOrMicrotaskContext();
	auto data = context->GetEmbedderData(16);
	auto rt = reinterpret_cast<NodeScriptRuntime*>(v8::Local<v8::External>::Cast(data)->Value());
	ScriptTrace(rt, "%s\n%s\n%s\n", *messageStr, stack.str(), *errorStr);
}

result_t NodeScriptRuntime::Create(IScriptHost* host)
{
	// assign the script host
	m_scriptHost = host;

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

	// don't create the runtime environment if nodejs is not initialized
	if (!g_nodeEnv.IsInitialized())
	{
		console::PrintError(_CFX_NAME_STRING(_CFX_COMPONENT_NAME), "Can't create resource %s. Node is not initialized.\n", m_resourceName);
		return FX_E_INVALIDARG;
	}

	fx::ResourceManager* resourceManager = fx::ResourceManager::GetCurrent();
	const fwRefContainer<fx::Resource> resource = resourceManager->GetResource(resourceName);

	m_isMonitorRuntime = resourceManager->IsMonitor();

	// create our UV loop
	m_uvLoop = Instance<net::UvLoopManager>::Get()->GetOrCreate(std::string("svMain"))->GetLoop();
	// uv_loop_init(m_uvLoop);

	// create our isolate from parent platform
	const auto allocator = node::CreateArrayBufferAllocator();
	m_isolate = node::NewIsolate(allocator, m_uvLoop, g_nodeEnv.GetPlatform());
	m_isolateData = node::CreateIsolateData(m_isolate, m_uvLoop, g_nodeEnv.GetPlatform(), allocator);

	m_isolate->SetCaptureStackTraceForUncaughtExceptions(true);
	m_isolate->AddMessageListener(OnMessage);

	m_isolate->AddGCPrologueCallback([](v8::Isolate* isolate, v8::GCType type,
									 v8::GCCallbackFlags flags, void* data)
	{
		auto* runtime = static_cast<NodeScriptRuntime*>(data);
		runtime->m_isInGc.fetch_add(1, std::memory_order_release);
	}, this);

	m_isolate->AddGCEpilogueCallback([](v8::Isolate* isolate, v8::GCType type,
									 v8::GCCallbackFlags flags, void* data)
	{
		auto* runtime = static_cast<NodeScriptRuntime*>(data);
		runtime->m_isInGc.fetch_sub(1, std::memory_order_release);
	}, this);

	runtimeMap[m_isolate] = this;

	{
		// create a scope to hold handles we use here
		SharedPushEnvironmentNoContext pushed(m_isolate);
		fx::PushEnvironment fxenv(this);

		// create global state
		v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(m_isolate);

		// create Citizen call routines
		v8::Local<v8::ObjectTemplate> citizenObject = v8::ObjectTemplate::New(m_isolate);

		// loop through routine list
		for (auto& routine : g_citizenFunctions)
		{
			citizenObject->Set(m_isolate, routine.first.c_str(), v8::FunctionTemplate::New(m_isolate, routine.second, v8::External::New(m_isolate, this)));
		}

		// register the functions that are accessible from global scope
		for (auto& routine : g_globalFunctions)
		{
			global->Set(m_isolate, routine.first.c_str(), v8::FunctionTemplate::New(m_isolate, routine.second, v8::External::New(m_isolate, this)));
		}

		// add a global 'print' function as alias for Citizen.trace for testing
		global->Set(m_isolate, "print", v8::FunctionTemplate::New(m_isolate, V8_Trace<NodeScriptRuntime, SharedPushEnvironmentNoContext>));

		// set the Citizen object
		global->Set(m_isolate, "Citizen", citizenObject);

		// create a cfx alias too for the Citizen object
		// global->Set(m_isolate, "cfx", citizenObject);

		// create a V8 context with explicit microtask queue and store it
		m_taskQueue = v8::MicrotaskQueue::New(m_isolate, v8::MicrotasksPolicy::kExplicit);
		const auto context = v8::Context::New(m_isolate, nullptr, global, {}, {}, m_taskQueue.get());
		m_context.Reset(m_isolate, context);

		// set the 'global' variable to the global itself (?) (NOTE: only works after context creation, maybe it shouldn't be done?)
		global->Set(m_isolate, "global", global);

		// store the ScRT in the context
		context->SetEmbedderData(16, v8::External::New(m_isolate, this));

		// allow "eval" and "new Function" (required by some fivem scripts)
		context->AllowCodeGenerationFromStrings(true);

		// context scope is required for environment creation
		v8::Context::Scope scope(context);
		node::InitializeContext(context);

		// create and load our child node environment

#ifdef _WIN32
		std::string selfPath = ToNarrow(MakeRelativeCitPath(_P("FXServer.exe")));
#else
		std::string selfPath = MakeRelativeCitPath(_P("FXServer"));

		std::string rootPath = selfPath;
		boost::algorithm::replace_all(rootPath, "/opt/cfx-server/FXServer", "");

		auto libPath = fmt::sprintf("%s/usr/lib/v8/:%s/lib/:%s/usr/lib/",
		rootPath,
		rootPath,
		rootPath);
#endif

		const std::vector<std::string> execArgv = {
#ifndef _WIN32
			"--library-path",
			libPath.c_str(),
			"--",
			selfPath.c_str(),
#endif
			"--start-node",
			"--fork-node22"
		};

		// pass our own executable name and start-node parameters for process forking compatibility
		// todo: add optional permissions for node
		m_nodeEnvironment = node::CreateEnvironment(m_isolateData, context, { selfPath }, execArgv, node::EnvironmentFlags::kNoCreateInspector);
		node::SetPermissionHandler(m_nodeEnvironment, std::bind(&NodeScriptRuntime::NodePermissionCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		node::LoadEnvironment(m_nodeEnvironment, g_envCode);
	}

	result_t hr;

	// loading platform scripts
	for (const char* platformScript : g_platformScripts)
	{
		if (FX_FAILED(hr = LoadSystemFile(const_cast<char*>(platformScript))))
		{
			return hr;
		}
	}

	return FX_S_OK;
}

result_t NodeScriptRuntime::Destroy()
{
	// seems like creating and destroying an isolate in the same tick causes a crash in some cases
	uv_run(m_uvLoop, UV_RUN_NOWAIT);

	m_eventRoutine = TEventRoutine();
	m_tickRoutine = std::function<void()>();
	m_callRefRoutine = TCallRefRoutine();
	m_deleteRefRoutine = TDeleteRefRoutine();
	m_duplicateRefRoutine = TDuplicateRefRoutine();

	SharedPushEnvironment pushed(this);

	node::EmitProcessBeforeExit(m_nodeEnvironment);
	node::EmitProcessExit(m_nodeEnvironment);
	node::Stop(m_nodeEnvironment);
	node::FreeEnvironment(m_nodeEnvironment);
	node::FreeIsolateData(m_isolateData);

	// uv_loop_close(m_uvLoop);
	// delete m_uvLoop;

	m_context.Reset();
	return FX_S_OK;
}

result_t NodeScriptRuntime::Tick()
{
	if (!m_tickRoutine)
	{
		return FX_S_OK;
	}

	if (!v8::Locker::IsLocked(GetIsolate()))
	{
		SharedPushEnvironment pushed(this);
		m_tickRoutine();
	}
	else
	{
		SharedPushEnvironmentNoIsolate pushed(this);
		m_tickRoutine();
	}
	return FX_S_OK;
}

result_t NodeScriptRuntime::TriggerEvent(char* eventName, char* eventPayload, uint32_t payloadSize, char* eventSource)
{
	if (m_eventRoutine)
	{
		SharedPushEnvironment pushed(this);
		m_eventRoutine(eventName, eventPayload, payloadSize, eventSource);
	}
	return FX_S_OK;
}

void* NodeScriptRuntime::GetParentObject()
{
	return m_parentObject;
}

void NodeScriptRuntime::SetParentObject(void* ptr)
{
	m_parentObject = reinterpret_cast<fx::Resource*>(ptr);
}

int NodeScriptRuntime::GetInstanceId()
{
	return m_instanceId;
}

int NodeScriptRuntime::HandlesFile(char* filename, IScriptHostWithResourceData* metadata)
{
	if (strstr(filename, ".js"))
	{
		return true;
	}
	return false;
}

result_t NodeScriptRuntime::LoadFileInternal(OMPtr<fxIStream> stream, char* scriptFile, v8::Local<v8::Script>* outScript)
{
	// read file data
	uint64_t length;
	result_t hr;

	if (FX_FAILED(hr = stream->GetLength(&length)))
	{
		return hr;
	}

	std::vector<char> fileData(length + 1);
	if (FX_FAILED(hr = stream->Read(&fileData[0], length, nullptr)))
	{
		return hr;
	}

	fileData[length] = '\0';

	// create the script data
	v8::Local<v8::String> scriptText = v8::String::NewFromUtf8(m_isolate, &fileData[0], v8::NewStringType::kNormal).ToLocalChecked();
	v8::Local<v8::String> fileName = v8::String::NewFromUtf8(m_isolate, scriptFile, v8::NewStringType::kNormal).ToLocalChecked();

	// compile the script in a TryCatch
	{
		v8::TryCatch eh(m_isolate);
		v8::ScriptOrigin origin(m_isolate, fileName);
		v8::MaybeLocal<v8::Script> script = v8::Script::Compile(GetContext(), scriptText, &origin);

		if (script.IsEmpty())
		{
			v8::String::Utf8Value str(m_isolate, eh.Exception());

			ScriptTrace(this, "Error parsing script %s in resource %s: %s\n", scriptFile, GetResourceName(), *str);

			// TODO: change?
			return FX_E_INVALIDARG;
		}

		*outScript = script.ToLocalChecked();
	}
	return FX_S_OK;
}

result_t NodeScriptRuntime::LoadHostFileInternal(char* scriptFile, v8::Local<v8::Script>* outScript, bool isSystem)
{
	// open the file
	OMPtr<fxIStream> stream;

	result_t hr = isSystem ? m_scriptHost->OpenSystemFile(scriptFile, stream.GetAddressOf()) : m_scriptHost->OpenHostFile(scriptFile, stream.GetAddressOf());

	if (FX_FAILED(hr))
	{
		// TODO: log this?
		return hr;
	}

	char* resourceName;
	m_resourceHost->GetResourceName(&resourceName);

	return LoadFileInternal(stream, (scriptFile[0] != '@') ? const_cast<char*>(fmt::sprintf("@%s/%s", resourceName, scriptFile).c_str()) : scriptFile, outScript);
}

result_t NodeScriptRuntime::RunFileInternal(char* scriptName, std::function<result_t(char*, v8::Local<v8::Script>*)> loadFunction)
{
	SharedPushEnvironment pushed(this);

	result_t hr;
	v8::Local<v8::Script> script;

	if (FX_FAILED(hr = loadFunction(scriptName, &script)))
	{
		return hr;
	}

	{
		v8::TryCatch eh(m_isolate);
		v8::MaybeLocal<v8::Value> value = script->Run(m_context.Get(m_isolate));

		if (value.IsEmpty())
		{
			v8::String::Utf8Value str(m_isolate, eh.Exception());
			v8::String::Utf8Value stack(m_isolate, GetStackTrace(eh, this));

			ScriptTrace(this, "Error loading script %s in resource %s: %s\nstack:\n%s\n", scriptName, GetResourceName(), *str, *stack);

			// TODO: change?
			return FX_E_INVALIDARG;
		}
	}
	return FX_S_OK;
}

result_t NodeScriptRuntime::LoadFile(char* scriptName)
{
	return RunFileInternal(scriptName, std::bind(&NodeScriptRuntime::LoadHostFileInternal, this, std::placeholders::_1, std::placeholders::_2, false));
}

result_t NodeScriptRuntime::LoadSystemFile(char* scriptName)
{
	return RunFileInternal(scriptName, std::bind(&NodeScriptRuntime::LoadHostFileInternal, this, std::placeholders::_1, std::placeholders::_2, true));
}

result_t NodeScriptRuntime::CallRef(int32_t refIdx, char* argsSerialized, uint32_t argsLength, IScriptBuffer** retval)
{
	*retval = nullptr;

	if (m_callRefRoutine)
	{
		SharedPushEnvironment pushed(this);

		auto rv = m_callRefRoutine(refIdx, argsSerialized, argsLength);
		return rv.CopyTo(retval);
	}
	return FX_S_OK;
}

result_t NodeScriptRuntime::DuplicateRef(int32_t refIdx, int32_t* outRefIdx)
{
	*outRefIdx = -1;

	if (m_duplicateRefRoutine)
	{
		SharedPushEnvironment pushed(this);

		*outRefIdx = m_duplicateRefRoutine(refIdx);
	}
	return FX_S_OK;
}

result_t NodeScriptRuntime::RemoveRef(int32_t refIdx)
{
	if (m_deleteRefRoutine)
	{
		SharedPushEnvironment pushed(this);

		m_deleteRefRoutine(refIdx);
	}
	return FX_S_OK;
}

result_t NodeScriptRuntime::WalkStack(char* boundaryStart, uint32_t boundaryStartLength, char* boundaryEnd, uint32_t boundaryEndLength, IScriptStackWalkVisitor* visitor)
{
	if (m_stackTraceRoutine)
	{
		SharedPushEnvironment pushed(this);

		char* out = nullptr;
		size_t outLen = 0;

		m_stackTraceRoutine(boundaryStart, boundaryEnd, &out, &outLen);

		if (out)
		{
			msgpack::unpacked up = msgpack::unpack(out, outLen);

			auto o = up.get().as<std::vector<msgpack::object>>();

			for (auto& e : o)
			{
				msgpack::sbuffer sb;
				msgpack::pack(sb, e);

				visitor->SubmitStackFrame(sb.data(), sb.size());
			}
		}
	}
	return FX_S_OK;
}

result_t NodeScriptRuntime::EmitWarning(char* channel, char* message)
{
	if (!m_context.IsEmpty())
	{
		SharedPushEnvironment pushed(this);

		auto context = GetContext();
		auto consoleMaybe = context->Global()->Get(context, v8::String::NewFromUtf8(m_isolate, "console").ToLocalChecked());
		v8::Local<v8::Value> console;

		if (consoleMaybe.ToLocal(&console))
		{
			auto consoleObject = console.As<v8::Object>();
			auto warnMaybe = consoleObject->Get(context, v8::String::NewFromUtf8(m_isolate, "warn").ToLocalChecked());

			v8::Local<v8::Value> warn;

			if (warnMaybe.ToLocal(&warn))
			{
				auto messageStr = fmt::sprintf("[%s] %s", channel, message);

				// console.warn() will append a newline itself, so we remove any redundant newline
				auto length = messageStr.length();
				if (messageStr[length - 1] == '\n')
				{
					--length;
				}

				v8::Local<v8::Value> args[] = {
					v8::String::NewFromUtf8(m_isolate, messageStr.c_str(), v8::NewStringType::kNormal, length).ToLocalChecked()
				};

				auto warnFunction = warn.As<v8::Function>();
				warnFunction->Call(context, v8::Null(m_isolate), std::size(args), args);
			}
		}
	}
	return FX_S_OK;
}

const char* NodeScriptRuntime::AssignStringValue(const v8::Local<v8::Value>& value, size_t* length)
{
	auto stringValue = std::make_unique<v8::String::Utf8Value>(GetIsolate(), value);
	const char* str = **(stringValue.get());
	*length = stringValue->length();

	// take ownership
	m_stringValues[m_curStringValue] = std::move(stringValue);

	// increment the string value
	m_curStringValue = (m_curStringValue + 1) % _countof(m_stringValues);

	// return the string
	return str;
}

// {91e0561f-ceea-4236-83a2-c18dee84f96e}
FX_DEFINE_GUID(CLSID_NodeScriptRuntime, 0x91e0561f, 0xceea, 0x4236, 0x83, 0xa2, 0xc1, 0x8d, 0xee, 0x84, 0xf9, 0x6e);
FX_NEW_FACTORY(NodeScriptRuntime);
FX_IMPLEMENTS(CLSID_NodeScriptRuntime, IScriptRuntime);
FX_IMPLEMENTS(CLSID_NodeScriptRuntime, IScriptFileHandlingRuntime);
}
