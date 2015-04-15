/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ServerInstance.h>

#include <OptionParser.h>

#include <boost/property_tree/xml_parser.hpp>

#include <boost/filesystem.hpp>

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
		// initialize the server configuration
		{
			OptionParser* optionParser = Instance<OptionParser>::Get(GetInstanceRegistry());

			boost::filesystem::path rootPath;

			try
			{
				rootPath = boost::filesystem::canonical(optionParser->GetConfigFile());

				m_rootPath = rootPath.string();
			}
			catch (std::exception& error)
			{
			}

			try
			{
				boost::property_tree::ptree pt;
				boost::property_tree::read_xml(optionParser->GetConfigFile(), pt);

				OnReadConfiguration(pt);
			}
			catch (boost::property_tree::ptree_error& error)
			{
				trace("error parsing configuration: %s\n", error.what());
				return;
			}
			catch (std::exception& error)
			{
				trace("error: %s\n", error.what());
				return;
			}
		}

		// tasks should be running in background threads; we'll just wait until someone wants to get rid of us
		while (!m_shouldTerminate)
		{
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
	}
}