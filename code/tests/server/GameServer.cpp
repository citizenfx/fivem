#include <StdInc.h>

#include "GameServer.h"

#include "GameServerInstance.h"

fx::GameServer::GameServer()
{
	
}

fx::GameServer::~GameServer()
{
	
}

void fx::GameServer::AttachToObject(ServerInstanceBase* instance)
{
	
}

std::string fx::GameServer::GetVariable(const std::string& key)
{
	return "";
}

void fx::GameServer::DropClientv(const fx::ClientSharedPtr& client, const std::string& resourceName, ClientDropReason clientDropReason, const std::string& reason)
{
	GameServerInstance::dropClientData.emplace(client, resourceName, clientDropReason, reason);
}

void fx::GameServer::ForceHeartbeatSoon()
{
	
}

void fx::GameServer::Broadcast(const net::Buffer& buffer)
{
	GameServerInstance::broadcastData.emplace(buffer.GetBuffer(), buffer.GetBuffer() + buffer.GetLength());
}
