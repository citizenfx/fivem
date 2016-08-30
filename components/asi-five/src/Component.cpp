/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ComponentLoader.h"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <wchar.h>

class ComponentInstance : public Component
{
public:
	virtual bool Initialize();

	virtual bool DoGameLoad(void* module);

	virtual bool Shutdown();
};

bool ComponentInstance::Initialize()
{
	InitFunctionBase::RunAll();

	return true;
}

bool ComponentInstance::DoGameLoad(void* module)
{
	HookFunction::RunAll();

	try
	{
		boost::filesystem::path plugins_path(MakeRelativeCitPath(L"plugins"));
		boost::filesystem::directory_iterator it(plugins_path), end;

		// if the directory doesn't exist, we create it
		if (!boost::filesystem::exists(plugins_path))
		{
			boost::filesystem::create_directory(plugins_path);
		}
		std::vector<wchar_t*> blacklistedAsis = std::vector<wchar_t*>({
			L"openiv.asi"
		});
		// load all .asi files in the plugins/ directory
		while (it != end)
		{
			if (it->path().extension() == ".asi")
			{
				bool bad = false;
				for (std::vector<wchar_t*>::iterator itt = blacklistedAsis.begin(); itt != blacklistedAsis.end(); ++itt){
					
					if (*itt != nullptr && *itt != L"")
					{
						if (wcsicmp(it->path().filename().c_str(), *itt) == 0) {
							bad = true;
							trace(va("Skipping blacklisted ASI %s - this plugin is not compatible with FiveReborn", it->path().filename().string().c_str()));
							if (*itt == L"openiv.asi")
							{
								FatalError("You cannot use OpenIV with FiveReborn. Please use clean game RPFs and remove OpenIV.asi from your plugins. Check fivereborn.com on how to use modded files with FiveReborn.");
							}
						}
					}
				}
				if (!bad) {
					if (!LoadLibrary(it->path().c_str()))
					{
						FatalError(va("Couldn't load %s", it->path().filename().string().c_str()));
					}
				}
			}
			it++;
		}
	}
	catch (...) {}

	return true;
}

bool ComponentInstance::Shutdown()
{
	return true;
}

extern "C" __declspec(dllexport) Component* CreateComponent()
{
	return new ComponentInstance();
}