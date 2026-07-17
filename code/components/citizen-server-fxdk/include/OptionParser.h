/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <console/ProgramArguments.h>

namespace fx
{
	class OptionParser : public fwRefCountable
	{
	private:
		std::string m_pipeAppendix;

	public:
		bool ParseArgumentString(const std::string& argumentString);

	public:
		inline const std::string& GetPipeAppendix() const
		{
			return m_pipeAppendix;
		}
	};
}

DECLARE_INSTANCE_TYPE(fx::OptionParser);
