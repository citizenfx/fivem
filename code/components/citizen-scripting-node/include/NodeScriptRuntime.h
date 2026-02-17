/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "StdInc.h"

#include <deque>
#include <atomic>

#include <fxScripting.h>
#include <Resource.h>
#include <ManifestVersion.h>

#include <om/OMComponent.h>

#include <v8config.h>
#include <v8.h>
#include <v8-profiler.h>
#include <node.h>
#include <uv.h>

#include "shared/RuntimeHelpers.h"
#include "fxScriptBuffer.h"

#include <ScriptInvoker.h>
using namespace fx::invoker;

namespace node::permission
{
enum class PermissionScope;
};

namespace fx::nodejs
{
class NodeScriptRuntime : public OMClass<NodeScriptRuntime, IScriptRuntime, IScriptFileHandlingRuntime, IScriptTickRuntime, IScriptEventRuntime,
	IScriptRefRuntime, IScriptStackWalkingRuntime, IScriptWarningRuntime>
{
private:
	typedef std::function<void(const char*, const char*, size_t, const char*)> TEventRoutine;
	typedef std::function<fx::OMPtr<IScriptBuffer>(int32_t, const char*, size_t)> TCallRefRoutine;
	typedef std::function<int32_t(int32_t)> TDuplicateRefRoutine;
	typedef std::function<void(int32_t)> TDeleteRefRoutine;
	typedef std::function<void(void*, void*, char**, size_t*)> TStackTraceRoutine;
	typedef std::function<void(v8::PromiseRejectMessage&)> TUnhandledPromiseRejectionRoutine;
	typedef std::function<void()> TTickRoutine;

	int m_instanceId;
	std::string m_name;
	std::string m_resourceName;
	bool m_isMonitorRuntime = false;

	// direct host access
	IScriptHost* m_scriptHost = nullptr;
	IScriptHostWithResourceData* m_resourceHost = nullptr;
	IScriptHostWithManifest* m_manifestHost = nullptr;
	fx::Resource* m_parentObject = nullptr;
	fx::OMPtr<IScriptRuntimeHandler> m_handler;

	// v8 and nodejs related
	uv_loop_t* m_uvLoop = nullptr;
	v8::Isolate* m_isolate = nullptr;
	v8::UniquePersistent<v8::Context> m_context;
	std::unique_ptr<v8::MicrotaskQueue> m_taskQueue;
	node::IsolateData* m_isolateData = nullptr;
	node::Environment* m_nodeEnvironment = nullptr;

	std::atomic<int> m_isInGc{0};

	// string values, which need to be persisted across calls as well
	std::unique_ptr<v8::String::Utf8Value> m_stringValues[50];
	int m_curStringValue;

	// routines and callbacks
	TTickRoutine m_tickRoutine;
	TEventRoutine m_eventRoutine;
	TCallRefRoutine m_callRefRoutine;
	TDuplicateRefRoutine m_duplicateRefRoutine;
	TDeleteRefRoutine m_deleteRefRoutine;
	TStackTraceRoutine m_stackTraceRoutine;
	TUnhandledPromiseRejectionRoutine m_unhandledPromiseRejectionRoutine;

public:
	NodeScriptRuntime()
	{
		m_instanceId = rand() ^ 0x3e3;
		m_name = "ScriptDomain_" + std::to_string(m_instanceId);
		m_curStringValue = 0;
	}

	result_t LoadFileInternal(OMPtr<fxIStream> stream, char* scriptFile, v8::Local<v8::Script>* outScript);
	result_t LoadHostFileInternal(char* scriptFile, v8::Local<v8::Script>* outScript, bool isSystem = false);
	result_t RunFileInternal(char* scriptName, std::function<result_t(char*, v8::Local<v8::Script>*)> loadFunction);
	result_t LoadSystemFile(char* scriptFile);
	const char* AssignStringValue(const v8::Local<v8::Value>& value, size_t* length);
	bool NodePermissionCallback(node::Environment* env, node::permission::PermissionScope permission_, const std::string_view& resource);

	v8::Isolate* GetIsolate() const
	{
		return m_isolate;
	}

	v8::Local<v8::Context> GetContext() const
	{
		return m_context.Get(m_isolate);
	}

	OMPtr<IScriptHost> GetScriptHost() const
	{
		return m_scriptHost;
	}

	const char* GetResourceName() const
	{
		char* resourceName = nullptr;
		m_resourceHost->GetResourceName(&resourceName);

		return resourceName;
	}

	void SetTickRoutine(const TTickRoutine& tickRoutine)
	{
		if (!m_tickRoutine)
		{
			m_tickRoutine = tickRoutine;
		}
	}

	void SetEventRoutine(const TEventRoutine& eventRoutine)
	{
		if (!m_eventRoutine)
		{
			m_eventRoutine = eventRoutine;
		}
	}

	void SetCallRefRoutine(const TCallRefRoutine& routine)
	{
		if (!m_callRefRoutine)
		{
			m_callRefRoutine = routine;
		}
	}

	void SetDuplicateRefRoutine(const TDuplicateRefRoutine& routine)
	{
		if (!m_duplicateRefRoutine)
		{
			m_duplicateRefRoutine = routine;
		}
	}

	void SetDeleteRefRoutine(const TDeleteRefRoutine& routine)
	{
		if (!m_deleteRefRoutine)
		{
			m_deleteRefRoutine = routine;
		}
	}

	void SetStackTraceRoutine(const TStackTraceRoutine& routine)
	{
		if (!m_stackTraceRoutine)
		{
			m_stackTraceRoutine = routine;
		}
	}

	void SetUnhandledPromiseRejectionRoutine(const TUnhandledPromiseRejectionRoutine& routine)
	{
		if (!m_unhandledPromiseRejectionRoutine)
		{
			m_unhandledPromiseRejectionRoutine = routine;
		}
	}

	void RunMicrotasks()
	{
		if (m_isInGc.load(std::memory_order_acquire) == 0 && m_isolate && m_taskQueue)
		{
			m_taskQueue->PerformCheckpoint(m_isolate);
		}
	}

	bool IsInGc() const
	{
		return m_isInGc.load(std::memory_order_acquire) != 0;
	}

public:
	NS_DECL_ISCRIPTRUNTIME;
	NS_DECL_ISCRIPTFILEHANDLINGRUNTIME;
	NS_DECL_ISCRIPTTICKRUNTIME;
	NS_DECL_ISCRIPTEVENTRUNTIME;
	NS_DECL_ISCRIPTREFRUNTIME;
	NS_DECL_ISCRIPTSTACKWALKINGRUNTIME;
	NS_DECL_ISCRIPTWARNINGRUNTIME;
};
}
