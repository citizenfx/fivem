#pragma once

#include <ServerInstanceBase.h>
#include <Client.h>
#include <NetBuffer.h>

#ifdef COMPILING_CITIZEN_SERVER_IMPL
#define SERVER_IMPL_EXPORT DLL_EXPORT
#else
#define SERVER_IMPL_EXPORT DLL_IMPORT
#endif

class SERVER_IMPL_EXPORT ServerEventPacketHandler
{
public:
	static void Handle(fx::ServerInstanceBase* instance, const fx::ClientSharedPtr& client,
	                   net::Buffer& buffer);

	static constexpr const char* GetPacketId()
	{
		return "msgServerEvent";
	}
};
