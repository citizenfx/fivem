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

#include <LaunchMode.h>
#include <CrossBuildRuntime.h>

#ifndef IS_FXSERVER
#include <CL2LaunchMode.h>
#include <CfxState.h>
#endif

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

	// parse and load additional components
	fwPlatformString componentsName = _P("components.json");
	FILE* componentCache = _pfopen(MakeRelativeCitPath(componentsName).c_str(), _P("rb"));
	if (!componentCache)
	{
		FatalError("Could not find component cache storage file (components.json).");
	}

	// read component cache file
	fseek(componentCache, 0, SEEK_END);
	int length = ftell(componentCache);

	fseek(componentCache, 0, SEEK_SET);

	std::vector<char> cacheBuf(length + 1);

	fread(cacheBuf.data(), 1, length, componentCache);
	cacheBuf[length] = '\0';

	fclose(componentCache);

	InitializeWithString({cacheBuf.data(), cacheBuf.size()});
}

void ComponentLoader::InitializeWithString(std::string_view cacheBuf)
{
	// run local initialization functions
	InitFunctionBase::RunAll();

	// set up the root component
	m_rootComponent = FxGameComponent::Create();
	AddComponent(m_rootComponent);

	// parse the list
	rapidjson::Document doc;
	doc.Parse(cacheBuf.data(), cacheBuf.size());

	if (doc.HasParseError())
	{
		FatalError("Error parsing components.json: %d", doc.GetParseError());
	}

#ifdef _WIN32
	wchar_t moduleName[MAX_PATH];
	GetModuleFileNameW(GetModuleHandleW(NULL), moduleName, std::size(moduleName));
#endif

	// look through the list for components to load
	for (auto it = doc.Begin(); it != doc.End(); ++it)
	{
		const auto& componentNameElement = *it;

		fwPlatformString nameWide;
		nameWide.reserve(componentNameElement.GetStringLength());
		nameWide.assign(componentNameElement.GetString());

		// replace colons with dashes
		std::replace(nameWide.begin(), nameWide.end(), ':', '-');

#ifdef _WIN32
		// don't load some useless stuff for ChromeBrowser
		if (wcsstr(moduleName, L"ChromeBrowser"))
		{
			if (nameWide != L"nui-core" && nameWide != L"vfs-core" && nameWide != L"net-base")
			{
				continue;
			}
		}

#ifndef IS_FXSERVER
		std::vector<std::wstring> neededComponentsList;

#ifdef GTA_FIVE
		if (wcsstr(moduleName, L"_ROS"))
		{
			neededComponentsList = {
				L"ros-patches-five",
				L"net-http-server",
				L"net-tcp-server",
				L"net-base",
				L"net-packet",
			};
		}
#endif

		// don't load some useless stuff for FXNode as well
		if (launch::IsFXNode())
		{
			neededComponentsList = {
				L"citizen-scripting-v8node",
				L"citizen-scripting-core",
				L"citizen-resources-core",
				L"scripting-gta",
				L"rage-scripting-five",
				L"rage-allocator-five",
				L"http-client",
				L"citizen-scripting-core",
				L"conhost-v2",
				L"vfs-core",
				L"net-tcp-server",
				L"net-base",
				L"net-packet",
			};
		}

		if (!neededComponentsList.empty())
		{
			bool wantThisComponent = false;

			for (auto& componentName : neededComponentsList)
			{
				if (nameWide == componentName)
				{
					wantThisComponent = true;
					break;
				}
			}

			if (!wantThisComponent)
			{
				continue;
			}
		}
#endif
#endif

#ifndef IS_FXSERVER
		if (nameWide == L"adhesive")
		{
			if (launch::IsSDK() || launch::IsSDKGuest())
			{
				continue;
			}

			if (CfxIsWine() || GetModuleHandleW(L"xtajit64.dll") != nullptr)
			{
				auto cfxState = CfxState::Get();
				if (cfxState->IsMasterProcess())
				{
					std::wstring environmentType;

					if (CfxIsWine())
					{
						environmentType = L"Wine";
					}
					else if (GetModuleHandleW(L"xtajit64.dll") != nullptr)
					{
						environmentType = L"Windows on ARM";
					}

					MessageBoxW(NULL, 
						va(
							L"The game is running in insecure mode because %s is not supported by the anti-cheat components at this time.\n"
							L"Most servers, as well as some authentication features will be unavailable.",
							environmentType),
						L"Cfx.re: Insecure mode", MB_OK | MB_ICONWARNING);
				}

				AddComponent(new DllGameComponent(va(PLATFORM_LIBRARY_STRING, L"sticky")));

				continue;
			}
		}
#endif

		AddComponent(new DllGameComponent(va(PLATFORM_LIBRARY_STRING, nameWide.c_str())));
	}

	for (auto& componentNamePair : m_knownComponents)
	{
		auto& component = componentNamePair.second;

		LoadDependencies(this, component);
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
		auto componentName = sortedList.front()->GetName();
		sortedList.pop();

		auto comp = LoadComponent(componentName.c_str());
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
	for (const auto& dependency : component->GetDepends())
	{
		// find the first component to provide this
		bool match = false;
			
		for (auto& knownNameComponentPair : loader->GetKnownComponents())
		{
			auto& knownComponent = knownNameComponentPair.second;

			auto matchProvides = knownComponent->GetProvides();

			for (auto& provide : matchProvides)
			{
				if (dependency.IsMatchedBy(provide))
				{
					component->AddDependency(knownComponent);

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

		trace("pre-gameload component %s\n", component->GetName());

		if (!instances.empty())
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
		if (strcmp(componentName, "adhesive") == 0)
		{
			component = m_knownComponents["sticky"];
		}

		if (!component.GetRef())
		{
			FatalError("Unknown component %s.", componentName);
		}
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

#ifdef _WIN32
#include <shellapi.h>
#endif

fwRefContainer<Component> ComponentData::CreateManualInstance()
{
	fwRefContainer<Component> instance = CreateComponent();

#ifdef _WIN32
	static std::vector<std::string> argvStrs;
	static std::vector<char*> argvRefs;

	static auto _ = ([]()
	{
		int argc;
		LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

		argvStrs.resize(argc);
		argvRefs.resize(argc);

		for (int i = 0; i < argc; i++)
		{
			argvStrs[i] = ToNarrow(argv[i]);
			argvRefs[i] = &argvStrs[i][0];
		}

		LocalFree(argv);

		return 0;
	})();

	instance->SetCommandLine(argvRefs.size(), &argvRefs[0]);
#endif

	m_instances.push_back(instance);

	return instance;
}

void ComponentData::Load()
{
	SetLoaded(true);
}
