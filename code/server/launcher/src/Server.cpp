/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <ComponentLoader.h>
#include <Server.h>

class ServerMain
{
public:
	virtual void Run(fwRefContainer<Component> component) = 0;
};

DECLARE_INSTANCE_TYPE(ServerMain);

#include <sstream>

#include <boost/algorithm/string/replace.hpp>

namespace fx
{
	void Server::Start(int argc, char* argv[])
	{
		ComponentLoader* loader = ComponentLoader::GetInstance();
		loader->Initialize();

		// combine argv to a separate command list
		std::stringstream args;
	
		for (int i = 1; i < argc; i++)
		{
			std::string arg = argv[i];
			boost::algorithm::replace_all(arg, "\\", "\\\\");

			args << "\"" << arg << "\" ";
		}

		std::string argStr = args.str();

		// get the right component
		auto compName = "citizen:server:main";

		if (argStr.find("\"+exec\" ") == std::string::npos)
		{
			compName = "citizen:server:monitor";
		}

		fwRefContainer<ComponentData> serverComponent = loader->LoadComponent(compName);

		ComponentLoader::GetInstance()->ForAllComponents([&](fwRefContainer<ComponentData> componentData)
		{
			for (auto& instance : componentData->GetInstances())
			{
				instance->SetCommandLine(argc, argv);
				instance->Initialize();
			}
		});

		if (!serverComponent.GetRef())
		{
			trace("Could not obtain citizen:server:main component, which is required for the server to start.\n");
			return;
		}

		// create a component instance
		fwRefContainer<Component> componentInstance = serverComponent->CreateInstance(argStr);

		// check if the server initialized properly
		if (componentInstance.GetRef() == nullptr)
		{
			return;
		}

		Instance<ServerMain>::Get()->Run(componentInstance);
	}
}
