#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <optional>
#include <functional>

namespace cfx::legitimacy
{
using AuthCallback = void(bool success, const char* data, size_t length);
using ProcessHeadersCallback = void(const char*, const char*);

void DLL_IMPORT AuthenticateDiscord(const char* userId, const char* code, AuthCallback callback);
void DLL_IMPORT AuthenticateDiscourse(const char* clientId, const char* authToken, AuthCallback callback);

bool DLL_IMPORT ShouldProcessHeaders(const char* hostname);
void DLL_IMPORT ProcessHeaders(char*, char*);

void DLL_IMPORT InitSteamSDKConnection();
bool DLL_IMPORT IsSteamRunning();
bool DLL_IMPORT IsSteamInitializedWrapper();
void DLL_IMPORT GetSteamAuthTicketWrapper(const std::function<void(std::pair<std::string, std::string>)>& callback, bool enforceSteamAuth);
uint64_t DLL_IMPORT GetSteamIdAsIntWrapper();
std::string DLL_IMPORT GetSteamUsernameWrapper();
void DLL_IMPORT SetSteamRichPresenceWrapper(std::string key, std::string value);
void DLL_IMPORT ResetSteamRichPresenceWrapper();
bool DLL_IMPORT SetSteamAppId(bool legacy);
void DLL_IMPORT WaitForAppSwitchWrapper();
}
