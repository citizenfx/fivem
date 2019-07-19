#pragma once

class ServerLicensingComponent : public fwRefCountable
{
public:
	ServerLicensingComponent(const std::string& key, const std::string& nucleusToken)
	{
		m_key = key;
		m_nucleusToken = nucleusToken;
	}

	inline std::string GetLicenseKey()
	{
		return m_key;
	}

	inline std::string GetNucleusToken()
	{
		return m_nucleusToken;
	}

private:
	std::string m_key;
	std::string m_nucleusToken;
};

DECLARE_INSTANCE_TYPE(ServerLicensingComponent);

#define LICENSING_EP "https://keymaster.fivem.net/"
