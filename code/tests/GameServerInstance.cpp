#include <StdInc.h>

#include "GameServerInstance.h"

#include "GameServer.h"

fx::GameServer* fx::GameServerInstance::Create()
{
	return new GameServer();
}
