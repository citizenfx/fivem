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

#ifdef _WIN32
#define PLATFORM_LIBRARY_STRING L"%s.dll"
#else
#define PLATFORM_LIBRARY_STRING "lib%s.so"
#endif

void ComponentLoader::Initialize()
{
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

	for (auto& component : components)
	{
		auto comp = LoadComponent(component.c_str());

		if (!comp.GetRef())
		{
			FatalError("Could not load component %s.", component.c_str());
		}

		trace("Initializing %s.\n", comp->GetName().c_str());

		comp->GetComponent()->Initialize();
	}
}

void ComponentLoader::DoGameLoad(void* hModule)
{
	for (auto& componentName : m_loadedComponents)
	{
		auto component = m_knownComponents[componentName];

		component->GetComponent()->DoGameLoad(hModule);
	}
}

void ComponentLoader::AddComponent(fwRefContainer<ComponentData> component)
{
	std::string name = component->GetName();

	m_knownComponents.insert(std::make_pair(name, component));
}

fwRefContainer<ComponentData> ComponentLoader::LoadComponent(const char* componentName)
{
	if (m_loadedComponents.find(componentName) != m_loadedComponents.end())
	{
		return m_knownComponents[componentName];
	}

	auto component = m_knownComponents[componentName];

	// match and resolve dependencies
	auto dependencies = component->GetDepends();

	for (auto& dependency : dependencies)
	{
		// find the first component to provide this
		bool match = false;

		for (auto& it : m_knownComponents)
		{
			auto matchProvides = it.second->GetProvides();

			for (auto& provide : matchProvides)
			{
				if (dependency.IsMatchedBy(provide))
				{
					trace("Resolving dependency for %s by %s (%s).\n", dependency.GetString().c_str(), it.second->GetName().c_str(), provide.GetString().c_str());

					LoadComponent(it.second->GetName().c_str());

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

		if (!match)
		{
			trace("Unable to resolve dependency for %s.\n", dependency.GetString().c_str());
			return nullptr;
		}
	}

	// load the component
	component->Load();

	m_loadedComponents.insert(componentName);

	return component;
}

void ComponentData::Load()
{
	m_component = CreateComponent();

	assert(m_component.GetRef());
}

bool Component::DoGameLoad(void* hModule) { return true; }