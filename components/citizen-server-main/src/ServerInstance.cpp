/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ServerInstance.h>

#include <OptionParser.h>

namespace fx
{
	ServerInstance::ServerInstance()
		: m_shouldTerminate(false)
	{
		// invoke target events
		OnServerCreate(this);
	}

	bool ServerInstance::SetArguments(const std::string& arguments)
	{
		OptionParser* optionParser = Instance<OptionParser>::Get(GetInstanceRegistry());
		
		return optionParser->ParseArgumentString(arguments);
	}

	void ServerInstance::Run()
	{
		// tasks should be running in background threads; we'll just wait until someone wants to get rid of us
		while (!m_shouldTerminate)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}

	fwEvent<ServerInstance*> ServerInstance::OnServerCreate;
}