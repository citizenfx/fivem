#pragma once

#include "fiDevice.h"

using rage::fiDevice;

#include "ResourceCache.h"
#include "ResourceScripting.h"

class ResourceUI;

#include <memory>
#include <set>

typedef std::map<std::string, std::string> ResourceMetaData;

class Resource;

class ResourceExport
{
private:
	std::string m_function;

	Resource* m_resource;

	ScriptFunctionRef m_scriptFunction;
public:
	ResourceExport(Resource* resource, std::string& functionName);

	inline std::string GetName() { return m_function; }

	inline Resource* GetResource() { return m_resource; }

	inline ScriptFunctionRef GetScriptFunction() { return m_scriptFunction; }

	inline void SetScriptFunction(ScriptFunctionRef ref) { m_scriptFunction = ref; }
};

enum ResourceState
{
	ResourceStateStopped,
	ResourceStateStopping,
	ResourceStateRunning,
	ResourceStateError
};

class Resource
{
private:
	std::string m_name;
	std::string m_path;

	ResourceState m_state;

	ResourceMetaData m_metaData;

	std::vector<std::string> m_scripts;

	std::vector<ResourceExport> m_exports;

	std::shared_ptr<ScriptEnvironment> m_scriptEnvironment;

	std::shared_ptr<ResourceUI> m_ui;

	std::vector<std::string> m_dependencies;

	std::set<std::string> m_dependants;

public:
	Resource(std::string& name, std::string& path);

	bool Parse();

	void Start();

	void Stop();

	void AddDependant(std::string& dependant);

	void RemoveDependant(std::string& dependant);

	std::string CallExport(std::string& exportName, std::string& argsSerialized);

	bool HasExport(std::string& exportName);

	bool HasRef(int luaRef);

	std::string CallRef(int luaRef, std::string& argsSerialized);

	void RemoveRef(int luaRef);

	int DuplicateRef(int luaRef);

	void TriggerEvent(std::string& eventName, std::string& argsSerialized, int source);

	void Tick();

public:
	inline std::string GetName() { return m_name; }

	inline std::string GetPath() { return m_path; }

	inline ResourceState GetState() { return m_state; }

	inline std::shared_ptr<ResourceUI> GetUI() { return m_ui; }

	inline ResourceMetaData& GetMetaData() { return m_metaData; }

	inline std::vector<ResourceExport>& GetExports() { return m_exports; }

	inline std::vector<std::string>& GetScripts() { return m_scripts; }
};

struct QueuedScriptEvent
{
	std::string eventName;
	std::string argsSerialized;
	int source;
};

class ResourceManager
{
private:
	std::unordered_map<std::string, std::shared_ptr<Resource>> m_resources;

	std::vector<QueuedScriptEvent> m_eventQueue;

	int m_stateNumber;

	ResourceCache* m_resourceCache;

public:
	void ScanResources(std::vector<std::pair<fiDevice*, std::string>>& paths);

	void ScanResources(fiDevice* device, std::string& path);

	std::shared_ptr<Resource> AddResource(std::string name, std::string path);

	void Tick();

	void Reset();

	void TriggerEvent(std::string& eventName, std::string& argsSerialized, int source = -1);
	
	void QueueEvent(std::string& eventName, std::string& argsSerialized, uint64_t source);

	ResourceCache* GetCache();

	std::shared_ptr<Resource> GetResource(std::string& name);

	inline int GetStateNumber() { return m_stateNumber; }
};

extern ResourceManager TheResources;