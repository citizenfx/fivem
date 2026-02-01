#include "SharedLegitimacyAPI.h"
#include "CfxLegitimacyAPI.h"

namespace cfx::legitimacy
{
void AuthenticateDiscord(const char* userId, const char* code, AuthCallback callback)
{
	return cfx_discord_authenticate(userId, code, callback);
}
void AuthenticateDiscourse(const char* clientId, const char* authToken, AuthCallback callback)
{
	return cfx_discourse_authenticate(clientId, authToken, callback);
}

bool ShouldProcessHeaders(const char* hostname)
{
	return cfx_should_process_headers(hostname);
}

void ProcessHeaders(char* key, char* value)
{
	return cfx_process_headers(key, value);
}

void InitSteamSDKConnection()
{
	return cfx_steam_init();
}

bool IsSteamRunning()
{
	return cfx_steam_is_running();
}

bool IsSteamInitializedWrapper()
{
	return cfx_steam_is_initialized();
}

void GetSteamAuthTicketWrapper(const SteamAuthTicketCallback& callback, bool enforceSteamAuth)
{
	auto* fn = new SteamAuthTicketCallback(callback);
	cfx_steam_get_auth_ticket([](const char* ticket, size_t ticket_len, const char* response, size_t response_len, void* user_data)
	{
		auto cb = static_cast<SteamAuthTicketCallback*>(user_data);
		std::string ticketStr;
		if (ticket && ticket_len > 0)
		{
			ticketStr.assign(ticket, ticket_len);
		}
		std::string responseStr;
		if (response && response_len > 0)
		{
			responseStr.assign(response, response_len);
		}
		(*cb)(std::make_pair(responseStr, ticketStr));
		delete cb;
	},
	enforceSteamAuth, fn);
}

uint64_t GetSteamIdAsIntWrapper()
{
	return cfx_steam_get_id();
}

std::string GetSteamUsernameWrapper()
{
	std::string result;
	const char* username = cfx_steam_get_username();
	if (username)
	{
		result = std::string(username);
	}
	return result;
}

void SetSteamRichPresenceWrapper(std::string key, std::string value)
{
	return cfx_steam_set_rich_presence(key.c_str(), value.c_str());
}

void ResetSteamRichPresenceWrapper()
{
	return cfx_steam_reset_rich_presence();
}

bool SetSteamAppId(bool legacy)
{
	return cfx_steam_set_app_id(legacy);
}

void WaitForAppSwitchWrapper()
{
	return cfx_steam_wait_for_app_switch();
}
}
