/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#pragma once

#include <console/ProgramArguments.h>

namespace fx
{
	class OptionParser : public fwRefCountable
	{
	private:
		std::vector<std::pair<std::string, std::string>> m_setList;
		std::vector<ProgramArguments> m_arguments;

	public:
		bool ParseArgumentString(const std::string& argumentString);

	public:
		inline const std::vector<std::pair<std::string, std::string>>& GetSetList() const
		{
			return m_setList;
		}

		inline const std::vector<ProgramArguments>& GetArguments() const
		{
			return m_arguments;
		}
	};
}

DECLARE_INSTANCE_TYPE(fx::OptionParser);
