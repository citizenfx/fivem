/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
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
