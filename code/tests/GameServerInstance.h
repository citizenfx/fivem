#pragma once
#include <optional>

#include "Client.h"
#include "ClientDropReasons.h"

namespace fx
{
class GameServer;

class GameServerInstance
{
public:
	struct DropClientData
	{
		fx::ClientSharedPtr client;
		std::string resourceName;
		ClientDropReason clientDropReason;
		std::string reason;

		DropClientData(const fx::ClientSharedPtr& client, const std::string& resource_name, const ClientDropReason client_drop_reason, const std::string& reason)
			: client(client),
			  resourceName(resource_name),
			  clientDropReason(client_drop_reason),
			  reason(reason)
		{
		}
	};

	static inline std::optional<std::vector<uint8_t>> broadcastData{};
	static inline std::optional<DropClientData> dropClientData{};

	static fx::GameServer* Create();
};
}
