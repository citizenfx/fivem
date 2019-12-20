/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <ServerInstanceBase.h>

#include <console/Console.CommandHelpers.h>

namespace fx
{
	class ServerInstance : public ServerInstanceBase
	{
	private:
		bool m_shouldTerminate;

		std::string m_rootPath;

	public:
		ServerInstance();

		bool SetArguments(const std::string& arguments);

		void Run();

	public:
		virtual const std::string& GetRootPath() override
		{
			return m_rootPath;
		}

	private:
		std::shared_ptr<ConsoleCommand> m_execCommand;
		std::shared_ptr<ConsoleCommand> m_quitCommand_0;
		std::shared_ptr<ConsoleCommand> m_quitCommand_1;
	};
}
