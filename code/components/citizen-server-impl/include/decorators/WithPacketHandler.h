#pragma once
#include "ForceConsteval.h"

namespace fx
{
	namespace ServerDecorators
	{
		template<typename... THandler>
		const fwRefContainer<fx::GameServer>& WithPacketHandler(const fwRefContainer<fx::GameServer>& server)
		{
			server->SetComponent(new HandlerMapComponent());

			// store the handler map
			HandlerMapComponent* map = server->GetComponent<HandlerMapComponent>().GetRef();

			server->SetPacketHandler([=](uint32_t packetId, const fx::ClientSharedPtr& client, net::Buffer& packet)
			{
				bool handled = false;

				// any fast-path handlers?
				([&]
				{
					if (!handled && packetId == fx::force_consteval<uint32_t, HashRageString(THandler::GetPacketId())>)
					{
						static THandler handler (server->GetInstance());
						handler.Handle(server->GetInstance(), client, packet);
						handled = true;
					}
				}(), ...);

				// regular handler map
				if (!handled)
				{
					auto entry = map->Get(packetId);

					if (entry)
					{
						auto cb = [client, entry, packet]() mutable
						{
							auto scope = client->EnterPrincipalScope();

							std::get<1>(*entry)(client, packet);
						};

						switch (std::get<fx::ThreadIdx>(*entry))
						{
						case fx::ThreadIdx::Main:
							gscomms_execute_callback_on_main_thread(cb);
							break;
						case fx::ThreadIdx::Net:
							cb();
							break;
						case fx::ThreadIdx::Sync:
							gscomms_execute_callback_on_sync_thread(cb);
							break;
						}
					}
				}
			});

			return server;
		}
	}
}
