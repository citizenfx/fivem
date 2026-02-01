#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <optional>
#include <functional>

namespace cfx::legitimacy
{
using AuthCallback = void(bool success, const char* data, size_t length);
using SteamAuthTicketCallback = std::function<void(std::pair<std::string, std::string>)>;

void AuthenticateDiscord(const char* userId, const char* code, AuthCallback callback);
void AuthenticateDiscourse(const char* clientId, const char* authToken, AuthCallback callback);

bool ShouldProcessHeaders(const char* hostname);
void ProcessHeaders(char* key, char* value);

void InitSteamSDKConnection();
bool IsSteamRunning();
bool IsSteamInitializedWrapper();
void GetSteamAuthTicketWrapper(const SteamAuthTicketCallback& callback, bool enforceSteamAuth);
uint64_t GetSteamIdAsIntWrapper();
std::string GetSteamUsernameWrapper();
void SetSteamRichPresenceWrapper(std::string key, std::string value);
void ResetSteamRichPresenceWrapper();
bool SetSteamAppId(bool legacy);
void WaitForAppSwitchWrapper();
}
