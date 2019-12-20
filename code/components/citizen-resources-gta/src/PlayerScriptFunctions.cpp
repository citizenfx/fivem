/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>

#include <ScriptEngine.h>
#include <NetworkPlayerMgr.h>

static inline int GetServerId(const rlGamerInfo& platformData)
{
	return (platformData.peerAddress.localAddr.ip.addr & 0xFFFF) ^ 0xFEED;
}

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
				auto platformData = player->GetGamerInfo();
				
				if (GetServerId(*platformData) == serverId)
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
			context.SetResult(GetServerId(*player->GetGamerInfo()));
		}
		else
		{
			context.SetResult(0);
		}
	});
});
