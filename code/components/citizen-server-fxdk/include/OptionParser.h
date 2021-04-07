/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
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
