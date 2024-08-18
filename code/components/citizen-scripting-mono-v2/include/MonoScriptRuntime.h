/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "StdInc.h"

#include <deque>

#include <fxScripting.h>
#include <Resource.h>
#include <ManifestVersion.h>

#include <om/OMComponent.h>

#include "MonoMethods.h"
#include "ScriptSharedData.h"

namespace fx::mono
{
class MonoScriptRuntime : public fx::OMClass<MonoScriptRuntime, IScriptRuntime, IScriptFileHandlingRuntime, IScriptTickRuntime,
	IScriptEventRuntime, IScriptRefRuntime, IScriptMemInfoRuntime, /*IScriptStackWalkingRuntime,*/ IScriptDebugRuntime, IScriptProfiler>
{
private:
	int m_instanceId;
	std::string m_name;
	std::string m_resourceName;
	MonoDomain* m_appDomain;
	int32_t m_appDomainId;

	// Direct host access
	IScriptHost* m_scriptHost;
	IScriptHostWithResourceData* m_resourceHost;
	IScriptHostWithManifest* m_manifestHost;

	fx::OMPtr<IScriptRuntimeHandler> m_handler;
	fx::Resource* m_parentObject;
	IDebugEventListener* m_debugListener;
	std::unordered_map<std::string, int> m_scriptIds;

	ScriptSharedData m_sharedData;

	// method targets
	Method m_loadAssembly;

	// method thunks, these are for calls that require performance
	Thunk<void(uint64_t gameTime, bool profiling)> m_tick;
	Thunk<void(MonoString* eventName, const char* argsSerialized, uint32_t serializedSize, MonoString* sourceId, uint64_t gameTime, bool profiling)> m_triggerEvent;

	Thunk<void(int32_t refIndex, char* argsSerialized, uint32_t argsSize, MonoArray** retval, uint64_t gameTime, bool profiling)> m_callRef;
	Thunk<void(int32_t refIndex, int32_t* newRefIdx)> m_duplicateRef = nullptr;
	Thunk<void(int32_t refIndex)> m_removeRef = nullptr;

	Thunk<int64_t()> m_getMemoryUsage = nullptr;
	Thunk<void()> m_startProfiling = nullptr;
	Thunk<void()> m_stopProfiling = nullptr;

public:
	MonoScriptRuntime();

	virtual ~MonoScriptRuntime() override = default;

	MonoArray* CanonicalizeRef(int referenceId) const;

	MonoArray* InvokeFunctionReference(MonoString* referenceId, MonoArray* argsSerialized) const;

	bool ReadAssembly(MonoString* name, MonoArray** assembly, MonoArray** symbols) const;

private:
	void InitializeMethods(MonoImage* monoImage);

public:
	NS_DECL_ISCRIPTRUNTIME;

	NS_DECL_ISCRIPTFILEHANDLINGRUNTIME;

	NS_DECL_ISCRIPTTICKRUNTIME;

	NS_DECL_ISCRIPTEVENTRUNTIME;

	NS_DECL_ISCRIPTREFRUNTIME;

	NS_DECL_ISCRIPTMEMINFORUNTIME;

	// NS_DECL_ISCRIPTSTACKWALKINGRUNTIME;

	NS_DECL_ISCRIPTDEBUGRUNTIME;

	NS_DECL_ISCRIPTPROFILER;
};

inline MonoScriptRuntime::MonoScriptRuntime()
{
	m_instanceId = rand();
	m_name = "ScriptDomain_" + std::to_string(m_instanceId);
}
}
