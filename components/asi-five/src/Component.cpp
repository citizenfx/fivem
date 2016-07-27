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
		char* const blacklistedAsis[] = {"openiv.asi"};
		// load all .asi files in the plugins/ directory
		while (it != end)
		{
			if (it->path().extension() == ".asi")
			{
				bool bad = false;
				for (int i = 0; i < sizeof(blacklistedAsis); i++) {
					if (boost::iequals(it->path().filename().string(), std::string(blacklistedAsis[i]))) {
						bad = true;
						trace(va("Skipping blacklisted ASI %s", it->path().filename().string()));
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