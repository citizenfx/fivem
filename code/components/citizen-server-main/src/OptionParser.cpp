/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <OptionParser.h>

#include <ServerInstance.h>

#include <boost/tokenizer.hpp>
#include <boost/program_options.hpp>

namespace fx
{
	bool OptionParser::ParseArgumentString(const std::string& argumentString)
	{
		boost::program_options::options_description desc;
		desc.add_options()
			("help,?", "Output help information.")
			("config-file,c", boost::program_options::value<std::string>()->required(), "The server configuration file to use.");

		boost::program_options::positional_options_description p;
		p.add("config-file", 1);

		// tokenize the input string
		boost::escaped_list_separator<char> tokenInfo("\\", " ", "\"");
		boost::tokenizer<boost::escaped_list_separator<char>> tokenizer(argumentString, tokenInfo);

		std::vector<std::string> options;
		std::copy_if(tokenizer.begin(), tokenizer.end(), std::back_inserter(options), [] (const std::string& opt)
		{
			return !opt.empty();
		});

		// parse the options
		boost::program_options::variables_map m;

		try
		{
			boost::program_options::store(boost::program_options::command_line_parser(options).
										  options(desc).positional(p).run(),
										  m);

			if (m.count("help") > 0)
			{
				std::ostringstream s;
				s << desc << std::endl;

				printf("%s\n", s.str().c_str());

				return false;
			}

			boost::program_options::notify(m);

			m_configFile = m["config-file"].as<std::string>();

			return true;
		}
		catch (boost::program_options::required_option& e)
		{
			trace("Required option missing: %s\n", e.what());

			return false;
		}
		catch (boost::program_options::error& e)
		{
			trace("error: %s\n", e.what());

			return false;
		}
	}
}

static InitFunction initFunction([] ()
{
	fx::ServerInstance::OnServerCreate.Connect([] (fx::ServerInstanceBase* server)
	{
		Instance<fx::OptionParser>::Set(new fx::OptionParser(), server->GetInstanceRegistry());
	});
});