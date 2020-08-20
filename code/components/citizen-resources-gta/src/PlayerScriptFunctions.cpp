/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>

#include <ScriptEngine.h>
#include <NetworkPlayerMgr.h>

#include <CrossBuildRuntime.h>

#ifdef GTA_FIVE
template<int Build>
static inline int GetServerId(const rlGamerInfo<Build>& platformData)
{
	return (platformData.peerAddress.localAddr.ip.addr & 0xFFFF) ^ 0xFEED;
}

static inline int DoGetServerId(CNetGamePlayer* player)
{
	if (Is2060())
	{
		return GetServerId(*player->GetGamerInfo<2060>());
	}
	else
	{
		return GetServerId(*player->GetGamerInfo<1604>());
	}
}
#else
static inline int DoGetServerId(CNetGamePlayer* player)
{
	return (player->GetGamerInfo()->peerAddress.localAddr.ip.addr & 0xFFFF) ^ 0xFEED;
}
#endif

static InitFunction initFunction([] ()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_FROM_SERVER_ID", [] (fx::ScriptContext& context)
	{
		int serverId = context.GetArgument<int>(0);

		for (int i = 0; i < 256; i++)
		{
			CNetGamePlayer* player = CNetworkPlayerMgr::GetPlayer(i);

			if (player)
			{
				if (DoGetServerId(player) == serverId)
				{
					context.SetResult(i);
					return;
				}
			}
		}

		context.SetResult(-1);
		return;
	});

	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_SERVER_ID", [] (fx::ScriptContext& context)
	{
		int playerId = context.GetArgument<int>(0);
		CNetGamePlayer* player = CNetworkPlayerMgr::GetPlayer(playerId);

		if (player)
		{
			context.SetResult(DoGetServerId(player));
		}
		else
		{
			context.SetResult(0);
		}
	});
});
