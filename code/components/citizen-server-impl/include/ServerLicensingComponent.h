#pragma once

class ServerLicensingComponent : public fwRefCountable
{
public:
	ServerLicensingComponent(const std::string& key)
	{
		m_key = key;
	}

	inline std::string GetLicenseKey()
	{
		return m_key;
	}

private:
	std::string m_key;
};

DECLARE_INSTANCE_TYPE(ServerLicensingComponent);

#define LICENSING_EP "https://keymaster.fivem.net/"
