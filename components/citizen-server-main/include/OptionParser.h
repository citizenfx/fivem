/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

namespace fx
{
	class OptionParser
	{
	private:
		std::string m_configFile;

	public:
		bool ParseArgumentString(const std::string& argumentString);

	public:
		inline const std::string& GetConfigFile() const
		{
			return m_configFile;
		}
	};
}

DECLARE_INSTANCE_TYPE(fx::OptionParser);