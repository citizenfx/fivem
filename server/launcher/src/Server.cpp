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
		//FatalError("Could not obtain citizen:server:main component, which is required for the server to start.\n");
		return;
	}

	// combine argv to a separate command list
	std::stringstream args;
	
	for (int i = 1; i < argc; i++)
	{
		args << "\"" << argv[i] << "\" ";
	}

	serverComponent->CreateInstance(args.str());

	while (true)
	{
		RunFrame();
	}
}

void Server::RunFrame()
{
	// TODO: somehow select-tick on something?
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
}
}