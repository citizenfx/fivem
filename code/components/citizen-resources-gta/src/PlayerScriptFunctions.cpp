/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
 */

#include <StdInc.h>
#include <ScriptEngine.h>
#include <NetworkPlayerMgr.h>

static inline int GetServerId(const ScPlayerData* platformData)
{
	return (platformData->addr.ipLan & 0xFFFF) ^ 0xFEED;
}

static InitFunction initFunction([] ()
{
	fx::ScriptEngine::RegisterNativeHandler("GET_PLAYER_FROM_SERVER_ID", [] (fx::ScriptContext& context)
	{
		int serverId = context.GetArgument<int>(0);

		for (int i = 0; i < 32; i++)
		{
			CNetGamePlayer* player = CNetworkPlayerMgr::GetPlayer(i);

			if (player)
			{
				auto platformData = player->GetPlatformPlayerData();
				
				if (GetServerId(platformData) == serverId)
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
			context.SetResult(GetServerId(player->GetPlatformPlayerData()));
		}
		else
		{
			context.SetResult(0);
		}
	});
});