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
        boost::escaped_list_separator<char> tokenInfo("\\", " ", "\"");
        boost::tokenizer<boost::escaped_list_separator<char>> tokenizer(argumentString, tokenInfo);

        std::vector<std::string> options;
        std::copy_if(tokenizer.begin(), tokenizer.end(), std::back_inserter(options), [] (const std::string& opt)
        {
                return !opt.empty();
        });

		ProgramArguments arguments(options);

		// the argument list to pass after the game initialized
		std::vector<ProgramArguments> argumentBits;
		std::vector<std::pair<std::string, std::string>> setList;

		{
			for (int i = 0; i < arguments.Count(); i++)
			{
				const std::string& argument = arguments[i];

				// if this is a command-like
				if (argument.length() > 1 && argument[0] == '+')
				{
					// if this is a set command, queue it for the early execution buffer
					if (argument == "+set" && (i + 2) < arguments.Count())
					{
						setList.push_back({ arguments[i + 1], arguments[i + 2] });
					}
					else
					{
						// capture bits up to the next +
						std::vector<std::string> thisBits;
						thisBits.push_back(argument.substr(1));

						for (size_t j = i + 1; j < arguments.Count(); j++)
						{
							const std::string& bit = arguments[j];

							if (bit.length() > 0 && bit[0] == '+')
							{
								break;
							}

							thisBits.push_back(bit);
						}

						argumentBits.push_back(ProgramArguments{ thisBits });
					}
				}
			}
		}

		m_arguments = argumentBits;
		m_setList = setList;

		return true;
	}
}
