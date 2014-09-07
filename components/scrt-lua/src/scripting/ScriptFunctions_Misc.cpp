#include "StdInc.h"
#include "ResourceScripting.h"
#include "scrEngine.h"

LUA_FUNCTION(GetKeyboardDelays)
{
	int param;
	SystemParametersInfo(SPI_GETKEYBOARDDELAY, 0, &param, 0);

	lua_pushnumber(L, (param * 250) + 250);

	SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &param, 0);

	lua_pushnumber(L, 1000.0 / (((param / 31.0) * 27.5) + 2.5));

	return 2;
}

LUA_FUNCTION(GetNetworkGameConfigPtr)
{
	static int networkGameConfig[30];
	networkGameConfig[0] = 16;
	networkGameConfig[2] = 32;

	lua_pushinteger(L, (int)networkGameConfig);

	return 1;
}

LUA_FUNCTION(NetworkChangeExtendedGameConfigCit)
{
	int gameConfig[31];

	gameConfig[1] = 16;
	gameConfig[2] = 0;
	gameConfig[3] = 32;
	gameConfig[4] = 0;
	gameConfig[5] = 0;
	gameConfig[6] = 0;
	gameConfig[7] = 0;
	gameConfig[8] = 0;

	gameConfig[9] = -1;
	gameConfig[10] = -1;
	gameConfig[11] = -1;
	gameConfig[12] = 0;
	gameConfig[13] = 1;
	gameConfig[14] = 7;
	gameConfig[15] = 0;
	gameConfig[16] = -1;

	gameConfig[17] = -1;
	gameConfig[18] = -1;
	gameConfig[19] = 1;
	gameConfig[20] = 1;
	gameConfig[21] = 0;
	gameConfig[22] = 1;
	gameConfig[23] = 2;
	gameConfig[24] = 0;

	gameConfig[25] = 1;
	gameConfig[26] = 0;
	gameConfig[27] = 0;
	gameConfig[28] = 1;
	gameConfig[29] = 0;
	gameConfig[30] = 0;

	NativeInvoke::Invoke<0x4CFE3998, int*>(gameConfig);

	return 0;
}