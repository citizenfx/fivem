/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

namespace terminal
{
class AuthenticateDetail;

//
// User interface.
//
class IUser1
{
public:
	//
	// The public interface identifier for this interface.
	//
	enum : uint64_t { InterfaceID = 0xbbccff7934ff018b };

public:
	//
	// Authenticates the client to the remote Terminal server as an anonymous server.
	//
	virtual concurrency::task<Result<AuthenticateDetail>> AuthenticateWithLicenseKey(const char* licenseKey) = 0;

	//
	// Gets the current primary account ID we are authenticated with, or 0 if none.
	//
	virtual uint64_t GetNPID() = 0;
};

//
// Result class for an authentication attempt.
//
class AuthenticateDetail
{
private:
	uint64_t m_npID;

public:
	inline AuthenticateDetail()
	{

	}

	inline AuthenticateDetail(uint64_t npID)
		: m_npID(npID)
	{

	}

	inline uint64_t GetNPID()
	{
		return m_npID;
	}
};
}