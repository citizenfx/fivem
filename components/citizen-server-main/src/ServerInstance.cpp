/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ServerInstance.h>

namespace fx
{
	ServerInstance::ServerInstance()
	{
		// invoke target events
		OnServerCreate(this);
	}

	void ServerInstance::SetArguments(const std::string& arguments)
	{
		//OptionParser* optionParser = Instance<OptionParser>::Get(GetInstanceRegistry());
		//
		//optionParser->ParseArgumentString(arguments);
	}

	void ServerInstance::Run()
	{
		
	}

	fwEvent<fwRefContainer<ServerInstance>> ServerInstance::OnServerCreate;
}