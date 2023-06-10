/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <ComponentLoader.h>
#include <Server.h>

#include <cfx_version.h>

#ifdef _WIN32
void InitializeMiniDumpOverride();
#endif

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

#ifdef _WIN32
		InitializeMiniDumpOverride();
#endif

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
		bool isFxdkMode = false;

		// get the right component
		auto compName = "citizen:server:main";

		if (argStr.find("\"+exec\" ") == std::string::npos)
		{
			compName = "citizen:server:monitor";
		}

		if ((argStr.find("\"--version\"") != std::string::npos &&
			argStr.find("\"--start-node\"") == std::string::npos) ||
			argStr.find("\"-V\"") != std::string::npos)
		{
			fmt::printf("Cfx.re Platform Server (FXServer) %s\n", GIT_DESCRIPTION);
			fmt::printf("Build date: %s\n", __DATE__);
			fmt::printf("\n");
			fmt::printf("Usage restrictions apply, see <https://fivem.net/terms> for further information.\n");
			fmt::printf("Commercial usage is not permitted without prior approval.\n");
			return;
		}

		if ((argStr.find("\"--help\"") != std::string::npos &&
			argStr.find("\"--start-node\"") == std::string::npos) ||
			argStr.find("\"-?\"") != std::string::npos)
		{
			fmt::printf("Usage:\n\n\t%s <+startupargs>\n\nSee https://docs.fivem.net/ for more usage information.\n", argv[0]);
			fmt::printf("\n");
			fmt::printf("Now... go make something great.\n");
			return;
		}

#ifdef _WIN32
		if (argStr.find("-fxdk") != std::string::npos)
		{
			isFxdkMode = true;
			compName = "citizen:server:fxdk";
		}
#endif

		fwRefContainer<ComponentData> serverComponent = loader->LoadComponent(compName);

		ComponentLoader::GetInstance()->ForAllComponents([&](fwRefContainer<ComponentData> componentData)
		{
			if (isFxdkMode && componentData->GetName() == "svadhesive")
			{
				return;
			}

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
