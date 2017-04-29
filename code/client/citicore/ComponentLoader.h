/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "Singleton.h"
#include "EventCore.h"
#include <unordered_map>

//
// An identifier for a Fx component. These are formatted like `category:subcategory:...[version]`.
// 
// Cross-DLL: no (passing is only by strings)
//
class CORE_EXPORT ComponentId
{
private:
	std::vector<std::string> m_categories;
	int m_versions[4];

public:
	ComponentId();

	static ComponentId Parse(const char* string);

	const std::string& GetCategory(size_t idx) const;

	inline const std::string& GetCategory() const
	{
		return GetCategory(0);
	}

	inline const std::string& GetSubCategory() const
	{
		return GetCategory(1);
	}

	inline const int* GetVersions() const
	{
		return m_versions;
	}

	std::string GetString() const;

	bool IsMatchedBy(const ComponentId& provider) const;

	int CompareVersion(const ComponentId& secondId) const;
};

class Component : public fwRefCountable
{
public:
	virtual inline bool Initialize()
	{
		return true;
	}

	virtual inline bool SetUserData(const std::string& userData)
	{
		(void)userData;

		return true;
	}

	virtual bool Shutdown() = 0;

    virtual inline bool DoGameLoad(void* hModule)
    {
        return true;
    }
};

class RunnableComponent : public Component
{
public:
	virtual void Run() = 0;
};

class CORE_EXPORT ComponentData : public fwRefCountable
{
protected:
	virtual Component* CreateComponent() = 0;

public:
	virtual std::string GetName() = 0;

	virtual std::vector<ComponentId> GetProvides() = 0;

	virtual std::vector<ComponentId> GetDepends() = 0;

	virtual bool ShouldAutoInstance() = 0;

private:
	std::vector<fwRefContainer<ComponentData>> m_dependencyComponents;

public:
	inline void AddDependency(const fwRefContainer<ComponentData>& dependency)
	{
		m_dependencyComponents.push_back(dependency);
	}

	inline const std::vector<fwRefContainer<ComponentData>>& GetDependencyDataList()
	{
		return m_dependencyComponents;
	}

private:
	std::vector<fwRefContainer<Component>> m_instances;

	bool m_isLoaded;

public:
	void Load();

	inline bool IsLoaded()
	{
		return m_isLoaded;
	}

	inline void SetLoaded(bool value)
	{
		m_isLoaded = value;
	}

	fwRefContainer<Component> CreateInstance(const std::string& userData);

	fwRefContainer<Component> CreateManualInstance();

	inline const std::vector<fwRefContainer<Component>>& GetInstances()
	{
		return m_instances;
	}
};

class CORE_EXPORT ComponentLoader : public fwSingleton<ComponentLoader>
{
private:
	typedef std::unordered_map<std::string, fwRefContainer<ComponentData>> TComponentList;

	typedef std::vector<fwRefContainer<ComponentData>> TLoadedList;

	fwRefContainer<ComponentData> m_rootComponent;

	TComponentList m_knownComponents;

	TLoadedList m_loadedComponents;

private:
	void AddComponent(fwRefContainer<ComponentData> component);

public:
	void Initialize();

	void DoGameLoad(void* hModule);

	void ForAllComponents(const std::function<void(fwRefContainer<ComponentData>)>& callback);

	fwRefContainer<ComponentData> LoadComponent(const char* component);

	inline auto& GetKnownComponents()
	{
		return m_knownComponents;
	}
};

#include <queue>

template<typename T>
struct GetDependencies
{
	
};

template<>
struct GetDependencies<fwRefContainer<ComponentData>>
{
	const std::vector<fwRefContainer<ComponentData>>& operator()(const fwRefContainer<ComponentData>& componentData)
	{
		return componentData->GetDependencyDataList();
	}
};

template<typename TListEntry, typename TList, typename TGetDependencies = GetDependencies<TListEntry>>
std::queue<TListEntry> SortDependencyList(const TList& list)
{
	std::queue<TListEntry> stack;

	std::map<TListEntry, bool> visited;

	std::function<void(const TListEntry&)> visit = [&] (const TListEntry& entry)
	{
		visited[entry] = true;

		for (auto& dependency : TGetDependencies()(entry))
		{
			if (!visited[dependency])
			{
				visit(dependency);
			}
		}

		stack.push(entry);
	};

	for (auto& entry : list)
	{
		if (!visited[entry])
		{
			visit(entry);
		}
	}

	return stack;
}

template<typename TListEntry>
std::queue<TListEntry> SortDependencyList(const std::vector<TListEntry>& list)
{
	return SortDependencyList<TListEntry, std::vector<TListEntry>>(list);
}

template<typename T, typename TOther>
inline T dynamic_component_cast(TOther value)
{
	try
	{
		return dynamic_cast<T>(value);
	}
	catch (std::bad_typeid)
	{
		return T();
	}
}