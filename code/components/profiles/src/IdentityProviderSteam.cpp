/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "ProfileManagerImpl.h"
#include "SteamComponentAPI.h"

#include <base64.h>

class SteamIdentityProvider : public ProfileIdentityProvider
{
public:
	virtual const char* GetIdentifierKey() override;

	virtual bool RequiresCredentials() override;

	virtual concurrency::task<ProfileIdentityResult> ProcessIdentity(fwRefContainer<Profile> profile, const std::map<std::string, std::string>& parameters) override;
};

const char* SteamIdentityProvider::GetIdentifierKey()
{
	return "steam";
}

bool SteamIdentityProvider::RequiresCredentials()
{
	return false;
}

concurrency::task<ProfileIdentityResult> SteamIdentityProvider::ProcessIdentity(fwRefContainer<Profile> profile, const std::map<std::string, std::string>& parameters)
{
	// get Steam
	auto steamComponent = Instance<ISteamComponent>::Get();

	// if Steam isn't running, return an error
	if (!steamComponent->IsSteamRunning())
	{
		steamComponent->Initialize();

		if (!steamComponent->IsSteamRunning())
		{
			return concurrency::task_from_result<ProfileIdentityResult>(ProfileIdentityResult("Steam must be running to sign in to a Steam-based profile."));
		}
	}

	// get the Steam interfaces
	IClientEngine* steamClient = steamComponent->GetPrivateClient();

	InterfaceMapper steamUser(steamClient->GetIClientUser(steamComponent->GetHSteamUser(), steamComponent->GetHSteamPipe(), "CLIENTUSER_INTERFACE_VERSION001"));

	if (!steamUser.IsValid())
	{
		return concurrency::task_from_result<ProfileIdentityResult>(ProfileIdentityResult("Steam must be running to sign in to a Steam-based profile."));
	}

	InterfaceMapper steamUtils(steamClient->GetIClientUtils(steamComponent->GetHSteamPipe(), "CLIENTUTILS_INTERFACE_VERSION001"));

	// verify we've the correct steam ID for this profile
	fwRefContainer<ProfileImpl> profileImpl(profile);

	uint64_t steamID;
	steamUser.Invoke<void>("GetSteamID", &steamID);

	bool found = false;
	
	for (int i = 0; i < profileImpl->GetNumIdentifiers(); i++)
	{
		auto identifier = profileImpl->GetIdentifierInternal(i);

		if (identifier.first == "steam")
		{
			if (_strtoui64(identifier.second.c_str(), nullptr, 16) == steamID)
			{
				found = true;
				break;
			}
		}
	}

	if (!found)
	{
		return concurrency::task_from_result<ProfileIdentityResult>(ProfileIdentityResult("You are not currently signed in to this profile using the Steam client. Please sign into this profile using Steam before signing in."));
	}

	// get a ticket
	uint32_t ticketLength;
	uint8_t ticketBuffer[4096 + 2 + 8];

	steamUser.Invoke<int>("GetAuthSessionTicket", &ticketBuffer[10], (int)sizeof(ticketBuffer) - 10, &ticketLength);

	// add the steam ID and ticket length to the buffer
	*(uint64_t*)&ticketBuffer[0] = steamID;
	*(uint16_t*)&ticketBuffer[8] = ticketLength;

	// encode the ticket buffer
	size_t outLength;
	char* str = base64_encode(ticketBuffer, ticketLength + 10, &outLength);

	// and return a result
	auto task = concurrency::task_from_result<ProfileIdentityResult>(ProfileIdentityResult(0, va("%d&", steamComponent->GetParentAppID()) + std::string(str, outLength)));

	free(str);

	return task;
}

static InitFunction initFunction([] ()
{
	ProfileManagerImpl* ourProfileManager = static_cast<ProfileManagerImpl*>(Instance<ProfileManager>::Get());

	ourProfileManager->AddIdentityProvider(new SteamIdentityProvider());
});