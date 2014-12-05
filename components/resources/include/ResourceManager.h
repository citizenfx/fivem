#pragma once

#include <stack>

#include "fiDevice.h"

using rage::fiDevice;

#include <msgpack.hpp>
#include "msgpack_fwstring.h"
#include "ResourceCache.h"
//#include "ResourceScripting.h"
#include "BaseResourceScripting.h"

class ResourceUI;

#include <memory>
#include <set>

typedef fwMap<fwString, fwString> ResourceMetaData;

class Resource;

class
#ifdef COMPILING_RESOURCES
	__declspec(dllexport)
#endif
	ResourceExport
{
private:
	fwString m_function;

	Resource* m_resource;

	fwRefContainer<BaseScriptEnvironment> m_scriptEnvironment;

	ScriptFunctionRef m_scriptFunction;
public:
	ResourceExport(Resource* resource, fwRefContainer<BaseScriptEnvironment> scriptEnvironment, fwString& functionName);

	inline fwString GetName() { return m_function; }

	inline Resource* GetResource() { return m_resource; }

	inline fwRefContainer<BaseScriptEnvironment> GetScriptEnvironment() { return m_scriptEnvironment; }

	inline ScriptFunctionRef GetScriptFunction() { return m_scriptFunction; }

	inline void SetScriptFunction(ScriptFunctionRef ref) { m_scriptFunction = ref; }
};

enum ResourceState
{
	ResourceStateStopped,
	ResourceStateStopping,
	ResourceStateStarting,
	ResourceStateRunning,
	ResourceStateParsing,
	ResourceStateError
};

class
#ifdef COMPILING_RESOURCES
	__declspec(dllexport)
#endif
	ResourceIdentity
{
private:
	fwString m_name;

public:
	ResourceIdentity(fwString name);

	inline fwString GetName() const { return m_name; }

	inline bool IsAnonymous() const
	{
		return m_name.empty();
	}

	static inline ResourceIdentity GetAnonymous() { return ResourceIdentity(""); }
};

class
#ifdef COMPILING_RESOURCES
	__declspec(dllexport)
#else
	__declspec(dllimport)
#endif
	Resource : public fwRefCountable
{
private:
	fwString m_name;
	fwString m_path;

	ResourceState m_state;

	ResourceMetaData m_metaData;

	fwVector<fwString> m_scripts;

	fwVector<rage::fiPackfile*> m_packFiles;

	fwVector<ResourceExport> m_exports;

	fwList<fwRefContainer<BaseScriptEnvironment>> m_scriptEnvironments;

	//std::shared_ptr<ResourceUI> m_ui;

	std::vector<fwString> m_dependencies;

	std::set<fwString> m_dependants;

public:
	Resource(fwString& name, fwString& path);

	~Resource();

	bool Parse();

	void Start();

	void Stop();

	bool EnsureScriptEnvironment();

	void AddDependant(fwString& dependant);

	void RemoveDependant(fwString& dependant);

	fwString CallExport(fwString& exportName, fwString& argsSerialized);

	bool HasExport(fwString& exportName);

	bool HasRef(int luaRef, uint32_t instance);

	fwString CallRef(int luaRef, uint32_t instance, fwString& argsSerialized);

	void RemoveRef(int luaRef, uint32_t instance);

	int DuplicateRef(int luaRef, uint32_t instance);

	void TriggerEvent(fwString& eventName, fwString& argsSerialized, int source);

	void Tick();

	void AddDependency(fwString dependency);

	void AddExport(fwString exportName, fwRefContainer<BaseScriptEnvironment> scriptEnvironment);

	void AddPackFiles(fwVector<rage::fiPackfile*>& packFiles);

	void SetMetaData(fwString key, fwString value);

	void AddScriptEnvironment(fwRefContainer<BaseScriptEnvironment> scriptEnvironment);

private:
	bool ParseInfoFile();

public:
	static fwEvent<fwRefContainer<Resource>> OnCreateScriptEnvironments;

	static fwEvent<fwRefContainer<Resource>> OnStartingResource;

	static fwEvent<fwRefContainer<Resource>> OnStartedResource;

	static fwEvent<fwRefContainer<Resource>> OnStoppingResource;

	static fwEvent<fwRefContainer<Resource>, fwString, fwString> OnSetMetaData;

public:
	inline fwString GetName() { return m_name; }

	inline fwString GetPath() { return m_path; }

	inline ResourceState GetState() { return m_state; }

	//inline std::shared_ptr<ResourceUI> GetUI() { return m_ui; }

	inline ResourceMetaData& GetMetaData() { return m_metaData; }

	inline fwVector<ResourceExport>& GetExports() { return m_exports; }

	inline fwVector<fwString>& GetScripts() { return m_scripts; }

	inline fwVector<rage::fiPackfile*>& GetPackFiles() { return m_packFiles; }

	inline fwList<fwRefContainer<BaseScriptEnvironment>>& GetScriptEnvironments() { return m_scriptEnvironments; }
};

struct QueuedScriptEvent
{
	fwString eventName;
	fwString argsSerialized;
	int source;
};

class
#ifdef COMPILING_RESOURCES
	__declspec(dllexport)
#else
	__declspec(dllimport)
#endif
	ResourceManager
{
private:
	fwHashMap<fwString, fwRefContainer<Resource>> m_resources;

	std::vector<QueuedScriptEvent> m_eventQueue;

	int m_stateNumber;

	ResourceCache* m_resourceCache;

	std::stack<bool> m_eventCancelationState;

	bool m_eventCanceled;

	bool m_isResetting;

public:
	void ScanResources(fwVector<std::pair<fiDevice*, fwString>>& paths);

	void ScanResources(fiDevice* device, fwString& path);

	fwRefContainer<Resource> AddResource(fwString name, fwString path);

	void DeleteResource(fwRefContainer<Resource> resource);

	void Tick();

	void Reset();

	bool TriggerEvent(fwString& eventName, fwString& argsSerialized, int source = -1);

	bool TriggerEvent(fwString& eventName, msgpack::sbuffer& argsSerialized, int source = -1);

	void CancelEvent();

	bool WasEventCanceled();

	void CleanUp();
	
	void QueueEvent(fwString& eventName, fwString& argsSerialized, uint64_t source);

	void ForAllResources(fwAction<fwRefContainer<Resource>> cb);

	ResourceCache* GetCache();

	fwRefContainer<Resource> GetResource(fwString& name);

	inline int GetStateNumber() { return m_stateNumber; }

	static fwEvent<fwString> OnTriggerEvent;

	static fwEvent<fwString> OnQueueResourceStart;

public:
	static fwEvent<> OnScriptReset;
};

extern 
#ifdef COMPILING_RESOURCES
	__declspec(dllexport)
#else
	__declspec(dllimport)
#endif
	ResourceManager TheResources;