/*
 * This file is part of FiveM: https://fivem.net/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <Client.h>

#include <optional>

namespace net
{
	class HttpRequest;
}

namespace fx
{
class ServerIdentityProviderBase : public fwRefCountable
{
public:
	//
	// Gets the identifier prefix, such as "steam", "ip" or "ros".
	//
	virtual std::string GetIdentifierPrefix() = 0;

	//
	// Attempts authentication on the current identifier type for the specified client.
	// Should call `cb` on completion.
	//
	virtual void RunAuthentication(const std::shared_ptr<Client>& clientPtr, const std::map<std::string, std::string>& postMap, const std::function<void(boost::optional<std::string>)>& cb) = 0;

	//
	// Attempts authentication on the current identifier type for the specified client, passing the original HTTP request.
	// Should call `cb` on completion.
	//
	virtual void RunAuthentication(const std::shared_ptr<Client>& clientPtr, const fwRefContainer<net::HttpRequest>& request, const std::map<std::string, std::string>& postMap, const std::function<void(boost::optional<std::string>)>& cb)
	{
		RunAuthentication(clientPtr, postMap, cb);
	}

	//
	// Gets the variance level of the identity provider.
	// The variance level describes how likely it is for the identity to change for a given user.
	// Values range from 1 to 5, where 5 is 'most variable'.
	//
	virtual int GetVarianceLevel() = 0;

	//
	// Gets the trust level of the identity provider.
	// The trust level describes how unlikely it is for the user's identity to be spoofed by a malicious client.
	// Values range from 1 to 5, where 5 is 'most trustworthy', i.e. external three-way authentication.
	//
	virtual int GetTrustLevel() = 0;
};

void RegisterServerIdentityProvider(ServerIdentityProviderBase* provider);
}
