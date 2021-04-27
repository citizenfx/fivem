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

namespace fx
{
	bool OptionParser::ParseArgumentString(const std::string& argumentString)
	{
		boost::escaped_list_separator<char> tokenInfo("\\", " ", "\"");
		boost::tokenizer<boost::escaped_list_separator<char>> tokenizer(argumentString, tokenInfo);

		std::vector<std::string> options;
		std::copy_if(tokenizer.begin(), tokenizer.end(), std::back_inserter(options), [](const std::string& opt)
		{
			return !opt.empty();
		});

		assert(options.size() >= 2);
		assert(options[0] == "-fxdk");

		m_pipeAppendix = options[1];

		return true;
	}
}
