/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <ComponentLoader.h>
#include <Server.h>

#include <sstream>

namespace fx
{
void Server::Start(int argc, char* argv[])
{
	ComponentLoader* loader = ComponentLoader::GetInstance();
	loader->Initialize();

	fwRefContainer<ComponentData> serverComponent = loader->LoadComponent("citizen:server:main");

	if (!serverComponent.GetRef())
	{
		trace("Could not obtain citizen:server:main component, which is required for the server to start.\n");
		return;
	}

	// combine argv to a separate command list
	std::stringstream args;
	
	for (int i = 1; i < argc; i++)
	{
		args << "\"" << argv[i] << "\" ";
	}

	// create a component instance
	fwRefContainer<Component> componentInstance = serverComponent->CreateInstance(args.str());

	// run the server's main routine
	fwRefContainer<RunnableComponent> runnableServer = dynamic_cast<RunnableComponent*>(componentInstance.GetRef());

	if (runnableServer.GetRef() != nullptr)
	{
		runnableServer->Run();
	}
	else
	{
		trace("citizen:server:main component does not implement RunnableComponent. Exiting.\n");
	}
}
}