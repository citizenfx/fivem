#pragma once

class ServerLicensingComponent : public fwRefCountable
{
public:
	ServerLicensingComponent(const std::string& key, const std::string& nucleusToken, const std::string& listingToken)
	{
		m_key = key;
		m_nucleusToken = nucleusToken;
		m_listingToken = listingToken;
	}

	inline std::string GetLicenseKey()
	{
		return m_key;
	}

	inline std::string GetNucleusToken()
	{
		return m_nucleusToken;
	}

	inline std::string GetListingToken()
	{
		return m_listingToken;
	}

private:
	std::string m_key;
	std::string m_nucleusToken;
	std::string m_listingToken;
};

DECLARE_INSTANCE_TYPE(ServerLicensingComponent);

#define LICENSING_EP "https://api.vmp.ir/"
