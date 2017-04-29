/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

//==========================  Open Steamworks  ================================
//
// This file is part of the Open Steamworks project. All individuals associated
// with this project do not claim ownership of the contents
// 
// The code, comments, and all related files, projects, resources,
// redistributables included with this project are Copyright Valve Corporation.
// Additionally, Valve, the Valve logo, Half-Life, the Half-Life logo, the
// Lambda logo, Steam, the Steam logo, Team Fortress, the Team Fortress logo,
// Opposing Force, Day of Defeat, the Day of Defeat logo, Counter-Strike, the
// Counter-Strike logo, Source, the Source logo, and Counter-Strike Condition
// Zero are trademarks and or registered trademarks of Valve Corporation.
// All other trademarks are property of their respective owners.
//
//=============================================================================

typedef void IClientApps;
typedef void IClientBilling;
typedef void IClientContentServer;
typedef void IClientFriends;
typedef void IClientGameCoordinator;
typedef void IClientGameServer;
typedef void IClientGameServerItems;
typedef void IClientGameStats;
typedef void IClientMasterServerUpdater;
typedef void IClientMatchmaking;
typedef void IClientMatchmakingServers;
typedef void IClientNetworking;
typedef void IClientRemoteStorage;
typedef void IClientUser;
typedef void IClientUserItems;
typedef void IClientUserStats;
typedef void IClientUtils;
typedef void IP2PController;
typedef void IClientAppManager;
typedef void IClientDepotBuilder;
typedef void IConCommandBaseAccessor;
typedef void IClientGameCoordinator;
typedef void IClientHTTP;
typedef void IClientGameServerStats;
typedef void IClientConfigStore;
typedef void IClientScreenshots;
typedef void IClientAudio;
typedef void IClientUnifiedMessages;
typedef void IClientStreamLauncher;
typedef void IClientNetworkDeviceManager;
typedef void IClientController;
typedef void IClientParentalSettings;
typedef void IClientDeviceAuth;
typedef void IClientMusic;
typedef void IClientProductBuilder;
typedef void IClientRemoteClientManager;
typedef void IClientRemoteControlManager;
typedef void IClientShortcuts;
typedef void IClientStreamClient;
typedef void IClientUGC;
typedef void IClientVR;

typedef int32_t int32;

typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef uint32_t unknown_ret;

typedef int EAccountType;
typedef int EUniverse;
typedef int32_t HSteamPipe;
typedef int32_t HSteamUser;

typedef uint64_t SteamAPICall_t;
typedef int ENotificationPosition;

typedef void* SteamAPIWarningMessageHook_t;

class IClientEngine
{

public:
	virtual HSteamPipe CreateSteamPipe() = 0;
	virtual bool BReleaseSteamPipe(HSteamPipe hSteamPipe) = 0;

	virtual HSteamUser CreateGlobalUser(HSteamPipe* phSteamPipe) = 0;
	virtual HSteamUser ConnectToGlobalUser(HSteamPipe hSteamPipe) = 0;

	virtual HSteamUser CreateLocalUser(HSteamPipe* phSteamPipe, EAccountType eAccountType) = 0;
	virtual void CreatePipeToLocalUser(HSteamUser hSteamUser, HSteamPipe* phSteamPipe) = 0;

	virtual void ReleaseUser(HSteamPipe hSteamPipe, HSteamUser hUser) = 0;

	virtual bool IsValidHSteamUserPipe(HSteamPipe hSteamPipe, HSteamUser hUser) = 0;

	virtual IClientUser *GetIClientUser(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientGameServer *GetIClientGameServer(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	virtual void SetLocalIPBinding(uint32 unIP, uint16 usPort) = 0;
	virtual char const *GetUniverseName(EUniverse eUniverse) = 0;

	virtual IClientFriends *GetIClientFriends(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientUtils *GetIClientUtils(HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientBilling *GetIClientBilling(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientMatchmaking *GetIClientMatchmaking(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientApps *GetIClientApps(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientMatchmakingServers *GetIClientMatchmakingServers(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;

	virtual void RunFrame() = 0;
	virtual uint32 GetIPCCallCount() = 0;

	virtual IClientUserStats *GetIClientUserStats(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientGameServerStats *GetIClientGameServerStats(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientNetworking *GetIClientNetworking(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientRemoteStorage *GetIClientRemoteStorage(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientScreenshots *GetIClientScreenshots(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;

	virtual void SetWarningMessageHook(SteamAPIWarningMessageHook_t pFunction) = 0;

	virtual IClientGameCoordinator *GetIClientGameCoordinator(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;

	virtual void SetOverlayNotificationPosition(ENotificationPosition eNotificationPosition) = 0;
	virtual bool HookScreenshots(bool bHook) = 0;
	virtual bool IsOverlayEnabled() = 0;

	virtual bool GetAPICallResult(HSteamPipe hSteamPipe, SteamAPICall_t hSteamAPICall, void* pCallback, int cubCallback, int iCallbackExpected, bool* pbFailed) = 0;

	virtual IClientProductBuilder *GetIClientProductBuilder(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientDepotBuilder *GetIClientDepotBuilder(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientNetworkDeviceManager *GetIClientNetworkDeviceManager(HSteamPipe hSteamPipe, char const* pchVersion) = 0;

	virtual void ConCommandInit(IConCommandBaseAccessor *pAccessor) = 0;

	virtual IClientAppManager *GetIClientAppManager(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientConfigStore *GetIClientConfigStore(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;

	virtual bool BOverlayNeedsPresent() = 0;

	virtual IClientGameStats *GetIClientGameStats(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientHTTP *GetIClientHTTP(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;

	virtual bool BShutdownIfAllPipesClosed() = 0;

	virtual IClientAudio *GetIClientAudio(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientMusic *GetIClientMusic(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientUnifiedMessages *GetIClientUnifiedMessages(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientController *GetIClientController(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientParentalSettings *GetIClientParentalSettings(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientStreamLauncher *GetIClientStreamLauncher(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientDeviceAuth *GetIClientDeviceAuth(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;

	virtual IClientRemoteClientManager *GetIClientRemoteClientManager(HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientStreamClient *GetIClientStreamClient(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientShortcuts *GetIClientShortcuts(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientRemoteControlManager *GetIClientRemoteControlManager(HSteamPipe hSteamPipe, char const* pchVersion) = 0;

	virtual unknown_ret Set_ClientAPI_CPostAPIResultInProcess(void(*)(uint64 ulUnk, void * pUnk, uint32 uUnk, int32 iUnk)) = 0;
	virtual unknown_ret Remove_ClientAPI_CPostAPIResultInProcess(void(*)(uint64 ulUnk, void * pUnk, uint32 uUnk, int32 iUnk)) = 0;
	virtual IClientUGC *GetIClientUGC(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) = 0;
	virtual IClientVR *GetIClientVR(char const * pchVersion) = 0;
};
