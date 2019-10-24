#pragma once

#include <CoreConsole.h>
#include <ServerInstanceBase.h>

#include <uvw.hpp>

namespace fx
{
	class MonitorInstance : public ServerInstanceBase
	{
	private:
		bool m_shouldTerminate;

		std::string m_rootPath;

	public:
		MonitorInstance();

		bool SetArguments(const std::string& arguments);

		void Initialize();

		void Run();

	public:
		virtual const std::string& GetRootPath() override
		{
			return m_rootPath;
		}

	private:
		std::vector<std::pair<std::string, std::string>> m_setList;
		std::vector<ProgramArguments> m_arguments;

		std::shared_ptr<uvw::TimerHandle> m_tickTimer;
	};
}
