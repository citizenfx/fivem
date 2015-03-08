/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
//#include "GameFlags.h"
#include "ResourceManager.h"
//#include "ResourceUI.h"
//#include <yaml-cpp/yaml.h>

#undef NDEBUG
#include <assert.h>

ResourceExport::ResourceExport(Resource* resource, fwRefContainer<BaseScriptEnvironment> scriptEnvironment, fwString& functionName)
	: m_resource(resource), m_function(functionName), m_scriptEnvironment(scriptEnvironment)
{
	m_scriptFunction = -1;
}

Resource::Resource(fwString& name, fwString& path)
	: m_name(name), m_path(path), m_state(ResourceStateStopped)
{
	
}

Resource::~Resource()
{
	trace("Destroying resource %s\n", m_name.c_str());
}

void Resource::Tick()
{
	if (m_state == ResourceStateRunning)
	{
		for (auto& scriptEnvironment : m_scriptEnvironments)
		{
			scriptEnvironment->Tick();
		}
		//m_scriptEnvironment->Tick();
	}
	else if (m_state == ResourceStateStopping)
	{
		m_scriptEnvironments.clear();

		m_state = ResourceStateStopped;

		fwString nameArgs = fwString(va("[\"%s\"]", m_name.c_str()));
		TheResources.TriggerEvent(fwString("resourceStopped"), nameArgs);

	}
}

void Resource::TriggerEvent(fwString& eventName, fwString& argsSerialized, int source)
{
	if (m_state == ResourceStateRunning)
	{
		for (auto& scriptEnvironment : m_scriptEnvironments)
		{
			scriptEnvironment->TriggerEvent(eventName, argsSerialized, source);
		}
	}
}

void Resource::Start()
{
	if (m_state != ResourceStateStopped)
	{
		trace("Tried to start resource %s, but it wasn't in the stopped state.\n", m_name.c_str());
		return;
	}

	// resolve dependencies
	for (auto& dep : m_dependencies)
	{
		auto resource = TheResources.GetResource(dep);

		if (!resource.GetRef())
		{
			trace("Can't resolve dependency %s from resource %s.\n", dep.c_str(), m_name.c_str());
			return;
		}

		resource->Start();
		resource->AddDependant(m_name);
	}

	//m_ui = std::make_shared<ResourceUI>(this);
	//m_ui->Create();
	OnStartingResource(this);

	// go
	m_state = ResourceStateStarting;

	if (!EnsureScriptEnvironment())
	{
		m_state = ResourceStateError;

		return;
	}

	// initialize any 'later' parts of the initialization file
	for (auto& scriptEnvironment : m_scriptEnvironments)
	{
		scriptEnvironment->DoInitFile(false);
	}

	for (auto& scriptEnvironment : m_scriptEnvironments)
	{
		scriptEnvironment->LoadScripts();
	}

	m_state = ResourceStateRunning;

	OnStartedResource(this);

	/*if (GameFlags::GetFlag(GameFlag::PlayerActivated))
	{
		msgpack::sbuffer nameArgs;
		msgpack::packer<msgpack::sbuffer> packer(nameArgs);

		packer.pack_array(1);
		packer.pack(m_name);

		TheResources.TriggerEvent(std::string("onClientResourceStart"), nameArgs);
	}*/
}

bool Resource::EnsureScriptEnvironment()
{
	/*if (!m_scriptEnvironment.get())
	{
		m_scriptEnvironment = std::make_shared<ScriptEnvironment>(this);
		if (!m_scriptEnvironment->Create())
		{
			trace("Resource %s caused an error during loading. Please see the above lines for details.\n", m_name.c_str());

			m_state = ResourceStateError;
			return false;
		}

		ParseInfoFile();
	}*/

	if (m_scriptEnvironments.size() == 0)
	{
		OnCreateScriptEnvironments(this);

		for (auto& scriptEnvironment : m_scriptEnvironments)
		{
			if (!scriptEnvironment->Create())
			{
				trace("Resource %s caused an error during loading in script environment %s. Please see the above lines for details.\n", m_name.c_str(), scriptEnvironment->GetEnvironmentName());

				m_state = ResourceStateError;
				return false;
			}
		}

		ParseInfoFile();
	}

	return true;
}

void Resource::Stop()
{
	if (m_state != ResourceStateRunning)
	{
		trace("Tried to stop resource %s, but it wasn't in the running state.\n", m_name.c_str());
		return;
	}

	if (m_dependants.size())
	{
		// copy the list so we won't break iterators by doing RemoveDependant
		auto dependants = m_dependants;

		for (auto& dep : dependants)
		{
			auto resource = TheResources.GetResource(fwString(dep));

			resource->Stop();
		}
	}

	m_dependants.clear();

	for (auto& dep : m_dependencies)
	{
		auto resource = TheResources.GetResource(dep);

		resource->RemoveDependant(m_name);
	}

	//m_ui->Destroy();
	OnStoppingResource(this);

	msgpack::sbuffer nameArgs;
	msgpack::packer<msgpack::sbuffer> packer(nameArgs);

	packer.pack_array(1);
	packer.pack(m_name);

	TheResources.TriggerEvent(fwString("onClientResourceStop"), nameArgs);

	for (auto& scriptEnvironment : m_scriptEnvironments)
	{
		scriptEnvironment->Destroy();
	}

	m_scriptEnvironments.clear();

	//assert(m_name != "mapmanager");

	m_state = ResourceStateStopping;
}

