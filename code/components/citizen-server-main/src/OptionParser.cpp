/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#include "StdInc.h"
#include <OptionParser.h>

#include <ServerInstance.h>

#include <console/OptionTokenizer.h>

namespace fx
{
	bool OptionParser::ParseArgumentString(const std::string& argumentString)
	{
		std::tie(m_arguments, m_setList) = TokenizeCommandLine(argumentString);

		return true;
	}
}
