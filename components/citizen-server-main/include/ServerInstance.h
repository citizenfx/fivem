/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <ServerInstanceBase.h>

namespace fx
{
	class ServerInstance : public ServerInstanceBase
	{
	private:
		InstanceRegistry m_instanceRegistry;

		bool m_shouldTerminate;

		std::string m_rootPath;

	public:
		ServerInstance();

		bool SetArguments(const std::string& arguments);

		void Run();

	public:
		virtual InstanceRegistry* GetInstanceRegistry() override
		{
			return &m_instanceRegistry;
		}

		virtual const std::string& GetRootPath() override
		{
			return m_rootPath;
		}
	};
}