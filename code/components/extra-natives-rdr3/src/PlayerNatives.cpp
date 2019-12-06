#include <StdInc.h>
#include <ScriptEngine.h>

#include <ScriptSerialization.h>
#include <NetworkPlayerMgr.h>

#include <Hooking.h>

static int(*netInterface_GetNumPhysicalPlayers)();
static CNetGamePlayer** (*netInterface_GetAllPhysicalPlayers)();

static HookFunction hookFunction([]()
{
	{
		auto location = hook::get_pattern<char>("48 83 EC 20 80 3D ? ? ? ? 00 4C 8B F1 74", -0x15);
		hook::set_call(&netInterface_GetNumPhysicalPlayers, location + 0x25);
		hook::set_call(&netInterface_GetAllPhysicalPlayers, location + 0x2C);
	}

	fx::ScriptEngine::RegisterNativeHandler("GET_ACTIVE_PLAYERS", [](fx::ScriptContext& context)
	{
		std::vector<int> playerList;

		int playerNum = netInterface_GetNumPhysicalPlayers();
		auto players = netInterface_GetAllPhysicalPlayers();

		for (int i = 0; i < playerNum; i++)
		{
			playerList.push_back(players[i]->physicalPlayerIndex);
		}

		context.SetResult(fx::SerializeObject(playerList));
	});
});
