/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ComponentLoader.h"
#include "DllGameComponent.h"
#include "FxGameComponent.h"
#include <Error.h>

#ifdef _WIN32
#define PLATFORM_LIBRARY_STRING L"%s.dll"
#else
#define PLATFORM_LIBRARY_STRING "lib%s.so"
#endif

static bool g_initialized;

static void LoadDependencies(ComponentLoader* loader, fwRefContainer<ComponentData>& component);

void ComponentLoader::Initialize()
{
    if (g_initialized)
    {
        return;
    }

    g_initialized = true;

	// run local initialization functions
	InitFunctionBase::RunAll();

	// set up the root component
	m_rootComponent = FxGameComponent::Create();
	AddComponent(m_rootComponent);

	// parse and load additional components
	FILE* componentCache = _pfopen(MakeRelativeCitPath(L"components.json").c_str(), _P("rb"));

	if (!componentCache)
	{
		FatalError("Could not find component cache storage file (components.json).");
	}

	// read component cache file
	fseek(componentCache, 0, SEEK_END);
	int length = ftell(componentCache);

	fseek(componentCache, 0, SEEK_SET);

	char* cacheBuf = new char[length + 1];
	fread(cacheBuf, 1, length, componentCache);
	cacheBuf[length] = '\0';

	fclose(componentCache);

	// parse the list
	rapidjson::Document doc;
	doc.Parse(cacheBuf);

	delete[] cacheBuf;

	if (doc.HasParseError())
	{
		FatalError("Error parsing components.json: %d", doc.GetParseError());
	}

	// look through the list for components to load
	std::vector<std::string> components;
	for (auto it = doc.Begin(); it != doc.End(); it++)
	{
		const char* name = it->GetString();

		components.push_back(name);

		// replace colons with dashes
		char* nameStr = strdup(name);
		char* p = nameStr;

		while (*p)
		{
			if (*p == ':')
			{
				*p = '-';
			}

			p++;
		}

		fwPlatformString nameWide(nameStr);

		free(nameStr);

		AddComponent(new DllGameComponent(va(PLATFORM_LIBRARY_STRING, nameWide.c_str())));
	}

	for (auto& component : m_knownComponents)
	{
		LoadDependencies(this, component.second);
	}

	// sort the list by dependency
	std::vector<fwRefContainer<ComponentData>> componentDatas;

	// get a set of values from the map
	std::transform(m_knownComponents.begin(), m_knownComponents.end(),
		std::back_inserter(componentDatas), [](const auto& a)
	{
		return std::get<1>(a);
	});

	std::queue<fwRefContainer<ComponentData>> sortedList = SortDependencyList(componentDatas);

	// clear the loaded list (it'll be added afterwards in order)
	m_loadedComponents.clear();

	while (!sortedList.empty())
	{
		auto comp = sortedList.front();
		sortedList.pop();

		comp = LoadComponent(comp->GetName().c_str());

		if (!comp.GetRef())
		{
			FatalError("Could not load component %s.", comp->GetName().c_str());
		}

		m_loadedComponents.push_back(comp);

		// create a component instance if need be 
		if (comp->ShouldAutoInstance())
		{
			comp->CreateInstance(std::string());
		}
	}
}

static void LoadDependencies(ComponentLoader* loader, fwRefContainer<ComponentData>& component)
{
	// match and resolve dependencies
	auto dependencies = component->GetDepends();

	for (auto& dependency : dependencies)
	{
		// find the first component to provide this
		bool match = false;

		for (auto& it : loader->GetKnownComponents())
		{
			auto matchProvides = it.second->GetProvides();

			for (auto& provide : matchProvides)
			{
				if (dependency.IsMatchedBy(provide))
				{
					component->AddDependency(it.second);

					match = true;

					break;
				}
			}

			// break if matched
			if (match)
			{
				break;
			}
		}

		if (!match && dependency.GetCategory() != "vendor")
		{
			FatalError("Unable to resolve dependency for %s.\n", dependency.GetString().c_str());
		}
	}
}

void ComponentLoader::DoGameLoad(void* hModule)
{
	for (auto& component : m_loadedComponents)
	{
		auto& instances = component->GetInstances();

		if (instances.size() > 0)
		{
			instances[0]->DoGameLoad(hModule);
		}
	}
}

void ComponentLoader::AddComponent(fwRefContainer<ComponentData> component)
{
	std::string name = component->GetName();

	component->SetLoaded(false);

	m_knownComponents.insert(std::make_pair(name, component));
}

void ComponentLoader::ForAllComponents(const std::function<void(fwRefContainer<ComponentData>)>& callback)
{
	for (auto& component : m_loadedComponents)
	{
		callback(component);
	}
}

fwRefContainer<ComponentData> ComponentLoader::LoadComponent(const char* componentName)
{
	auto component = m_knownComponents[componentName];

	if (!component.GetRef())
	{
		FatalError("Unknown component %s.", componentName);
	}

	if (component->IsLoaded())
	{
		return component;
	}

	// load the component
	component->Load();

	return component;
}

fwRefContainer<Component> ComponentData::CreateInstance(const std::string& userData)
{
	auto instance = CreateManualInstance();

	if (instance.GetRef() && !instance->SetUserData(userData))
	{
		instance = nullptr;
	}

	return instance;
}

fwRefContainer<Component> ComponentData::CreateManualInstance()
{
	fwRefContainer<Component> instance = CreateComponent();
	m_instances.push_back(instance);

	return instance;
}

void ComponentData::Load()
{
	SetLoaded(true);
}