void Resource::AddPackFiles(fwVector<rage::fiPackfile*>& packFiles)
{
	for (auto& packFile : packFiles)
	{
		m_packFiles.push_back(packFile);
	}
}

void Resource::AddDependant(fwString& dependant)
{
	m_dependants.insert(dependant);
}

void Resource::RemoveDependant(fwString& dependant)
{
	m_dependants.erase(dependant);
}

fwString Resource::CallExport(fwString& exportName, fwString& argsSerialized)
{
	// if not running, return
	if (m_state != ResourceStateRunning)
	{
		return fwString("[null]");
	}

	// find the export
	for (auto& exp : m_exports)
	{
		if (exp.GetName() == exportName)
		{
			return exp.GetScriptEnvironment()->CallExport(exp.GetScriptFunction(), argsSerialized);
			//return m_scriptEnvironment->CallExport(exp.GetScriptFunction(), argsSerialized);
		}
	}

	return fwString("[null]");
}

int Resource::DuplicateRef(int luaRef, uint32_t instance)
{
	for (auto& scriptEnvironment : m_scriptEnvironments)
	{
		if (scriptEnvironment->GetInstanceId() == instance)
		{
			return scriptEnvironment->DuplicateRef(luaRef);
		}
	}

	assert(!"invalid reference duplication attempt");

	return 0;
}

bool Resource::HasRef(int luaRef, uint32_t instance)
{
	for (auto& scriptEnvironment : m_scriptEnvironments)
	{
		if (scriptEnvironment->GetInstanceId() == instance)
		{
			return true;
		}
	}

	return false;
}

fwString Resource::CallRef(int luaRef, uint32_t instance, fwString& argsSerialized)
{
	for (auto& scriptEnvironment : m_scriptEnvironments)
	{
		if (scriptEnvironment->GetInstanceId() == instance)
		{
			return scriptEnvironment->CallExport(luaRef, argsSerialized);
		}
	}
	
	return fwString();
}

void Resource::RemoveRef(int luaRef, uint32_t instance)
{
	for (auto& scriptEnvironment : m_scriptEnvironments)
	{
		if (scriptEnvironment->GetInstanceId() == instance)
		{
			scriptEnvironment->RemoveRef(luaRef);
		}
	}
}

bool Resource::HasExport(fwString& exportName)
{
	// find the export
	for (auto& exp : m_exports)
	{
		if (exp.GetName() == exportName)
		{
			return true;
		}
	}

	return false;
}

void Resource::AddExport(fwString exportName, fwRefContainer<BaseScriptEnvironment> scriptEnvironment)
{
	m_exports.push_back(ResourceExport(this, scriptEnvironment, exportName));
}

void Resource::AddDependency(fwString dependency)
{
	m_dependencies.push_back(dependency);
}

void Resource::SetMetaData(fwString key, fwString value)
{
	if (m_metaData[key] != value)
	{
		OnSetMetaData(this, key, value);
	}

	m_metaData[key] = value;
}

bool Resource::Parse()
{
	if (!EnsureScriptEnvironment())
	{
		return false;
	}

	m_state = ResourceStateParsing;

	bool result = ParseInfoFile();

	m_state = ResourceStateStopped;

	return result;
}

bool Resource::ParseInfoFile()
{
	for (auto& scriptEnvironment : m_scriptEnvironments)
	{
		if (!scriptEnvironment->DoInitFile(true))
		{
			return false;
		}
	}

	return true;
	//return m_scriptEnvironment->DoInitFile(true);
}

void Resource::AddScriptEnvironment(fwRefContainer<BaseScriptEnvironment> scriptEnvironment)
{
	m_scriptEnvironments.push_back(scriptEnvironment);
}

__declspec(dllexport) fwEvent<fwRefContainer<Resource>>	Resource::OnCreateScriptEnvironments;
__declspec(dllexport) fwEvent<fwRefContainer<Resource>> Resource::OnStartingResource;
__declspec(dllexport) fwEvent<fwRefContainer<Resource>> Resource::OnStartedResource;
__declspec(dllexport) fwEvent<fwRefContainer<Resource>> Resource::OnStoppingResource;
__declspec(dllexport) fwEvent<fwRefContainer<Resource>, fwString, fwString> Resource::OnSetMetaData;
__declspec(dllexport) fwEvent<const fwString&, const fwString&, int> ResourceManager::OnTriggerEvent;