/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

namespace fx
{
	class ServerInstance : public fwRefCountable
	{
	private:
		InstanceRegistry m_instanceRegistry;

		bool m_shouldTerminate;

	public:
		ServerInstance();

		bool SetArguments(const std::string& arguments);

		void Run();

	public:
		inline InstanceRegistry* GetInstanceRegistry()
		{
			return &m_instanceRegistry;
		}

	public:
		static fwEvent<ServerInstance*> OnServerCreate;
	};
}