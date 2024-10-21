#pragma once

#include <ForceConsteval.h>

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

			server->SetPacketHandler([=](uint32_t packetId, const fx::ClientSharedPtr& client, net::ByteReader& reader, ENetPacketPtr& packet)
			{
				bool handled = false;

				// any fast-path handlers?
				([&server, &handled, packetId, &client, &reader, &packet]
				{
					if (!handled && packetId == THandler::PacketType)
					{
						static THandler handler (server->GetInstance());
						handler.Process(server->GetInstance(), client, reader, packet);
						handled = true;
					}
				}(), ...);

				// regular handler map
				if (!handled)
				{
					auto entry = map->Get(packetId);

					if (entry)
					{
						const uint64_t offset = reader.GetOffset();
						auto cb = [client, entry, offset, packet]
						{
							auto scope = client->EnterPrincipalScope();
							net::ByteReader movedReader (packet->data, packet->dataLength);
							movedReader.Seek(offset);
							std::get<1>(*entry)(client, movedReader, packet);
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
