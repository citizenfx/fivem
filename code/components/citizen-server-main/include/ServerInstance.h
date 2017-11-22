/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
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
	};
}
