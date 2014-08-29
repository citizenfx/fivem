#include "StdInc.h"
#include "GameFlags.h"
#include "ResourceManager.h"
#include "ResourceUI.h"
#include <yaml-cpp/yaml.h>

#undef NDEBUG
#include <assert.h>

ResourceExport::ResourceExport(Resource* resource, std::string& functionName)
	: m_resource(resource), m_function(functionName)
{
	m_scriptFunction = -1;
}

Resource::Resource(std::string& name, std::string& path)
	: m_name(name), m_path(path), m_state(ResourceStateStopped)
{
	
}

void Resource::Tick()
{
	if (m_state == ResourceStateRunning)
	{
		m_scriptEnvironment->Tick();
	}
	else if (m_state == ResourceStateStopping)
	{
		m_scriptEnvironment = nullptr;

		m_state = ResourceStateStopped;

		std::string nameArgs = std::string(va("[\"%s\"]", m_name.c_str()));
		TheResources.TriggerEvent(std::string("resourceStopped"), nameArgs);

	}
}

void Resource::TriggerEvent(std::string& eventName, std::string& argsSerialized, int source)
{
	if (m_state == ResourceStateRunning)
	{
		m_scriptEnvironment->TriggerEvent(eventName, argsSerialized, source);
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

		if (!resource.get())
		{
			trace("Can't resolve dependency %s from resource %s.\n", dep.c_str(), m_name.c_str());
			return;
		}

		resource->Start();
		resource->AddDependant(m_name);
	}

	m_ui = std::make_shared<ResourceUI>(this);
	m_ui->Create();

	// go
	m_state = ResourceStateStarting;

	if (!EnsureScriptEnvironment())
	{
		m_state = ResourceStateError;

		return;
	}

	// initialize any 'later' parts of the initialization file
	m_scriptEnvironment->DoInitFile(false);

	m_scriptEnvironment->LoadScripts();

	m_state = ResourceStateRunning;

	if (GameFlags::GetFlag(GameFlag::PlayerActivated))
	{
		msgpack::sbuffer nameArgs;
		msgpack::packer<msgpack::sbuffer> packer(nameArgs);

		packer.pack_array(1);
		packer.pack(m_name);

		TheResources.TriggerEvent(std::string("onClientResourceStart"), nameArgs);
	}
}

bool Resource::EnsureScriptEnvironment()
{
	if (!m_scriptEnvironment.get())
	{
		m_scriptEnvironment = std::make_shared<ScriptEnvironment>(this);
		if (!m_scriptEnvironment->Create())
		{
			trace("Resource %s caused an error during loading. Please see the above lines for details.\n", m_name.c_str());

			m_state = ResourceStateError;
			return false;
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
			auto resource = TheResources.GetResource(std::string(dep));

			resource->Stop();
		}
	}

	m_dependants.clear();

	for (auto& dep : m_dependencies)
	{
		auto resource = TheResources.GetResource(dep);

		resource->RemoveDependant(m_name);
	}

	m_ui->Destroy();

	msgpack::sbuffer nameArgs;
	msgpack::packer<msgpack::sbuffer> packer(nameArgs);

	packer.pack_array(1);
	packer.pack(m_name);

	TheResources.TriggerEvent(std::string("onClientResourceStop"), nameArgs);

	m_scriptEnvironment->Destroy();
	m_scriptEnvironment = nullptr;

	assert(m_name != "mapmanager");

	m_state = ResourceStateStopping;
}

void Resource::AddPackFiles(std::vector<rage::fiPackfile*>& packFiles)
{
	for (auto& packFile : packFiles)
	{
		m_packFiles.push_back(packFile);
	}
}

void Resource::AddDependant(std::string& dependant)
{
	m_dependants.insert(dependant);
}

void Resource::RemoveDependant(std::string& dependant)
{
	m_dependants.erase(dependant);
}

std::string Resource::CallExport(std::string& exportName, std::string& argsSerialized)
{
	// if not running, return
	if (m_state != ResourceStateRunning)
	{
		return std::string("[null]");
	}

	// find the export
	for (auto& exp : m_exports)
	{
		if (exp.GetName() == exportName)
		{
			return m_scriptEnvironment->CallExport(exp.GetScriptFunction(), argsSerialized);
		}
	}

	return std::string("[null]");
}

int Resource::DuplicateRef(int luaRef)
{
	return m_scriptEnvironment->DuplicateRef(luaRef);
}

bool Resource::HasRef(int luaRef, uint32_t instance)
{
	return m_scriptEnvironment.get() && m_scriptEnvironment->GetInstanceId() == instance;
}

std::string Resource::CallRef(int luaRef, std::string& argsSerialized)
{
	return m_scriptEnvironment->CallExport(luaRef, argsSerialized);
}

void Resource::RemoveRef(int luaRef)
{
	if (m_scriptEnvironment.get())
	{
		m_scriptEnvironment->RemoveRef(luaRef);
	}
}

bool Resource::HasExport(std::string& exportName)
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

void Resource::AddExport(std::string exportName)
{
	m_exports.push_back(ResourceExport(this, exportName));
}

void Resource::AddDependency(std::string dependency)
{
	m_dependencies.push_back(dependency);
}

void Resource::SetMetaData(std::string key, std::string value)
{
	m_metaData.insert(std::make_pair(key, value));
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
	return m_scriptEnvironment->DoInitFile(true);
}