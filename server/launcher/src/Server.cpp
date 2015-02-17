/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <ComponentLoader.h>
#include <Server.h>

namespace fx
{
void Server::Start(int argc, char* argv[])
{
	ComponentLoader* loader = ComponentLoader::GetInstance();
	loader->Initialize();

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