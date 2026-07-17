/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
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
