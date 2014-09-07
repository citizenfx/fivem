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

	const std::string& GetCategory(int idx) const;

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

class CORE_EXPORT Component : public fwRefCountable
{
public:
	virtual bool Initialize() = 0;

	virtual bool Shutdown() = 0;

	virtual bool DoGameLoad(HANDLE hModule);
};

class CORE_EXPORT ComponentData : public fwRefCountable
{
protected:
	virtual Component* CreateComponent() = 0;

public:
	virtual std::string GetName() = 0;

	virtual std::vector<ComponentId> GetProvides() = 0;

	virtual std::vector<ComponentId> GetDepends() = 0;

private:
	fwRefContainer<Component> m_component;

public:
	void Load();

	inline fwRefContainer<Component> GetComponent() { return m_component; }
};

class CORE_EXPORT ComponentLoader : public fwSingleton<ComponentLoader>
{
private:
	typedef std::unordered_map<std::string, fwRefContainer<ComponentData>> TComponentList;

	typedef std::set<std::string> TLoadedList;

	fwRefContainer<ComponentData> m_rootComponent;

	TComponentList m_knownComponents;

	TLoadedList m_loadedComponents;

private:
	void AddComponent(fwRefContainer<ComponentData> component);

public:
	void Initialize();

	void DoGameLoad(HANDLE hModule);

	fwRefContainer<ComponentData> LoadComponent(const char* component);
};