/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "BaseResourceScripting.h"
#include "ResourceManager.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/threads.h>

class MonoScriptEnvironment : public BaseScriptEnvironment
{
private:
	MonoDomain* m_scriptDomain;

	uint32_t m_instanceId;

	MonoAssembly* m_scriptManagerAssembly;

	MonoImage* m_scriptManagerImage;

	MonoMethod* m_scriptLoadMethod;

	MonoMethod* m_scriptTickMethod;

	MonoMethod* m_scriptEventMethod;

	MonoMethod* m_scriptCallRefMethod;

	MonoMethod* m_scriptRemoveRefMethod;

	Resource* m_resource;

	std::shared_ptr<CMissionCleanup> m_missionCleanup;

private:
	void OutputExceptionDetails(MonoObject* exc);

public:
	MonoScriptEnvironment(Resource* resource);

	virtual ~MonoScriptEnvironment();

	virtual const char* GetEnvironmentName();

	virtual bool Create();

	virtual void Destroy();

	virtual void Tick();

	virtual void TriggerEvent(fwString& eventName, fwString& argsSerialized, int source);

	virtual bool DoInitFile(bool isPreParse);

	virtual bool LoadScripts();

	virtual fwString CallExport(ScriptFunctionRef ref, fwString& argsSerialized);

	virtual uint32_t GetInstanceId();

	virtual ScriptFunctionRef DuplicateRef(ScriptFunctionRef ref);

	virtual void RemoveRef(ScriptFunctionRef ref);

	virtual void Activate();

	virtual CMissionCleanup* GetMissionCleanup();

	inline MonoDomain* GetScriptDomain() { return m_scriptDomain; }

	virtual Resource* GetResource() { return m_resource; }
};

void MonoAddInternalCalls();