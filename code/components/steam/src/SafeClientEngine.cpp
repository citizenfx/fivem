/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <SafeClientEngine.h>
#include <ClientEngineMapper.h>

class SafeClientEngine : public IClientEngine
{
private:
	IClientEngine* m_baseEngine;

	ClientEngineMapper m_engineMapper;

public:
	SafeClientEngine(IClientEngine* baseEngine, HSteamPipe steamPipe, HSteamUser steamUser)
		: m_baseEngine(baseEngine), m_engineMapper(baseEngine, steamPipe, steamUser)
	{
		
	}

	virtual HSteamPipe CreateSteamPipe() override
	{
		return m_baseEngine->CreateSteamPipe();
	}

	virtual bool BReleaseSteamPipe(HSteamPipe hSteamPipe) override
	{
		return m_baseEngine->BReleaseSteamPipe(hSteamPipe);
	}

	virtual HSteamUser CreateGlobalUser(HSteamPipe* phSteamPipe) override
	{
		return m_baseEngine->CreateGlobalUser(phSteamPipe);
	}

	virtual HSteamUser ConnectToGlobalUser(HSteamPipe hSteamPipe) override
	{
		return m_baseEngine->ConnectToGlobalUser(hSteamPipe);
	}

	virtual HSteamUser CreateLocalUser(HSteamPipe* phSteamPipe, EAccountType eAccountType) override
	{
		return m_baseEngine->CreateLocalUser(phSteamPipe, eAccountType);
	}

	virtual void CreatePipeToLocalUser(HSteamUser hSteamUser, HSteamPipe* phSteamPipe) override
	{
		return m_baseEngine->CreatePipeToLocalUser(hSteamUser, phSteamPipe);
	}

	virtual void ReleaseUser(HSteamPipe hSteamPipe, HSteamUser hUser) override
	{
		return m_baseEngine->ReleaseUser(hSteamPipe, hUser);
	}

	virtual bool IsValidHSteamUserPipe(HSteamPipe hSteamPipe, HSteamUser hUser) override
	{
		return m_baseEngine->IsValidHSteamUserPipe(hSteamPipe, hUser);
	}

	virtual IClientUser *GetIClientUser(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientGameServer *GetIClientGameServer(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}

	virtual void SetLocalIPBinding(uint32 unIP, uint16 usPort) override
	{

	}
	virtual char const *GetUniverseName(EUniverse eUniverse) override
	{
		return "";
	}

	virtual IClientFriends *GetIClientFriends(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientUtils *GetIClientUtils(HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientBilling *GetIClientBilling(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientMatchmaking *GetIClientMatchmaking(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientApps *GetIClientApps(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientMatchmakingServers *GetIClientMatchmakingServers(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}

	virtual void RunFrame() override
	{

	}
	virtual uint32 GetIPCCallCount() override
	{
		return 0;
	}

	virtual IClientUserStats *GetIClientUserStats(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientGameServerStats *GetIClientGameServerStats(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientNetworking *GetIClientNetworking(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientRemoteStorage *GetIClientRemoteStorage(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientScreenshots *GetIClientScreenshots(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}

	virtual void SetWarningMessageHook(SteamAPIWarningMessageHook_t pFunction) override
	{
		
	}

	virtual IClientGameCoordinator *GetIClientGameCoordinator(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}

	virtual void SetOverlayNotificationPosition(ENotificationPosition eNotificationPosition) override
	{
		
	}
	virtual bool HookScreenshots(bool bHook) override
	{
		return false;
	}
	virtual bool IsOverlayEnabled() override
	{
		return false;
	}

	virtual bool GetAPICallResult(HSteamPipe hSteamPipe, SteamAPICall_t hSteamAPICall, void* pCallback, int cubCallback, int iCallbackExpected, bool* pbFailed) override
	{
		return false;
	}

	virtual IClientProductBuilder *GetIClientProductBuilder(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientDepotBuilder *GetIClientDepotBuilder(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientNetworkDeviceManager *GetIClientNetworkDeviceManager(HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}

	virtual void ConCommandInit(IConCommandBaseAccessor *pAccessor) override
	{

	}

	virtual IClientAppManager *GetIClientAppManager(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientConfigStore *GetIClientConfigStore(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}

	virtual bool BOverlayNeedsPresent() override
	{
		return false;
	}

	virtual IClientGameStats *GetIClientGameStats(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientHTTP *GetIClientHTTP(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}

	virtual bool BShutdownIfAllPipesClosed() override
	{
		return false;
	}

	virtual IClientAudio *GetIClientAudio(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientMusic *GetIClientMusic(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientUnifiedMessages *GetIClientUnifiedMessages(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientController *GetIClientController(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientParentalSettings *GetIClientParentalSettings(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientStreamLauncher *GetIClientStreamLauncher(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientDeviceAuth *GetIClientDeviceAuth(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}

	virtual IClientRemoteClientManager *GetIClientRemoteClientManager(HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientStreamClient *GetIClientStreamClient(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientShortcuts *GetIClientShortcuts(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientRemoteControlManager *GetIClientRemoteControlManager(HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}

	virtual unknown_ret Set_ClientAPI_CPostAPIResultInProcess(void(*)(uint64 ulUnk, void * pUnk, uint32 uUnk, int32 iUnk)) override
	{
		return 0;
	}
	virtual unknown_ret Remove_ClientAPI_CPostAPIResultInProcess(void(*)(uint64 ulUnk, void * pUnk, uint32 uUnk, int32 iUnk)) override
	{
		return 0;
	}
	virtual IClientUGC *GetIClientUGC(HSteamUser hSteamUser, HSteamPipe hSteamPipe, char const* pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
	virtual IClientVR *GetIClientVR(char const * pchVersion) override
	{
		return m_engineMapper.GetInterface(pchVersion);
	}
};

IClientEngine* CreateSafeClientEngine(IClientEngine* baseEngine, HSteamPipe steamPipe, HSteamUser steamUser)
{
	static SafeClientEngine engine(baseEngine, steamPipe, steamUser);

	return &engine;
}