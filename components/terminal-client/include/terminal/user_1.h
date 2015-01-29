/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

namespace terminal
{
class AuthenticateDetail;
class TokenBag;

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
	// Authenticates the client to the remote Terminal server with the specified NPv2 token bag.
	//
	virtual concurrency::task<Result<AuthenticateDetail>> AuthenticateWithTokenBag(const TokenBag& tokenBag) = 0;

	//
	// Gets the current primary account ID we are authenticated with, or 0 if none.
	//
	virtual uint64_t GetNPID() = 0;
};

//
// An enumeration containing token types for NPv2 authentication.
//
enum class TokenType
{
	// A Rockstar Online Services ticket.
	ROS = 1,

	// A Steam app ownership ticket.
	Steam = 2
};

//
// A container for one or multiple tokens for authentication.
//
class TokenBag
{
private:
	std::vector<std::pair<TokenType, std::string>> m_tokens;

public:
	inline void AddToken(TokenType type, std::string token)
	{
		m_tokens.push_back(std::make_pair(type, token));
	}

	std::string ToString() const; // not exported, meant for internal usage
};

//
// Result class for an authentication attempt.
//
class AuthenticateDetail
{
private:
	uint64_t m_npID;

	std::map<std::string, std::string> m_identifiers;

public:
	inline AuthenticateDetail()
	{

	}

	inline AuthenticateDetail(uint64_t npID)
		: m_npID(npID)
	{

	}

	AuthenticateDetail(uint64_t npID, const std::string& sessionTokenString);

	inline uint64_t GetNPID() const
	{
		return m_npID;
	}

	inline Result<std::string> GetIdentifier(std::string identifierType) const
	{
		auto it = m_identifiers.find(identifierType);

		return (it != m_identifiers.end()) ? Result<std::string>(it->second) : Result<void>(ErrorCode::InvalidAuthDetails);
	}
};
}